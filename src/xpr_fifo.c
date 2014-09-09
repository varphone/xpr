#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32)
#include <crtdbg.h>
#include <malloc.h>
#include <intrin.h>
#endif
#include <string.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_utils.h>

struct XPR_Fifo {
    volatile unsigned int head;
    volatile unsigned int tail;
    unsigned int elementSize;
    unsigned int maxElements;
    unsigned char* data;
};

XPR_Fifo* XPR_FifoCreate(int elementSize, int maxElements)
{
    XPR_Fifo* f = (XPR_Fifo*)aligned_malloc(sizeof(*f), 8);
    if (f) {
        f->elementSize = elementSize;
        f->maxElements = maxElements;
        f->head = 0;
        f->tail = 0;
        f->data = (unsigned char*)aligned_malloc(elementSize * maxElements, 8);
        if (!f->data) {
            free((void*)f);
            f = 0;
        }
    }
    return f;
}

int XPR_FifoDestroy(XPR_Fifo* f)
{
    if (f && f->data)
        aligned_free((void*)f->data);
    if (f)
        aligned_free((void*)f);
    return 0;
}

int XPR_FifoGet(XPR_Fifo* f, void* buffer, int size)
{
    int l = 0;
    int n = 0;
    if (!f || !buffer || XPR_FifoIsEmpty(f))
        return -1;
    size = MIN(size, XPR_FifoGetLength(f));
    for (l = 0; l<size; l++) {
        n = f->tail % f->maxElements;
        memcpy((unsigned char*)buffer + l * f->elementSize,
            f->data + n * f->elementSize,
            f->elementSize);
        f->tail++;
    }
    return size;
}

int XPR_FifoPut(XPR_Fifo* f, const void* data, int size)
{
    int l = 0;
    int n = 0;
    if (!f || !data || XPR_FifoIsFull(f))
        return -1;
    size = MIN(size, XPR_FifoGetAvailableSize(f));
     for (l = 0; l<size; l++) {
        n = f->head % f->maxElements;
        memcpy(f->data + n * f->elementSize,
            (const unsigned char*)data + l * f->elementSize,
            f->elementSize);
        f->head++;
    }
    return size;
}

int XPR_FifoPeek(XPR_Fifo* f, void* buffer, int size, int offset)
{
    int l = 0;
    int n = 0;
    if (!f || !buffer || XPR_FifoIsEmpty(f))
        return -1;
    size = MIN(size, XPR_FifoGetLength(f) - offset);
    for (l = 0; l<size; l++) {
        n = (f->tail + offset + l) % f->maxElements;
        memcpy((unsigned char*)buffer + l * f->elementSize,
            f->data + n * f->elementSize,
            f->elementSize);
    }
    return size;
}

uintptr_t XPR_FifoGetAsAtomic(XPR_Fifo* f)
{
    int l = 0;
    int n = 0;
    uintptr_t* v = 0;
    if (!f || XPR_FifoIsEmpty(f))
        return 0;
    n = f->tail % f->maxElements;
    v = (uintptr_t*)(f->data + n * f->elementSize);
    f->tail++;
    (void)l;
    return *v;
}

int XPR_FifoPutAsAtomic(XPR_Fifo* f, uintptr_t data)
{
    int l = 0;
    int n = 0;
    uintptr_t* v = 0;
    if (!f || XPR_FifoIsFull(f))
        return -1;
    //printf("XPR_FifoPutAsAtomic %p, %d, %d\n", f, f->head, f->tail);
    n = f->head % f->maxElements;
    v = (uintptr_t*)(f->data + n * f->elementSize);
    f->head++;
    *v = data;
    (void)l;
    return 0;
}

int XPR_FifoDrain(XPR_Fifo* f, int size)
{
    if (!f || XPR_FifoIsEmpty(f))
        return -1;
    size = MIN(size, XPR_FifoGetLength(f));
    f->tail += size;
    return size;
}

void XPR_FifoReset(XPR_Fifo* f)
{
    if (f)
        f->head = f->tail = 0;
}

int XPR_FifoGetAvailableSize(const XPR_Fifo* f)
{
    return f ? (f->maxElements - XPR_FifoGetLength(f)) : 0;
}

int XPR_FifoGetElementSize(const XPR_Fifo* f)
{
    return f ? f->elementSize : 0;
}

int XPR_FifoGetLength(const XPR_Fifo* f)
{
    if (!f)
        return 0;
    if (f->head < f->tail)
        return f->head + (0xffffffff - f->tail);
    return f->head - f->tail;
}

int XPR_FifoGetSize(const XPR_Fifo* f)
{
    return f ? f->maxElements : 0;
}

int XPR_FifoIsEmpty(const XPR_Fifo* f)
{
    return XPR_FifoGetLength(f) == 0;
}

int XPR_FifoIsFull(const XPR_Fifo* f)
{
    return XPR_FifoGetLength(f) == XPR_FifoGetSize(f);
}

