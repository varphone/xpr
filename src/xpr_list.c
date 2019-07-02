/*
 * File: xpr_list.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Double/Singly Linked List
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-02 08:20:17 Tuesday, 2 July
 * Last Modified : 2019-07-02 10:43:04 Tuesday, 2 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2010 - 2019 Varphone Wong, Varphone.com
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 */
#include <xpr/xpr_errno.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_sync.h>
#include <xpr/xpr_utils.h>

struct XPR_List
{
    XPR_ListType type;           // Type of the elements
    XPR_ListNodeAlloc alloc;     // For allocate elements
    XPR_ListNodeFree free;       // For free elements
    XPR_ListNodeCompare compare; // For sort elements
    XPR_Mutex mutex;             // Synchronization lock
    XPR_ListNode* head;          // The first element of the list
    XPR_ListNode* tail;          // The last element of the list (DL only)
    int length;                  // Num of elements added to the list
    void* opaque;
};

// Returns the node at position n in the linked list.
static XPR_ListNode* slnFindAt(XPR_ListNode* head, int pos, int length)
{
    int nth = 0;
    if (pos < 0)
        pos += length;
    while (head && ++nth <= pos)
        head = head->next;
    return head;
}

// Returns the last node in the the linked list.
static XPR_ListNode* slnFindTail(XPR_ListNode* head)
{
    while (head && head->next)
        head = head->next;
    return head;
}

// Applies fn to each element in the linked list.
static void slnForEach(XPR_ListNode* head, XPR_ListForEachFn fn, void* opaque)
{
    while (head) {
        if (fn(opaque, head) < 0)
            break;
        head = head->next;
    }
}

// Calculate the length of the linked list.
static int slnLength(XPR_ListNode* head)
{
    int length = 0;
    while (head) {
        head = head->next;
        length++;
    }
    return length;
}

// Remove and free node from linked list and return prev node.
static int slnRemove(XPR_ListNode** head, XPR_ListNode* node,
                     XPR_ListNodeFree nodeFree, XPR_ListNode** prev)
{
    XPR_ListNode* temp = NULL;
    while (*head) {
        if (*head == node) {
            *head = node->next;
            nodeFree(node);
            node = NULL;
            break;
        }
        temp = *head;
        head = &temp->next;
    }
    if (node == NULL && prev)
        *prev = temp;
    return node == NULL ? XPR_ERR_OK : XPR_ERR_GEN_UNEXIST;
}

// Remove and free all nodes from linked list.
static void slnRemoveAll(XPR_ListNode* head, XPR_ListNodeFree nodeFree)
{
    XPR_ListNode* temp = NULL;
    while (head) {
        temp = head->next;
        nodeFree(head);
        head = temp;
    }
}

// Insert node with sort order to linked list.
static void slnSortedInsert(XPR_ListNode** head, XPR_ListNode* node,
                           XPR_ListNodeCompare compare, int reverse)
{
    XPR_ListNode** pp = head;
    while (*pp) {
        XPR_ListNode* n1 = reverse ? node : *pp;
        XPR_ListNode* n2 = reverse ? *pp : node;
        if (compare(n1, n2) > 0) {
            node->next = *pp;
            *pp = node;
            break;
        }
        pp = &(*pp)->next;
    }
    if (*pp && (*pp)->next == NULL) {
        (*pp)->next = node;
    }
    else {
        *pp = node;
    }
}

// Sort all nodes in linked list.
static void slnSort(XPR_ListNode** head, XPR_ListNodeCompare compare,
                       int reverse)
{
    XPR_ListNode* next = (*head)->next;
    XPR_ListNode* temp = NULL;
    // Take the first element out of the list.
    (*head)->next = NULL;
    // Sort and insert again.
    while (next) {
        temp = next->next;
        next->next = NULL; // Unlinked to node.
        slnSortedInsert(head, next, compare, reverse);
        next = temp;
    }
}

static int slnTake(XPR_ListNode** head, XPR_ListNode* node)
{
    XPR_ListNode* temp = NULL;
    while (*head) {
        if (*head == node) {
            *head = node->next;
            node->next = NULL;
            node = NULL;
            break;
        }
        temp = *head;
        head = &temp->next;
    }
    return node == NULL ? XPR_ERR_OK : XPR_ERR_GEN_UNEXIST;
}

static int slistAppend(XPR_List* list, XPR_ListNode* node)
{
    node->next = NULL;
    // Add to first position if empty.
    if (list->head == NULL) {
        list->head = list->tail = node;
        return XPR_ERR_OK;
    }
    // Add to last position if compare not exists.
    if (list->compare == NULL) {
        list->tail->next = node;
        list->tail = node;
        return XPR_ERR_OK;
    }
    slnSortedInsert(&list->head, node, list->compare, 0);
    // Update tail if changed.
    list->tail = slnFindTail(node);
    return XPR_ERR_OK;
}

static int slistRemove(XPR_List* list, XPR_ListNode* node)
{
    XPR_ListNode* prev = NULL;
    int err = slnRemove(&list->head, node, list->free, &prev);
    // Update tail if changed.
    if (err == XPR_ERR_OK && prev)
        list->tail = slnFindTail(prev);
    return err;
}

static int slistRemoveAt(XPR_List* list, int pos)
{
    return slistRemove(list, slnFindAt(list->head, pos, list->length));
}

XPR_API XPR_List* XPR_ListCreate(XPR_ListType type, XPR_ListNodeAlloc nodeAlloc,
                                 XPR_ListNodeFree nodeFree,
                                 XPR_ListNodeCompare nodeCompare)
{
    XPR_List* list = XPR_Alloc(sizeof(*list));
    if (list) {
        list->type = type;
        list->alloc = nodeAlloc;
        list->free = nodeFree;
        list->compare = nodeCompare;
        XPR_MutexInit(&list->mutex);
        list->head = NULL;
        list->tail = NULL;
        list->length = 0;
    }
    return list;
}

XPR_API void XPR_ListDestroy(XPR_List* list)
{
    if (!list)
        return;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        slnRemoveAll(list->head, list->free);
        break;
    default:
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    XPR_Free(list);
}

XPR_API int XPR_ListAppend(XPR_List* list, void* node)
{
    if (!list || !node)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        err = slistAppend(list, node);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    if (err == XPR_ERR_OK)
        list->length++;
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}

XPR_API void* XPR_ListAppendNew(XPR_List* list)
{
    if (!list)
        return NULL;
    void* node = list->alloc ? list->alloc() : NULL; 
    if (node) {
        if (XPR_ListAppend(list, node) != XPR_ERR_OK) {
            list->free(node);
            node = NULL;
        }
    }
    return node;
}

XPR_API void* XPR_ListAt(XPR_List* list, int pos)
{
    if (!list)
        return NULL;
    XPR_ListNode* node = NULL;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        node = slnFindAt(list->head, pos, list->length);
        break;
    default:
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return node;
}

XPR_API int XPR_ListClear(XPR_List* list)
{
    if (!list)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_DOUBLE_LINKED:
    case XPR_LIST_SINGLY_LINKED:
        slnRemoveAll(list->head, list->free);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}

XPR_API void* XPR_ListFirst(XPR_List* list)
{
    if (!list)
        return NULL;
    XPR_ListNode* node = NULL;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_DOUBLE_LINKED:
    case XPR_LIST_SINGLY_LINKED:
        node = list->head;
        break;
    default:
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return node;
}

XPR_API void* XPR_ListLast(XPR_List* list)
{
    if (!list)
        return NULL;
    XPR_ListNode* node = NULL;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_DOUBLE_LINKED:
    case XPR_LIST_SINGLY_LINKED:
        node = list->tail;
        break;
    default:
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return node;
}

XPR_API void* XPR_ListNext(XPR_List* list, void* curr)
{
    if (!list || !curr)
        return NULL;
    XPR_ListNode* node = curr;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_DOUBLE_LINKED:
    case XPR_LIST_SINGLY_LINKED:
        if (node == list->tail)
            node = NULL;
        else
            node = node->next;
        break;
    default:
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return node;
}

XPR_API int XPR_ListRemove(XPR_List* list, void* node)
{
    if (!list || !node)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        err = slistRemove(list, node);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    if (err == XPR_ERR_OK)
        list->length--;
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}

XPR_API int XPR_ListRemoveAt(XPR_List* list, int pos)
{
    if (!list)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        err = slistRemoveAt(list, pos);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    if (err == XPR_ERR_OK)
        list->length--;
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}

XPR_API void XPR_ListForEach(XPR_List* list, XPR_ListForEachFn fn,
                             void* opaque)
{
    if (!list || !fn)
        return;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_DOUBLE_LINKED:
    case XPR_LIST_SINGLY_LINKED:
        slnForEach(list->head, fn, opaque);
        break;
    default:
        break;
    }
    XPR_MutexUnlock(&list->mutex);
}

XPR_API int XPR_ListReverse(XPR_List* list, XPR_ListNodeCompare compare)
{
    if (!list)
        return XPR_ERR_GEN_NULL_PTR;
    compare = compare ? compare : list->compare;
    if (!compare)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        slnSort(&list->head, compare, 1);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}

XPR_API int XPR_ListSort(XPR_List* list, XPR_ListNodeCompare compare)
{
    if (!list)
        return XPR_ERR_GEN_NULL_PTR;
    compare = compare ? compare : list->compare;
    if (!compare)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        slnSort(&list->head, compare, 0);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}

XPR_API int XPR_ListTake(XPR_List* list, void* node)
{
    if (!list)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_MutexLock(&list->mutex);
    switch (list->type) {
    case XPR_LIST_SINGLY_LINKED:
        err = slnTake(&list->head, node);
        break;
    default:
        err = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    XPR_MutexUnlock(&list->mutex);
    return XPR_ERR_OK;
}
