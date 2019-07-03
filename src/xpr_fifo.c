#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32)
#include <crtdbg.h>
#include <intrin.h>
#include <malloc.h>

#endif // defined(WIN32) || defined(_WIN32)
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_utils.h>
#if defined(HAVE_MP)
#include <xpr/xpr_sync.h>
#endif // defined(HAVE_MP)

struct XPR_Fifo
{
    XPR_Atomic head;
    XPR_Atomic tail;
    unsigned int elementSize;
    unsigned int maxElements;
    unsigned char* data;
#if defined(HAVE_MP)
    XPR_SpinLock lock;
#endif // defined(HAVE_MP)
};

#if defined(HAVE_MP)
#define XPR_FIFO_LOCK_INIT(f)   XPR_SpinLockInit(&(f)->lock)
#define XPR_FIFO_LOCK_FINI(f)   XPR_SpinLockFini(&(f)->lock)
#define XPR_FIFO_LOCK(f)        XPR_SpinLockLock(&(f)->lock)
#define XPR_FIFO_UNLOCK(f)      XPR_SpinLockUnlock(&(f)->lock)
#else
#define XPR_FIFO_LOCK_INIT(f)   do {} while (0)
#define XPR_FIFO_LOCK_FINI(f)   do {} while (0)
#define XPR_FIFO_LOCK(l)        do {} while (0)
#define XPR_FIFO_UNLOCK(l)      do {} while (0)
#endif // defined(HAVE_MP)

XPR_API XPR_Fifo* XPR_FifoCreate(int elementSize, int maxElements)
{
    XPR_Fifo* f = (XPR_Fifo*)aligned_malloc(sizeof(*f), 8);
    if (f) {
        f->elementSize = elementSize;
        f->maxElements = maxElements;
        f->head = 0;
        f->tail = 0;
        f->data = (unsigned char*)aligned_malloc(elementSize * maxElements, 8);
        XPR_FIFO_LOCK_INIT(f);
        if (!f->data) {
            free((void*)f);
            f = 0;
        }
    }
    return f;
}

XPR_API int XPR_FifoDestroy(XPR_Fifo* f)
{
    if (f && f->data)
        aligned_free((void*)f->data);
    if (f) {
        XPR_FIFO_LOCK_FINI(f);
        aligned_free((void*)f);
    }
    return 0;
}

XPR_API int XPR_FifoGet(XPR_Fifo* f, void* buffer, int size)
{
    int l = 0;
    int n = 0;
    if (!f || !buffer)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_FIFO_LOCK(f);
    if (XPR_FifoIsEmpty(f)) {
        XPR_FIFO_UNLOCK(f);
        return XPR_ERR_GEN_NOBUF;
    }
    size = MIN(size, XPR_FifoGetLength(f));
    for (l = 0; l < size; l++) {
        n = f->tail % f->maxElements;
        memcpy((unsigned char*)buffer + l * f->elementSize,
               f->data + n * f->elementSize, f->elementSize);
        XPR_AtomicInc(&f->tail);
    }
    XPR_FIFO_UNLOCK(f);
    return size;
}

XPR_API int XPR_FifoPut(XPR_Fifo* f, const void* data, int size)
{
    int l = 0;
    int n = 0;
    if (!f || !data)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_FIFO_LOCK(f);
    if (XPR_FifoIsFull(f)) {
        XPR_FIFO_UNLOCK(f);
        return XPR_ERR_GEN_NOBUF;
    }
    size = MIN(size, XPR_FifoGetAvailableSize(f));
    for (l = 0; l < size; l++) {
        n = f->head % f->maxElements;
        memcpy(f->data + n * f->elementSize,
               (const unsigned char*)data + l * f->elementSize, f->elementSize);
        XPR_AtomicInc(&f->head);
    }
    XPR_FIFO_UNLOCK(f);
    return size;
}

XPR_API int XPR_FifoPeek(XPR_Fifo* f, void* buffer, int size, int offset)
{
    int l = 0;
    int n = 0;
    if (!f || !buffer)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_FIFO_LOCK(f);
    if (XPR_FifoIsEmpty(f)) {
        XPR_FIFO_UNLOCK(f);
        return XPR_ERR_GEN_NOBUF;
    }
    size = MIN(size, XPR_FifoGetLength(f) - offset);
    for (l = 0; l < size; l++) {
        n = (f->tail + offset + l) % f->maxElements;
        memcpy((unsigned char*)buffer + l * f->elementSize,
               f->data + n * f->elementSize, f->elementSize);
    }
    XPR_FIFO_UNLOCK(f);
    return size;
}

XPR_API uintptr_t XPR_FifoGetAsAtomic(XPR_Fifo* f)
{
    int n = 0;
    uintptr_t v = 0;
    if (!f)
        return 0;
    XPR_FIFO_LOCK(f);
    if (XPR_FifoIsEmpty(f)) {
        XPR_FIFO_UNLOCK(f);
        return 0;
    }
    n = f->tail % f->maxElements;
    XPR_AtomicInc(&f->tail);
    v = *(uintptr_t*)(f->data + n * f->elementSize);
    XPR_FIFO_UNLOCK(f);
    return v;
}

XPR_API int XPR_FifoPutAsAtomic(XPR_Fifo* f, uintptr_t data)
{
    int n = 0;
    if (!f)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_FIFO_LOCK(f);
    if (XPR_FifoIsFull(f)) {
        XPR_FIFO_UNLOCK(f);
        return XPR_ERR_GEN_NOBUF;
    }
    n = f->head % f->maxElements;
    XPR_AtomicInc(&f->head);
    *(uintptr_t*)(f->data + n * f->elementSize) = data;
    XPR_FIFO_UNLOCK(f);
    return XPR_ERR_OK;
}

// FIXME:
XPR_API uintptr_t XPR_FifoPeekAsAtomic(XPR_Fifo* f, int offset)
{
    return 0;
}

XPR_API int XPR_FifoDrain(XPR_Fifo* f, int size)
{
    if (!f || XPR_FifoIsEmpty(f))
        return -1;
    size = MIN(size, XPR_FifoGetLength(f));
    XPR_AtomicAdd(&f->tail, size);
    return size;
}

XPR_API void XPR_FifoReset(XPR_Fifo* f)
{
    if (f)
        f->head = f->tail = 0;
}

XPR_API int XPR_FifoGetAvailableSize(const XPR_Fifo* f)
{
    return f ? (f->maxElements - XPR_FifoGetLength(f)) : 0;
}

XPR_API int XPR_FifoGetElementSize(const XPR_Fifo* f)
{
    return f ? f->elementSize : 0;
}

XPR_API int XPR_FifoGetLength(const XPR_Fifo* f)
{
    if (!f)
        return 0;
    if (f->head < f->tail)
        return (int)(f->head + (XPR_ATOMIC_MAX - f->tail));
    return (int)(f->head - f->tail);
}

XPR_API int XPR_FifoGetSize(const XPR_Fifo* f)
{
    return f ? f->maxElements : 0;
}

XPR_API int XPR_FifoIsEmpty(const XPR_Fifo* f)
{
    return XPR_FifoGetLength(f) == 0;
}

XPR_API int XPR_FifoIsFull(const XPR_Fifo* f)
{
    return XPR_FifoGetLength(f) == XPR_FifoGetSize(f);
}
