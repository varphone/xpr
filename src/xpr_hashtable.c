/*
 * File: xpr_hashtable.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 哈希表
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-12 09:02:31 Friday, 12 July
 * Last Modified : 2019-07-12 09:02:37 Friday, 12 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012-2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012-2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-12   varphone    创始版本建立
 * 
 */
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_hashtable.h>
#include <xpr/xpr_sync.h>
#include <xpr/xpr_utils.h>

#define XPR_HASHTABLE_WARN_CONFLICTS 32

// 哈希表项定义
struct XPR_HashTableItem {
    uint32_t hash1; // 哈希1
    uint32_t hash2; // 哈希2
    uint32_t hash3; // 哈希3
    int refcount;   // 引用计数
    void* key;      // 键
    void* value;    // 值
};

// 哈希表项别名
typedef struct XPR_HashTableItem XPR_HashTableItem;

// 哈希表定义
struct XPR_HashTable
{
    size_t capacity;          // 容量
    XPR_HashTableAlgo* algo;  // 算法
    XPR_RecursiveMutex lock;  // 同步锁
    size_t count;             // 哈希表项实际项数量
    XPR_HashTableItem* items; // 哈希表项列表
};

// Copy a string
void* strCopy(void* str)
{
    if (str)
        return XPR_StrDup(str);
    return NULL;
}

// Drop a string
void strDrop(void* str)
{
    if (str)
        XPR_Free(str);
}

// Crypt table for MPQ hash
static uint32_t sMpqCryptTable[0x500];
static XPR_Atomic sMpqCryptTableInited = 0;

// Initialize the mpq crypt table
static void mpqInitCryptTable()
{
    uint32_t seed = 0x00100001;
    uint32_t index1 = 0;
    uint32_t index2 = 0;
    int i;

    for (index1 = 0; index1 < 0x100; index1++) {
        for (index2 = index1, i = 0; i < 5; i++, index2 += 0x100) {
            uint32_t temp1, temp2;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp1 = (seed & 0xFFFF) << 0x10;
            seed = (seed * 125 + 3) % 0x2AAAAB;
            temp2 = (seed & 0xFFFF);
            sMpqCryptTable[index2] = (temp1 | temp2);
        }
    }
}

// Return hash of the string (MPQ)
uint32_t mpqHash(void* str, uint32_t seed)
{
    uint8_t* key = (uint8_t*)str;
    uint32_t seed1 = 0x7FED7FED;
    uint32_t seed2 = 0xEEEEEEEE;

    if (!sMpqCryptTableInited) {
        mpqInitCryptTable();
        XPR_AtomicInc(&sMpqCryptTableInited);
    }
    while (*key != 0) {
        int ch = *key++;
        // Turn on the follow line without case sensitive
        // ch = toupper(ch);
        seed1 = sMpqCryptTable[(seed << 8) + ch] ^ (seed1 + seed2);
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
    }
    return seed1;
}

// Return hash of the string (BKQ)
uint32_t bkdHash(void* str, uint32_t seed)
{
    char* ptr = (char*)str;
    uint32_t hash = 0;
    while (*ptr) {
        hash = hash * 131 + *ptr + seed;
        ptr++;
    }
    return hash;
}

// Return TRUE if the ptr valid
static int ptrValidate(void* ptr)
{
    return ptr ? XPR_TRUE : XPR_FALSE;
}

// Default algo for string type
static XPR_HashTableAlgo sStrAlgo = {
    strCopy,     // keyCopy
    strDrop,     // keyDrop
    mpqHash,     // keyHash
    ptrValidate, // keyValidate
    strCopy,     // valueCopy
    strDrop,     // valueDrop
};

// Remove the item matched to the index from the table
static void hashTableErase(XPR_HashTable* self, int index)
{
    XPR_HashTableItem* item = &self->items[index];
    self->algo->keyDrop(item->key);
    self->algo->valueDrop(item->value);
    item->hash1 = 0;
    item->hash2 = 0;
    item->hash3 = 0;
    item->refcount = 0;
    item->key = NULL;
    item->value = NULL;
    self->count--;
}

// Return index of the key, -1 if not found
static int hashTableIndex(XPR_HashTable* self, void* key)
{
    uint32_t hash1 = self->algo->keyHash(key, 0);
    uint32_t hash2 = self->algo->keyHash(key, 1);
    uint32_t hash3 = self->algo->keyHash(key, 2);
    int start = hash1 % self->capacity;
    int index = start;
    int conflicts = 0;
    XPR_HashTableItem* item = &self->items[index];
    while (item->refcount) {
        if (item->hash1 == hash1 && item->hash2 == hash2 &&
            item->hash3 == hash3) {
            if (conflicts >= XPR_HASHTABLE_WARN_CONFLICTS) {
                DBG(DBG_L1,
                    "XPR_HASHTABLE @ %p: Too many conflicts(%d) on index, you "
                    "have to enlarge the capacity!",
                    self, conflicts);
            }
            return index;
        }
        index = (index + 1) % self->capacity;
        if (index == start)
            break;
        item = &self->items[index];
        conflicts++;
    }
    return -1;
}

// Put key and value into the table
static int hashTablePut(XPR_HashTable* self, void* key, void* value)
{
    if (self->count >= self->capacity)
        return -1;
    uint32_t hash1 = self->algo->keyHash(key, 0);
    uint32_t hash2 = self->algo->keyHash(key, 1);
    uint32_t hash3 = self->algo->keyHash(key, 2);
    int start = hash1 % self->capacity;
    int index = start;
    int conflicts = 0;
    XPR_HashTableItem* item = &self->items[index];
    while (item->refcount) {
        index = (index + 1) % self->capacity;
        if (index == start) {
            if (conflicts >= XPR_HASHTABLE_WARN_CONFLICTS) {
                DBG(DBG_L1,
                    "XPR_HASHTABLE @ %p: Too may conflicts(%d) on put, you "
                    "have to enlarge the capacity!",
                    self, conflicts);
            }
            return -1;
        }
        item = &self->items[index];
        conflicts++;
    };
    item->hash1 = hash1;
    item->hash2 = hash2;
    item->hash3 = hash3;
    item->refcount = 1;
    item->key = self->algo->keyCopy(key);
    item->value = self->algo->valueCopy(value);
    self->count++;
    if (conflicts >= XPR_HASHTABLE_WARN_CONFLICTS) {
        DBG(DBG_L1,
            "XPR_HASHTABLE @ %p: Too may conflicts(%d) on put, you have to "
            "enlarge the capacity!",
            self, conflicts);
    }
    return index;
}

// Update value of the item matched to index in the table
static void hashTableUpdate(XPR_HashTable* self, int index, void* value)
{
    XPR_HashTableItem* item = &self->items[index];
    self->algo->valueDrop(item->value);
    item->value = self->algo->valueCopy(value);
}

XPR_API XPR_HashTable* XPR_HashTableCreate(size_t capacity, XPR_HashTableAlgo* algo)
{
    XPR_HashTable* self = XPR_Alloc(sizeof(*self));
    if (self) {
        self->capacity = capacity;
        self->algo = algo ? algo : &sStrAlgo;
        XPR_RecursiveMutexInit(&self->lock);
        self->count = 0;
        self->items = XPR_Alloc(sizeof(XPR_HashTableItem) * self->capacity);
        memset(self->items, 0, sizeof(XPR_HashTableItem) * self->capacity);
    }
    return self;
}

XPR_API void XPR_HashTableDestroy(XPR_HashTable* self)
{
    if (self) {
        XPR_RecursiveMutexLock(&self->lock);
        XPR_HashTableClear(self);
        if (self->items) {
            XPR_Free(self->items);
            self->items = NULL;
        }
        XPR_RecursiveMutexUnlock(&self->lock);
        XPR_RecursiveMutexFini(&self->lock);
        XPR_Free(self);
    }
}

XPR_API void XPR_HashTableClear(XPR_HashTable* self)
{
    if (!self)
        return;
    XPR_RecursiveMutexLock(&self->lock);
    for (int i = 0; i < self->capacity; i++) {
        XPR_HashTableItem* item = &self->items[i];
        self->algo->keyDrop(item->key);
        self->algo->valueDrop(item->value);
        item->hash1 = 0;
        item->hash2 = 0;
        item->hash3 = 0;
        item->refcount = 0;
        item->key = NULL;
        item->value = NULL;
    }
    self->count = 0;
    XPR_RecursiveMutexUnlock(&self->lock);
}


XPR_API int XPR_HashTableErase(XPR_HashTable* self, void* key)
{
    if (!self)
        return XPR_ERR_GEN_NULL_PTR;
    if (!self->algo->keyValidate(key))
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int index = -1;
    XPR_RecursiveMutexLock(&self->lock);
    index = hashTableIndex(self, key);
    if (index >= 0)
        hashTableErase(self, index);
    XPR_RecursiveMutexUnlock(&self->lock);
    return index >= 0 ? XPR_ERR_OK : XPR_ERR_SYS(ENOENT);
}

XPR_API void* XPR_HashTableGet(XPR_HashTable* self, void* key)
{
    if (!self)
        return NULL;
    if (!self->algo->keyValidate(key))
        return NULL;
    void* value = NULL;
    XPR_RecursiveMutexLock(&self->lock);
    int index = hashTableIndex(self, key);
    if (index >= 0)
        value = self->items[index].value;
    XPR_RecursiveMutexUnlock(&self->lock);
    return value;
}

XPR_API void* XPR_HashTableIterFirst(XPR_HashTable* self)
{
    if (!self || (self && self->count <= 0))
        return NULL;
    XPR_HashTableItem* start = self->items;
    XPR_HashTableItem* end = start + self->capacity;
    XPR_RecursiveMutexLock(&self->lock);
    while (start < end && !start->refcount)
        start++;
    if (start >= end)
        start = NULL;
    XPR_RecursiveMutexUnlock(&self->lock);
    return start;
}

XPR_API void* XPR_HashTableIterNext(XPR_HashTable* self, void* iter)
{
    if (!self || (self && self->count <= 0))
        return NULL;
    XPR_HashTableItem* start = (XPR_HashTableItem*)(iter) + 1;
    XPR_HashTableItem* end = self->items + self->capacity;
    XPR_RecursiveMutexLock(&self->lock);
    while (start < end && !start->refcount)
        start++;
    if (start >= end)
        start = NULL;
    XPR_RecursiveMutexUnlock(&self->lock);
    return start;
}

XPR_API void* XPR_HashTableIterKey(void* iter)
{
    if (iter)
        return ((XPR_HashTableItem*)(iter))->key;
    return NULL;
}

XPR_API void* XPR_HashTableIterValue(void* iter)
{
    if (iter)
        return ((XPR_HashTableItem*)(iter))->value;
    return NULL;
}

XPR_API int XPR_HashTableSet(XPR_HashTable* self, void* key, void* value)
{
    if (!self)
        return XPR_ERR_GEN_NULL_PTR;
    if (!self->algo->keyValidate(key))
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int index = -1;
    XPR_RecursiveMutexLock(&self->lock);
    index = hashTableIndex(self, key);
    if (index >= 0)
        hashTableUpdate(self, index, value);
    else
        index = hashTablePut(self, key, value);
    XPR_RecursiveMutexUnlock(&self->lock);
    return index >= 0 ? XPR_ERR_OK : XPR_ERR_SYS(ENOSPC);
}

XPR_API int XPR_HashTableSize(XPR_HashTable* self)
{
    if (!self)
        return 0;
    return self->count;
}
