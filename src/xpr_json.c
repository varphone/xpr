#include "deps/jansson/jansson.h"
#include <xpr/xpr_json.h>

struct XPR_JSON {
    json_t json;
};

#ifdef PACKAGE_VERSION
#  undef PACKAGE_VERSION
#endif
#if defined(_MSC_VER)
#  define PACKAGE_VERSION "1.1.1 ("##__DATE__##" "##__TIME__##")"
#else
#  define PACKAGE_VERSION "1.1.1 (" __DATE__ " " __TIME__ ")"
#endif

const char* XPR_JSON_Version(void)
{
    return PACKAGE_VERSION;
}

int XPR_JSON_VersionNumber(void)
{
    return XPR_JSON_VERSION;
}

void* XPR_JSON_Alloc(size_t size)
{
    return malloc(size);
}

void XPR_JSON_Free(void* ptr)
{
    if (ptr)
        free(ptr);
}

XPR_JSON* XPR_JSON_IncRef(XPR_JSON* json)
{
    if (!json)
        return 0;
    return (XPR_JSON*)json_incref((json_t*)json);
}

void XPR_JSON_DecRef(XPR_JSON* json)
{
    if (json)
        json_decref((json_t*)json);
}

int XPR_JSON_RefCount(XPR_JSON* json)
{
    if (!json)
        return -1;
    return (int)((json_t*)json)->refcount;
}


XPR_JSON* XPR_JSON_LoadString(const char* text)
{
    if (!text)
        return 0;
    return (XPR_JSON*)json_loads(text, 0, 0);
}

XPR_JSON* XPR_JSON_LoadFileName(const char* fileName)
{
    json_error_t je;
    if (!fileName)
        return 0;

    return (XPR_JSON*)json_load_file(fileName, 0, &je);
}

char* XPR_JSON_DumpString(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_dumps((json_t*)json, JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_SORT_KEYS);
}

int XPR_JSON_DumpFileName(XPR_JSON* json, const char* fileName)
{
    if (!json || !fileName)
        return -1;
    return json_dump_file((json_t*)json, fileName, JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_SORT_KEYS);
}

int XPR_JSON_DumpFileStream(XPR_JSON* json, FILE* fileStream)
{
    if (!json || !fileStream)
        return -1;
    return json_dumpf((json_t*)json, fileStream, JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_SORT_KEYS);
}

XPR_JSON_TYPE XPR_JSON_Typeof(XPR_JSON* json)
{
    if (!json)
        return XPR_JSON_UNDEFINED;
    return (XPR_JSON_TYPE)json_typeof((json_t*)json);
}

int XPR_JSON_IsObject(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_object((json_t*)json);
}

int XPR_JSON_IsArray(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_array((json_t*)json);
}

int XPR_JSON_IsString(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_string((json_t*)json);
}

int XPR_JSON_IsInteger(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_integer((json_t*)json);
}

int XPR_JSON_IsReal(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_real((json_t*)json);
}

int XPR_JSON_IsTrue(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_true((json_t*)json);
}

int XPR_JSON_IsFalse(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_false((json_t*)json);
}

int XPR_JSON_IsNull(XPR_JSON* json)
{
    return json_is_null((json_t*)json);
}

// Object
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_Object(void)
{
    return (XPR_JSON*)json_object();
}

int XPR_JSON_ObjectSet(XPR_JSON* json, const char* key, XPR_JSON* val)
{
    if (!json)
        return 0;
    return json_object_set((json_t*)json, key, (json_t*)val);
}

int XPR_JSON_ObjectSetNew(XPR_JSON* json, const char* key, XPR_JSON* val)
{
    if (!json || !key)
        return 0;
    return json_object_set_new((json_t*)json, key, (json_t*)val);
}

XPR_JSON* XPR_JSON_ObjectGet(XPR_JSON* json, const char* key)
{
    if (!json || !key)
        return 0;
    return (XPR_JSON*)json_object_get((json_t*)json, key);
}

size_t XPR_JSON_ObjectSize(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_object_size((json_t*)json);
}

void* XPR_JSON_ObjectIter(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_object_iter((json_t*)json);
}

void* XPR_JSON_ObjectIterAt(XPR_JSON* json, const char* key)
{
    if (!json || !key)
        return 0;
    return json_object_iter_at((json_t*)json, key);
}

void* XPR_JSON_ObjectIterNext(XPR_JSON* json, void* iter)
{
    if (!json || !iter)
        return 0;
    return json_object_iter_next((json_t*)json, iter);
}

const char* XPR_JSON_ObjectIterKey(void* iter)
{
    if (!iter)
        return 0;
    return json_object_iter_key(iter);
}

XPR_JSON* XPR_JSON_ObjectIterValue(void* iter)
{
    if (!iter)
        return 0;
    return (XPR_JSON*)json_object_iter_value(iter);
}

int XPR_JSON_ObjectIterSet(XPR_JSON* json, void* iter, XPR_JSON* val)
{
    if (!json || !iter)
        return -1;
    return json_object_iter_set((json_t*)json, iter, (json_t*)val);
}

int XPR_JSON_ObjectIterSetNew(XPR_JSON* json, void* iter, XPR_JSON* val)
{
    if (!json || !iter)
        return -1;
    return json_object_iter_set_new((json_t*)json, iter, (json_t*)val);
}

// Array
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_Array(void)
{
    return (XPR_JSON*)json_array();
}

int XPR_JSON_ArraySet(XPR_JSON* json, size_t index, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_set((json_t*)json, index, (json_t*)val);
}

int XPR_JSON_ArraySetNew(XPR_JSON* json, size_t index, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_set_new((json_t*)json, index, (json_t*)val);
}

XPR_JSON* XPR_JSON_ArrayGet(XPR_JSON* json, size_t index)
{
    if (!json)
        return 0;
    return (XPR_JSON*)json_array_get((json_t*)json, index);
}

size_t XPR_JSON_ArraySize(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_array_size((json_t*)json);
}

int XPR_JSON_ArrayAppend(XPR_JSON* json, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_append((json_t*)json, (json_t*)val);
}

int XPR_JSON_ArrayAppendNew(XPR_JSON* json, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_append_new((json_t*)json, (json_t*)val);
}

// String
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_String(const char* val)
{
    return (XPR_JSON*)json_string(val);
}

int XPR_JSON_StringSet(XPR_JSON* json, const char* val)
{
    if (!json)
        return -1;
    return json_string_set_nocheck((json_t*)json, val);
}

const char* XPR_JSON_StringValue(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_string_value((json_t*)json);
}

// Integer
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_Integer(int val)
{
    return (XPR_JSON*)json_integer(val);
}

XPR_JSON* XPR_JSON_Integer64(int64_t val)
{
    return (XPR_JSON*)json_integer(val);
}

int XPR_JSON_IntegerSet(XPR_JSON* json, int val)
{
    if (!json)
        return -1;
    return json_integer_set((json_t*)json, val);
}

int XPR_JSON_Integer64Set(XPR_JSON* json, int64_t val)
{
    if (!json)
        return -1;
    return json_integer_set((json_t*)json, val);
}

int XPR_JSON_IntegerValue(XPR_JSON* json)
{
    if (!json)
        return 0;
    return (int)json_integer_value((json_t*)json);
}

int64_t XPR_JSON_Integer64Value(XPR_JSON* json)
{
    if (!json)
        return 0;
    return (int64_t)json_integer_value((json_t*)json);
}

// Real
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_Real(double val)
{
    return (XPR_JSON*)json_real(val);
}

int XPR_JSON_RealSet(XPR_JSON* json, double val)
{
    if (!json)
        return -1;
    return json_real_set((json_t*)json, val);
}

double XPR_JSON_RealValue(XPR_JSON* json)
{
    if (!json)
        return 0.0;
    return json_real_value((json_t*)json);
}

// Boolean
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_True(void)
{
    return (XPR_JSON*)json_true();
}

int XPR_JSON_TrueValue(XPR_JSON* json)
{
    return 1;
}

XPR_JSON* XPR_JSON_False(void)
{
    return (XPR_JSON*)json_false();
}

int XPR_JSON_FalseValue(XPR_JSON* json)
{
    return 0;
}

XPR_JSON* XPR_JSON_Boolean(int val)
{
    if (val)
        return (XPR_JSON*)json_true();
    else
        return (XPR_JSON*)json_false();
}

int XPR_JSON_BooleanValue(XPR_JSON* json)
{
    if (XPR_JSON_Typeof(json) == XPR_JSON_TRUE)
        return 1;
    if (XPR_JSON_Typeof(json) == XPR_JSON_FALSE)
        return 0;
    return -1;
}

// Null
////////////////////////////////////////////////////////////////////////////////
XPR_JSON* XPR_JSON_Null(void)
{
    return (XPR_JSON*)json_null();
}

void* XPR_JSON_NullValue(XPR_JSON* json)
{
    return NULL;
}

