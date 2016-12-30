#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_fq.h>
#include <xpr/xpr_mem.h>
#if defined(HAVE_MP)
#include <xpr/xpr_sync.h>
#endif // defined(HAVE_MP)

struct XPR_FQ {
    size_t			maxQueues;			// 最大队列长度
    size_t			maxBufferSize;		// 最大缓冲区长度
    XPR_Fifo*       freeList;           // 空闲 Fifo
    XPR_Fifo*       queuedList;         // 队列 Fifo
    XPR_FQ_ENTRY*  	entries;            // 条目列表
    uint8_t         stuff[8];           //
};

#if defined(HAVE_MP)
#define XPR_FQ_LOCK_INIT(f)   XPR_SpinLockInit(&(f)->lock)
#define XPR_FQ_LOCK_FINI(f)   XPR_SpinLockFini(&(f)->lock)
#define XPR_FQ_LOCK(f)        XPR_SpinLockLock(&(f)->lock)
#define XPR_FQ_UNLOCK(f)      XPR_SpinLockUnlock(&(f)->lock)
#else
#define XPR_FQ_LOCK_INIT(f)   do {} while (0)
#define XPR_FQ_LOCK_FINI(f)   do {} while (0)
#define XPR_FQ_LOCK(f)        do {} while (0)
#define XPR_FQ_UNLOCK(f)      do {} while (0)
#endif // defined(HAVE_MP)

XPR_API XPR_FQ* XPR_FQ_Create(size_t maxQueues, size_t maxBufferSize)
{
    size_t      i       = 0;
    size_t      total   = sizeof(XPR_FQ) + sizeof(XPR_FQ_ENTRY) * maxQueues;
    XPR_FQ*     fq      = XPR_Alloc(total);
    if (fq) {
        memset(fq, 0, total);
        fq->maxQueues = maxQueues;
        fq->maxBufferSize = maxBufferSize;
        fq->freeList = XPR_FifoCreate(sizeof(XPR_FQ_ENTRY*), (int)maxQueues);
        fq->queuedList = XPR_FifoCreate(sizeof(XPR_FQ_ENTRY*), (int)maxQueues);
        fq->entries = (XPR_FQ_ENTRY*)(((uint8_t*)fq) + sizeof(*fq));
        for (i = 0; i < maxQueues; i++) {
            XPR_FifoPutAsAtomic(fq->freeList, (uintptr_t)&fq->entries[i]);
        }
    }
    return fq;
}


XPR_API int XPR_FQ_Destroy(XPR_FQ* fq)
{
    XPR_FQ_ENTRY*   entry = 0;
    if (fq == NULL)
        return XPR_ERR_GEN_NULL_PTR;
    while ((entry = (XPR_FQ_ENTRY*)XPR_FifoGetAsAtomic(fq->queuedList))) {
        if (entry->data)
            free((void*)entry->data);
        entry->data = NULL;
        entry->length = 0;
        entry->flags = 0;
    }
    XPR_FifoDestroy(fq->freeList);
    XPR_FifoDestroy(fq->queuedList);
    free((void*)fq);
    return XPR_ERR_OK;
}

XPR_API int XPR_FQ_Clear(XPR_FQ* fq)
{
    if (fq == NULL)
        return XPR_ERR_GEN_NULL_PTR;

    return XPR_ERR_OK;
}

XPR_API XPR_FQ_ENTRY* XPR_FQ_PopFront(XPR_FQ* fq)
{
    XPR_FQ_ENTRY* ent = NULL;
    if (fq == NULL)
        return NULL;
    ent = (XPR_FQ_ENTRY*)XPR_FifoGetAsAtomic(fq->queuedList);
    return ent;
}

XPR_API int XPR_FQ_ReleaseEntry(XPR_FQ* fq, XPR_FQ_ENTRY* entry)
{
    if (fq == NULL || entry == NULL)
        return XPR_ERR_GEN_NULL_PTR;
    if (entry->data)
        free((void*)entry->data);
    entry->data = NULL;
    entry->length = 0;
    entry->flags = 0;
    XPR_FifoPutAsAtomic(fq->freeList, (uintptr_t)entry);
    return XPR_ERR_OK;
}

XPR_API int XPR_FQ_PushBack(XPR_FQ* fq, const XPR_FQ_ENTRY* entry)
{
    void*           ptr = NULL;
    XPR_FQ_ENTRY*   ent = NULL;
    if (fq == NULL)
        return XPR_ERR_GEN_NULL_PTR;
    ent = (XPR_FQ_ENTRY*)XPR_FifoGetAsAtomic(fq->freeList);
    if (ent == NULL) {
        return XPR_ERR_GEN_NOBUF;
    }
    ptr = XPR_Alloc(entry->length);
    if (ptr == NULL) {
        XPR_FifoPutAsAtomic(fq->freeList, (uintptr_t)ent);
        return XPR_ERR_GEN_NOMEM;
    }
    memcpy(ptr, entry->data, entry->length);
    ent->data = ptr;
    ent->length = entry->length;
    ent->flags = entry->flags;
    XPR_FifoPutAsAtomic(fq->queuedList, (uintptr_t)ent);
    return XPR_ERR_OK;
}

XPR_API int XPR_FQ_PushBackRaw(XPR_FQ* fq, const void* data, size_t length, size_t flags)
{
    XPR_FQ_ENTRY ent = { (uint8_t*)data, length, flags };
    return XPR_FQ_PushBack(fq, &ent);
}