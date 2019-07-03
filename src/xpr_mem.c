#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <xpr/xpr_mem.h>

XPR_API void* XPR_Alloc(size_t size)
{
    return malloc(size);
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
