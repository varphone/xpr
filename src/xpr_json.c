#include "deps/jansson/jansson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_json.h>
#include <xpr/xpr_utils.h>

struct XPR_JSON {
    json_t json;
};

#ifdef PACKAGE_VERSION
#  undef PACKAGE_VERSION
#endif
#if defined(_MSC_VER)
#  define PACKAGE_VERSION "1.6.1 ("##__DATE__##" "##__TIME__##")"
#else
#  define PACKAGE_VERSION "1.6.1 (" __DATE__ " " __TIME__ ")"
#endif

XPR_API const char* XPR_JSON_Version(void)
{
    return PACKAGE_VERSION;
}

XPR_API int XPR_JSON_VersionNumber(void)
{
    return XPR_JSON_VERSION;
}

XPR_API void* XPR_JSON_Alloc(size_t size)
{
    return malloc(size);
}

XPR_API void XPR_JSON_Free(void* ptr)
{
    if (ptr)
        free(ptr);
}

XPR_API XPR_JSON* XPR_JSON_IncRef(XPR_JSON* json)
{
    if (!json)
        return 0;
    return (XPR_JSON*)json_incref((json_t*)json);
}

XPR_API void XPR_JSON_DecRef(XPR_JSON* json)
{
    if (json)
        json_decref((json_t*)json);
}

XPR_API int XPR_JSON_RefCount(XPR_JSON* json)
{
    if (!json)
        return -1;
    return (int)((json_t*)json)->refcount;
}

XPR_API XPR_JSON* XPR_JSON_LoadString(const char* text)
{
    if (!text)
        return 0;
    return (XPR_JSON*)json_loads(text, 0, 0);
}

XPR_API XPR_JSON* XPR_JSON_LoadFileName(const char* fileName)
{
    json_error_t je;
    if (!fileName)
        return 0;

    return (XPR_JSON*)json_load_file(fileName, 0, &je);
}

XPR_API char* XPR_JSON_DumpString(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_dumps((json_t*)json, JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_SORT_KEYS);
}

XPR_API int XPR_JSON_DumpFileName(XPR_JSON* json, const char* fileName)
{
    if (!json || !fileName)
        return -1;
    return json_dump_file((json_t*)json, fileName, JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_SORT_KEYS);
}

XPR_API int XPR_JSON_DumpFileStream(XPR_JSON* json, FILE* fileStream)
{
    if (!json || !fileStream)
        return -1;
    return json_dumpf((json_t*)json, fileStream, JSON_ENCODE_ANY | JSON_INDENT(4) | JSON_SORT_KEYS);
}

XPR_API XPR_JSON_TYPE XPR_JSON_Typeof(XPR_JSON* json)
{
    if (!json)
        return XPR_JSON_UNDEFINED;
    return (XPR_JSON_TYPE)json_typeof((json_t*)json);
}

XPR_API int XPR_JSON_IsObject(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_object((json_t*)json);
}

XPR_API int XPR_JSON_IsArray(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_array((json_t*)json);
}

XPR_API int XPR_JSON_IsString(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_string((json_t*)json);
}

XPR_API int XPR_JSON_IsInteger(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_integer((json_t*)json);
}

XPR_API int XPR_JSON_IsReal(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_real((json_t*)json);
}

XPR_API int XPR_JSON_IsTrue(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_true((json_t*)json);
}

XPR_API int XPR_JSON_IsFalse(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_is_false((json_t*)json);
}

XPR_API int XPR_JSON_IsNull(XPR_JSON* json)
{
    return json_is_null((json_t*)json);
}

// Object
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_Object(void)
{
    return (XPR_JSON*)json_object();
}

XPR_API int XPR_JSON_ObjectSet(XPR_JSON* json, const char* key, XPR_JSON* val)
{
    if (!json)
        return 0;
    return json_object_set((json_t*)json, key, (json_t*)val);
}

XPR_API int XPR_JSON_ObjectSetNew(XPR_JSON* json, const char* key, XPR_JSON* val)
{
    if (!json || !key)
        return 0;
    return json_object_set_new((json_t*)json, key, (json_t*)val);
}

XPR_API XPR_JSON* XPR_JSON_ObjectGet(XPR_JSON* json, const char* key)
{
    if (!json || !key)
        return 0;
    return (XPR_JSON*)json_object_get((json_t*)json, key);
}

XPR_API size_t XPR_JSON_ObjectSize(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_object_size((json_t*)json);
}

XPR_API int XPR_JSON_ObjectRemove(XPR_JSON* json, const char* key)
{
    if (!json || !key)
        return -1;
    return json_object_del((json_t*)json, key);
}

XPR_API int XPR_JSON_ObjectClear(XPR_JSON* json)
{
    if (!json)
        return -1;
    return json_object_clear((json_t*)json);
}

XPR_API void* XPR_JSON_ObjectIter(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_object_iter((json_t*)json);
}

XPR_API void* XPR_JSON_ObjectIterAt(XPR_JSON* json, const char* key)
{
    if (!json || !key)
        return 0;
    return json_object_iter_at((json_t*)json, key);
}

XPR_API void* XPR_JSON_ObjectIterNext(XPR_JSON* json, void* iter)
{
    if (!json || !iter)
        return 0;
    return json_object_iter_next((json_t*)json, iter);
}

XPR_API const char* XPR_JSON_ObjectIterKey(void* iter)
{
    if (!iter)
        return 0;
    return json_object_iter_key(iter);
}

XPR_API XPR_JSON* XPR_JSON_ObjectIterValue(void* iter)
{
    if (!iter)
        return 0;
    return (XPR_JSON*)json_object_iter_value(iter);
}

XPR_API int XPR_JSON_ObjectIterSet(XPR_JSON* json, void* iter, XPR_JSON* val)
{
    if (!json || !iter)
        return -1;
    return json_object_iter_set((json_t*)json, iter, (json_t*)val);
}

XPR_API int XPR_JSON_ObjectIterSetNew(XPR_JSON* json, void* iter, XPR_JSON* val)
{
    if (!json || !iter)
        return -1;
    return json_object_iter_set_new((json_t*)json, iter, (json_t*)val);
}

// Array
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_Array(void)
{
    return (XPR_JSON*)json_array();
}

XPR_API int XPR_JSON_ArraySet(XPR_JSON* json, size_t index, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_set((json_t*)json, index, (json_t*)val);
}

XPR_API int XPR_JSON_ArraySetNew(XPR_JSON* json, size_t index, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_set_new((json_t*)json, index, (json_t*)val);
}

XPR_API XPR_JSON* XPR_JSON_ArrayGet(XPR_JSON* json, size_t index)
{
    if (!json)
        return 0;
    return (XPR_JSON*)json_array_get((json_t*)json, index);
}

XPR_API size_t XPR_JSON_ArraySize(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_array_size((json_t*)json);
}

XPR_API int XPR_JSON_ArrayAppend(XPR_JSON* json, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_append((json_t*)json, (json_t*)val);
}

XPR_API int XPR_JSON_ArrayAppendNew(XPR_JSON* json, XPR_JSON* val)
{
    if (!json)
        return -1;
    return json_array_append_new((json_t*)json, (json_t*)val);
}

XPR_API int XPR_JSON_ArrayInsertNew(XPR_JSON* json, size_t index, XPR_JSON* val)
{
	if (!json || !val)
		return -1;
	return json_array_insert_new((json_t*)json, index, (json_t*)val);
}

XPR_API int XPR_JSON_ArrayRemove(XPR_JSON* json, size_t index)
{
    if (!json)
        return -1;
    return json_array_remove((json_t*)json, index);
}

XPR_API int XPR_JSON_ArrayClear(XPR_JSON* json)
{
    if (!json)
        return -1;
    return json_array_clear((json_t*)json);
}

// String
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_String(const char* val)
{
    return (XPR_JSON*)json_string(val);
}

XPR_API int XPR_JSON_StringSet(XPR_JSON* json, const char* val)
{
    if (!json)
        return -1;
    return json_string_set_nocheck((json_t*)json, val);
}

XPR_API const char* XPR_JSON_StringValue(XPR_JSON* json)
{
    if (!json)
        return 0;
    return json_string_value((json_t*)json);
}

// Integer
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_Integer(int val)
{
    return (XPR_JSON*)json_integer(val);
}

XPR_API XPR_JSON* XPR_JSON_Integer64(int64_t val)
{
    return (XPR_JSON*)json_integer(val);
}

XPR_API int XPR_JSON_IntegerSet(XPR_JSON* json, int val)
{
    if (!json)
        return -1;
    return json_integer_set((json_t*)json, val);
}

XPR_API int XPR_JSON_Integer64Set(XPR_JSON* json, int64_t val)
{
    if (!json)
        return -1;
    return json_integer_set((json_t*)json, val);
}

XPR_API int XPR_JSON_IntegerValue(XPR_JSON* json)
{
    if (!json)
        return 0;
    return (int)json_integer_value((json_t*)json);
}

XPR_API int64_t XPR_JSON_Integer64Value(XPR_JSON* json)
{
    if (!json)
        return 0;
    return (int64_t)json_integer_value((json_t*)json);
}

// Real
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_Real(double val)
{
    return (XPR_JSON*)json_real(val);
}

XPR_API int XPR_JSON_RealSet(XPR_JSON* json, double val)
{
    if (!json)
        return -1;
    return json_real_set((json_t*)json, val);
}

XPR_API double XPR_JSON_RealValue(XPR_JSON* json)
{
    if (!json)
        return 0.0;
    return json_real_value((json_t*)json);
}

// Boolean
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_True(void)
{
    return (XPR_JSON*)json_true();
}

XPR_API int XPR_JSON_TrueValue(XPR_JSON* json)
{
    return 1;
}

XPR_API XPR_JSON* XPR_JSON_False(void)
{
    return (XPR_JSON*)json_false();
}

XPR_API int XPR_JSON_FalseValue(XPR_JSON* json)
{
    return 0;
}

XPR_API XPR_JSON* XPR_JSON_Boolean(int val)
{
    if (val)
        return (XPR_JSON*)json_true();
    else
        return (XPR_JSON*)json_false();
}

XPR_API int XPR_JSON_BooleanValue(XPR_JSON* json)
{
    if (XPR_JSON_Typeof(json) == XPR_JSON_TRUE)
        return 1;
    if (XPR_JSON_Typeof(json) == XPR_JSON_FALSE)
        return 0;
    return -1;
}

// Null
////////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_Null(void)
{
    return (XPR_JSON*)json_null();
}

XPR_API void* XPR_JSON_NullValue(XPR_JSON* json)
{
    return NULL;
}

// Copy
///////////////////////////////////////////////////////////////////////////////
XPR_API XPR_JSON* XPR_JSON_Copy(XPR_JSON* src)
{
    return (XPR_JSON*)json_copy((json_t*)src);
}

XPR_API XPR_JSON* XPR_JSON_DeepCopy(XPR_JSON* src)
{
    return (XPR_JSON*)json_deep_copy((const json_t*)src);
}

struct xpath_scan {
	int			created;
	int			array_index;
	char		last_key[256];
	XPR_JSON*	root;
	XPR_JSON*	target;
	XPR_JSON*	value;
};

static void __xpath(void* opaque, char* key)
{
	int 			i  = 0;
	int 			ai = -1;	// 数组索引
	char* 			bb = NULL;	// 数组中括号起始位置
	struct xpath_scan* 	xps = (struct xpath_scan*)opaque;

	if (!xps || !key)
		return;

	strcpy_s(xps->last_key, sizeof(xps->last_key), key);

	bb = strchr(xps->last_key, '[');
	if (bb) {
		*bb = 0;
		ai = strtol(++bb, NULL, 10);
		xps->array_index = ai;
	}

	if (xps->last_key[0] == 0) {
		xps->target = xps->root;
	}
	else {
		if (xps->target)
			xps->root = xps->target;

		xps->target = XPR_JSON_ObjectGet(xps->root, (const char*)xps->last_key);
		// 
		if (bb && ai != -1) {
			if (xps->created && xps->target == NULL) {
				XPR_JSON_ObjectSetNew(xps->root, (const char*)xps->last_key, XPR_JSON_Array());
				xps->target = XPR_JSON_ObjectGet(xps->root, (const char*)xps->last_key);
			}
			xps->root = xps->target;
			xps->target = XPR_JSON_ArrayGet(xps->root, ai);
			if (xps->created && xps->target == NULL) {
				for (i = 0; i <= ai; i++) {
					if (XPR_JSON_ArrayGet(xps->root, i) == NULL)
						XPR_JSON_ArrayInsertNew(xps->root, i, XPR_JSON_Object());
				}
				xps->target = XPR_JSON_ArrayGet(xps->root, ai);
			}
		}
		else {
			if (xps->created && xps->target == NULL) {
				XPR_JSON_ObjectSetNew(xps->root, xps->last_key, XPR_JSON_Object());
			}
			xps->target = XPR_JSON_ObjectGet(xps->root, (const char*)xps->last_key);
		}
	}
}

XPR_API XPR_JSON* XPR_JSON_XPathGet(XPR_JSON* json, const char* xpath)
{
	struct xpath_scan xps;
	xps.created = 0;
	xps.root = json;
	xps.target = NULL;
	xpr_foreach_s(xpath, -1, "/", __xpath, &xps);
	return xps.target;
}

XPR_API int XPR_JSON_XPathSet(XPR_JSON* json, const char* xpath, XPR_JSON* value)
{
	struct xpath_scan xps;
	xps.created = 1;
	xps.root = json;
	xps.target = NULL;
	xpr_foreach_s(xpath, -1, "/", __xpath, &xps);
	if (xps.target && xps.last_key[0] != 0) {
		if (XPR_JSON_IsArray(xps.root))
			return XPR_JSON_ArraySet(xps.root, xps.array_index, value);
		return XPR_JSON_ObjectSet(xps.root, xps.last_key, value);
	}
	return XPR_ERR_ERROR;
}

XPR_API int XPR_JSON_XPathSetNew(XPR_JSON* json, const char* xpath, XPR_JSON* value)
{
	struct xpath_scan xps;
	xps.created = 1;
	xps.root = json;
	xps.target = NULL;
	xpr_foreach_s(xpath, -1, "/", __xpath, &xps);
	if (xps.target && xps.last_key[0] != 0) {
		if (XPR_JSON_IsArray(xps.root))
			return XPR_JSON_ArraySetNew(xps.root, xps.array_index, value);
		return XPR_JSON_ObjectSetNew(xps.root, xps.last_key, value);
	}
	return XPR_ERR_ERROR;
}

XPR_API int XPR_JSON_XPathIsNull(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return XPR_JSON_IsNull(jx);
}

XPR_API int XPR_JSON_XPathGetBoolean(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return XPR_JSON_BooleanValue(jx);
}

XPR_API int XPR_JSON_XPathSetBoolean(XPR_JSON* json, const char* xpath, int value)
{
	return XPR_JSON_XPathSetNew(json, xpath, value ? XPR_JSON_True() : XPR_JSON_False());
}

XPR_API int XPR_JSON_XPathGetInt(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return XPR_JSON_IntegerValue(jx);
}

XPR_API int XPR_JSON_XPathSetInt(XPR_JSON* json, const char* xpath, int value)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath); 
	if (jx)
		return XPR_JSON_IntegerSet(jx, value);
	return XPR_JSON_XPathSetNew(json, xpath, XPR_JSON_Integer(value));
}

XPR_API int64_t XPR_JSON_XPathGetInt64(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return XPR_JSON_Integer64Value(jx);
}

XPR_API int XPR_JSON_XPathSetInt64(XPR_JSON* json, const char* xpath, int64_t value)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	if (jx)
		return XPR_JSON_Integer64Set(jx, value);
	return XPR_JSON_XPathSetNew(json, xpath, XPR_JSON_Integer64(value));
}

XPR_API float XPR_JSON_XPathGetFloat(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return (float)XPR_JSON_RealValue(jx);
}

XPR_API int XPR_JSON_XPathSetFloat(XPR_JSON* json, const char* xpath, float value)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	if (jx)
		return XPR_JSON_RealSet(jx, value);
	return XPR_JSON_XPathSetNew(json, xpath, XPR_JSON_Real(value));
}

XPR_API double XPR_JSON_XPathGetDouble(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return XPR_JSON_RealValue(jx);
}

XPR_API int XPR_JSON_XPathSetDouble(XPR_JSON* json, const char* xpath, double value)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	if (jx)
		return XPR_JSON_RealSet(jx, value);
	return XPR_JSON_XPathSetNew(json, xpath, XPR_JSON_Real(value));
}

XPR_API double XPR_JSON_XPathGetNumber(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	if (XPR_JSON_IsTrue(jx) || XPR_JSON_IsFalse(jx))
		return XPR_JSON_BooleanValue(jx);
	if (XPR_JSON_IsInteger(jx))
		return XPR_JSON_IntegerValue(jx);
	if (XPR_JSON_IsReal(jx))
		return XPR_JSON_RealValue(jx);
	if (XPR_JSON_IsString(jx))
		return strtod(XPR_JSON_StringValue(jx), NULL);
	return 0.0;
}

XPR_API const char* XPR_JSON_XPathGetString(XPR_JSON* json, const char* xpath)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	return XPR_JSON_StringValue(jx);
}

XPR_API int XPR_JSON_XPathSetString(XPR_JSON* json, const char* xpath, const char* value)
{
	XPR_JSON* jx = XPR_JSON_XPathGet(json, xpath);
	if (jx)
		return XPR_JSON_StringSet(jx, value);
	return XPR_JSON_XPathSetNew(json, xpath, XPR_JSON_String(value));
}
