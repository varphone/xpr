#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <xpr/xpr_mem.h>

void* XPR_Alloc(size_t size)
{
    return malloc(size);
}

void XPR_Free(void* ptr)
{
    if(ptr) {
		free(ptr);
		ptr = NULL;
	}
}

void XPR_Freep(void** pptr)
{
    if (pptr) {
        if (*pptr) {
            XPR_Free(*pptr);
            *pptr = NULL;
        }
    }
}

void XPR_Freev(void** vptr)
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

char* XPR_StrDup(const char* str)
{
#if defined(_MSC_VER)
    return _strdup(str);
#else
    return strdup(str);
#endif
}

wchar_t* XPR_StrDupW(const wchar_t* str)
{
#if defined(_MSC_VER)
    return _wcsdup(str);
#else
    return wcsdup(str);
#endif
}

