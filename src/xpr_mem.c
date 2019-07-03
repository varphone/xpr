#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_utils.h>

#define kRcHeadMarker 0x52434D4D // 'RCMM'

typedef struct RcHead {
    unsigned int marker;
    XPR_Atomic refcount;
    XPR_Atomic stuff[4];
} RcHead;

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
