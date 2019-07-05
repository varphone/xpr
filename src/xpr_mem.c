#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#if defined(__linux__) || defined(__unix__)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_utils.h>

#define kMemMapPageSize 0x1000
#define kMemMapPageSizeMask 0xfffffffffffff000
#define kRcHeadMarker 0x52434D4D // 'RCMM'

typedef struct MemMapping
{
    XPR_ListNodeSL _base;
    uintptr_t phyAddr;
    uintptr_t virtAddr;
    size_t length;
    XPR_Atomic refcount;
} MemMapping;

typedef struct RcHead
{
    unsigned int marker;
    XPR_Atomic refcount;
    XPR_Atomic stuff[4];
} RcHead;

static int sMemMappingFd = -1;
static const char* sMemMappingDev = "/dev/mem";
static XPR_List* sMemMappings = NULL;

XPR_API void* XPR_Alloc(size_t size)
{
    return malloc(size);
}

XPR_API void* XPR_AllocRc(size_t size)
{
    void* p = XPR_Alloc(sizeof(RcHead) + size);
    if (p) {
        RcHead* head = (RcHead*)(p);
        head->marker = kRcHeadMarker;
        XPR_AtomicAssign(&head->refcount, 1);
        XPR_AtomicAssign(&head->stuff[0], 1);
        XPR_AtomicAssign(&head->stuff[1], 2);
        XPR_AtomicAssign(&head->stuff[2], 3);
        XPR_AtomicAssign(&head->stuff[3], 4);
        p = (char*)(p) + sizeof(RcHead);
    }
    return p;
}

XPR_API void* XPR_CloneRc(void* ptr)
{
    if (!ptr)
        return NULL;
    RcHead* head = (RcHead*)((char*)(ptr) - sizeof(RcHead));
    if (head->marker != kRcHeadMarker) {
        DBG(DBG_L0, "XPR_MEM: XPR_CloneRc(%p) marker ? %08X != %08X", ptr,
            head->marker, kRcHeadMarker);
        return NULL;
    }
    if (XPR_AtomicInc(&head->refcount) < 2) {
        DBG(DBG_L0, "XPR_MEM: XPR_CloneRc(%p) refcount < 1", ptr);
        return NULL;
    }
    return ptr;
}

XPR_API void XPR_Free(void* ptr)
{
    if (ptr) {
        free(ptr);
        ptr = NULL;
    }
}

XPR_API void XPR_Freep(void** pptr)
{
    if (pptr) {
        if (*pptr) {
            XPR_Free(*pptr);
            *pptr = NULL;
        }
    }
}

XPR_API void XPR_Freev(void** vptr)
{
    if (vptr) {
        void** p = vptr;
        while (*p) {
            XPR_Free(*p);
            p++;
        }
        XPR_Free(vptr);
    }
}

XPR_API void XPR_FreeRc(void* ptr)
{
    if (!ptr)
        return;
    RcHead* head = (RcHead*)((char*)(ptr) - sizeof(RcHead));
    if (head->marker != kRcHeadMarker) {
        DBG(DBG_L0, "XPR_MEM: XPR_FreeRc(%p) marker ? %08X != %08X", ptr,
            head->marker, kRcHeadMarker);
        return;
    }
    if (XPR_AtomicDec(&head->refcount) == 0) {
        XPR_Free(head);
    }
}

static void* memMappingAlloc()
{
    MemMapping* mm = XPR_Alloc(sizeof(*mm));
    if (mm) {
        memset(mm, 0, sizeof(*mm));
        XPR_AtomicAssign(&mm->refcount, 1);
    }
    return mm;
}

static void memMappingFree(void* ptr)
{
    XPR_Free(ptr);
}

static int memMappingCompare(void* a, void* b)
{
    MemMapping* ma = (MemMapping*)(a);
    MemMapping* mb = (MemMapping*)(b);
    if (ma->phyAddr < mb->phyAddr)
        return -1;
    if (ma->phyAddr > mb->phyAddr)
        return 1;
    return 0;
}

#if defined(__linux__) || defined(__unix__)
static void* memMappingRef(uintptr_t phyAddr, size_t size)
{
    if (sMemMappings == NULL)
        return NULL;
    MemMapping* mm = XPR_ListFirst(sMemMappings);
    while (mm) {
        if ((phyAddr >= mm->phyAddr) &&
            ((phyAddr + size) <= (mm->phyAddr + mm->length))) {
            XPR_AtomicInc(&mm->refcount);
            return (void*)(mm->virtAddr + phyAddr - mm->phyAddr);
        }
        mm = XPR_ListNext(sMemMappings, mm);
    }
    return NULL;
}

static int memMappingUnrefPhy(uintptr_t phyAddr, size_t size)
{
    if (sMemMappings == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    MemMapping* mm = XPR_ListFirst(sMemMappings);
    while (mm) {
        if ((phyAddr >= mm->phyAddr) &&
            (phyAddr <= (mm->phyAddr + mm->length))) {
            if (XPR_AtomicDec(&mm->refcount) == 0) {
                munmap((void*)(mm->virtAddr), mm->length);
                XPR_ListRemove(sMemMappings, mm);
            }
            return XPR_ERR_OK;
        }
        mm = XPR_ListNext(sMemMappings, mm);
    }
    return XPR_ERR_GEN_UNEXIST;
}

static int memMappingUnrefVirt(uintptr_t virtAddr)
{
    if (sMemMappings == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    MemMapping* mm = XPR_ListFirst(sMemMappings);
    while (mm) {
        if ((virtAddr >= mm->virtAddr) &&
            (virtAddr <= (mm->virtAddr + mm->length))) {
            if (XPR_AtomicDec(&mm->refcount) == 0) {
                munmap((void*)(mm->virtAddr), mm->length);
                XPR_ListRemove(sMemMappings, mm);
            }
            return XPR_ERR_OK;
        }
        mm = XPR_ListNext(sMemMappings, mm);
    }
    return XPR_ERR_GEN_UNEXIST;
}

static void* MemMapFile(const char* fileName, size_t size, int readOnly)
{
    int openFlags = readOnly ? O_RDONLY : (O_RDWR | O_CREAT);
    int openMode = readOnly ? (S_IRUSR | S_IRGRP) : (S_IRWXU | S_IRWXG);
    int fd = open(fileName, openFlags | O_SYNC, openMode);
    if (fd < 0) {
        DBG(DBG_L2, "XPR_MEM: MemMapFile: open(%s) failed, errno: %d", fileName,
            errno);
        return NULL;
    }
    int mmapFlags = readOnly ? PROT_READ : (PROT_READ | PROT_WRITE);
    void* virt = mmap(0, size, mmapFlags, MAP_SHARED, fd, 0);
    close(fd);
    if (virt == NULL) {
        DBG(DBG_L2, "XPR_MEM: MemMapFile: mmap(%d) failed, errno: %d", size,
            errno);
        return NULL;
    }
    return virt;
}

XPR_API void* XPR_MemMap(uintptr_t phyAddr, size_t size)
{
    void* virtAddr = memMappingRef(phyAddr, size);
    if (virtAddr)
        return virtAddr;

    if (sMemMappingFd < 0) {
        sMemMappingFd = open(sMemMappingDev, O_RDWR | O_SYNC);
        if (sMemMappingDev < 0) {
            DBG(DBG_L2, "XPR_MEM: XPR_MemMap: open(%s) failed, errno: %s",
                sMemMappingDev);
            return NULL;
        }
    }

    // addr align in page_size(4K)
    uintptr_t phyAddrInPage = phyAddr & kMemMapPageSizeMask;
    uintptr_t pageDiff = phyAddr - phyAddrInPage;
    // size in page_size
    size_t sizeInPage =
        ((size + pageDiff - 1) & kMemMapPageSizeMask) + kMemMapPageSize;
    virtAddr = mmap((void*)0, sizeInPage, PROT_READ | PROT_WRITE, MAP_SHARED,
                    sMemMappingFd, phyAddrInPage);
    if (virtAddr == MAP_FAILED) {
        DBG(DBG_L2, "XPR_MEM: XPR_MemMap: mmap(0x%X,0x%X) failed, errno: %d",
            sizeInPage, phyAddrInPage);
        return NULL;
    }

    if (sMemMappings == NULL) {
        sMemMappings = XPR_ListCreate(XPR_LIST_SINGLY_LINKED, memMappingAlloc,
                                      memMappingFree, memMappingCompare);
        if (sMemMappings == NULL) {
            munmap((void*)virtAddr, sizeInPage);
            return NULL;
        }
    }

    MemMapping* mm = memMappingAlloc();
    if (mm) {
        mm->phyAddr = phyAddrInPage;
        mm->virtAddr = (uintptr_t)(virtAddr);
        mm->length = sizeInPage;
        XPR_ListAppend(sMemMappings, mm);
    }

    return (void*)(virtAddr + pageDiff);
}

XPR_API void* XPR_MemMapFile(const char* fileName, size_t size)
{
    return MemMapFile(fileName, size, 0);
}

XPR_API void* XPR_MemMapFileRO(const char* fileName, size_t size)
{
    return MemMapFile(fileName, size, 1);
}

XPR_API int XPR_MemUnmap(void* virtAddr, size_t size)
{
    if (!virtAddr)
        return XPR_ERR_GEN_NULL_PTR;
    if (size <= 0)
        return memMappingUnrefVirt((uintptr_t)(virtAddr));
    if (munmap((void*)(virtAddr), size) < 0)
        return XPR_ERR_ERROR;
    return XPR_ERR_OK;
}
#else
XPR_API void* XPR_MemMap(uintptr_t phyAddr, size_t size)
{
    // FIXME:
    return NULL;
}

XPR_API void* XPR_MemMapFile(const char* fileName, size_t size)
{
    // FIXME:
    return NULL;
}

XPR_API void* XPR_MemMapFileRO(const char* fileName, size_t size)
{
    // FIXME:
    return NULL;
}

XPR_API int XPR_MemUnmap(void* virtAddr)
{
    // FIXME:
    return XPR_ERR_GEN_NOT_SUPPORT;
}
#endif

XPR_API long XPR_RcGetMeta(void* ptr, int slot)
{
    if (!ptr || slot < 0 || slot > 3)
        return 0;
    RcHead* head = (RcHead*)((char*)(ptr) - sizeof(RcHead));
    if (head->marker != kRcHeadMarker) {
        DBG(DBG_L0, "XPR_MEM: XPR_RcGetMeta(%p) marker ? %08X != %08X", ptr,
            head->marker, kRcHeadMarker);
        return 0;
    }
    return head->stuff[slot];
}

XPR_API int XPR_RcSetMeta(void* ptr, int slot, long val)
{
    if (!ptr || slot < 0 || slot > 3)
        return XPR_FALSE;
    RcHead* head = (RcHead*)((char*)(ptr) - sizeof(RcHead));
    if (head->marker != kRcHeadMarker) {
        DBG(DBG_L0, "XPR_MEM: XPR_RcSetMeta(%p) marker ? %08X != %08X", ptr,
            head->marker, kRcHeadMarker);
        return XPR_FALSE;
    }
    XPR_AtomicAssign(&head->stuff[slot], val);
    return XPR_TRUE;
}

XPR_API char* XPR_StrDup(const char* str)
{
#if defined(_MSC_VER)
    return _strdup(str);
#else
    return strdup(str);
#endif
}

XPR_API wchar_t* XPR_StrDupW(const wchar_t* str)
{
#if defined(_MSC_VER)
    return _wcsdup(str);
#else
    return wcsdup(str);
#endif
}
