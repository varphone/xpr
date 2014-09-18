#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_json.h>
#include <xpr/xpr_utils.h>

#define MAX_KEY_LEN  16
#define CHECK_KVS(k,v,s) \
        if(!k||!v||!s)\
            return -1

#define CHECK_KV(k,v) \
        if(!k || !v)\
            return -1

static XPR_UPS_Entry* root = 0;
static XPR_JSON*      root_json = 0;

//查找给定节点，成功返回节点指针，失败返回NULL.
//需要注意的是传进来的key要注意末尾是不是有'/',
//如/sysem/network/ /system/network是不一样的，前者表示目录，后者表示具体的项
int XPR_UPS_FindEntry(const char* key, XPR_JSON** json,
                      XPR_UPS_Entry** entry)
{
    int i = 0, j = 0, len = 0, leaf = 0, count = 0;
    char* saveptr, *s, *name;
    char* names[MAX_KEY_LEN];
    XPR_JSON* child_json = 0;
    XPR_JSON* parent_json = root_json;
    *json = NULL;
    *entry = NULL;
    XPR_UPS_Entry* p = root;
    if (strcmp(key, p->names[0]) == 0) {
        *json = root_json;
        *entry = p;
        return XPR_ERR_SUCCESS;
    }
    len = strlen(key);
    leaf = key[len - 1] == '/' ? 0 : 1 ;
    s = malloc(len + 1);
    if (!s)
        return XPR_ERR_UPS_NOMEM;
    s[len] = '\0';
    strcpy(s, key);
    name = strtok_r(s, "/", &saveptr);
    if (!name) {
        free(s);
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    }
    names[i++] = "/";
    while (name) {
        if (i >= MAX_KEY_LEN) {
            free(s);
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        }
        names[i++] = name;
        name = strtok_r(NULL, "/", &saveptr);
    }
    count = i;
    i = 0;
    while (p && i < count) {
        // 只有是叶子节点的时候，也就是最后一层的时候，才会有多个名字存在需要遍历，其他情况不需要
        if (leaf && (i == count - 1)) {
            child_json  =  XPR_JSON_ObjectGet(parent_json, names[i]);
            while (p) {
                for (j = 0, name = (char*)p->names[j]; name != 0;
                     j++, name = (char*)p->names[j]) {
                    if (strcmp(names[i], name) == 0 &&
                        p->type != XPR_UPS_ENTRY_TYPE_DIR) {
                        free(s);
                        *json = child_json;
                        *entry = p;
                        return XPR_ERR_SUCCESS;
                    }
                }
                p = p->next;
            }
            free(s);
            return XPR_ERR_UPS_UNEXIST;
        }
        // 树形的目录检索
        else if (strcmp(names[i], p->names[0]) == 0) {
            if (i != 0) {
                parent_json =  XPR_JSON_ObjectGet(parent_json, names[i]);
                child_json = parent_json;
            }
            if (++i == count) {
                free(s);
                *json = child_json;
                *entry = p;
                return XPR_ERR_SUCCESS;
            }
            p = p->subs;
        }
        // 检索他的兄弟节点
        else {
            child_json  =  XPR_JSON_ObjectGet(parent_json, names[i]);
            p = p->next;
        }
    }
    free(s);
    return XPR_ERR_UPS_UNEXIST;
}

static int XPR_UPS_GetData(const char* key, XPR_UPS_EntryType type,
                           void* buffer, int* size)
{
    int ret = 0;
    XPR_JSON* json = NULL;
    XPR_UPS_Entry* entry = NULL;
    if (!key || !buffer)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    if (type == XPR_UPS_ENTRY_TYPE_STRING && !size)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    ret = XPR_UPS_FindEntry(key, &json, &entry);
    if (XPR_ERR_SUCCESS != ret)
        return ret;
    if (entry->get)
        return entry->get(entry, json, key, buffer, size);
    return XPR_ERR_SUCCESS;
}

static int XPR_UPS_SetData(const char* key, XPR_UPS_EntryType type,
                           const void* data, int size)
{
    int ret = 0;
    XPR_JSON* json = NULL;
    XPR_UPS_Entry* entry = NULL;
    if (!key || !data)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    ret = XPR_UPS_FindEntry(key, &json, &entry);
    if (XPR_ERR_SUCCESS != ret)
        return ret;
    if (entry->set)
        return entry->set(entry, json, key, data, size);
    return XPR_ERR_SUCCESS;
}

void XPR_UPS_RegisterSingle(XPR_UPS_Entry* ent)
{
    int ret = 0;
    XPR_JSON* json = NULL;
    XPR_UPS_Entry* entry = NULL;
    if (!ent)
        return;
    if (root == 0) {
        root = ent;
        root->prev = root->next = root->subs = 0;
    }
    else {
        ret = XPR_UPS_FindEntry(ent->root, &json, &entry);
        if (XPR_ERR_SUCCESS != ret) {
            printf("cant not find:%s\n", ent->root);
            return;
        }
        if (!entry->subs) {
            entry->subs = ent;
        }
        else {
            entry = entry->subs;
            while (entry) {
                if (ent->type ==
                    XPR_UPS_ENTRY_TYPE_DIR  // 如果目录已存在则直接退出，不做挂载
                    && strcmp(entry->names[0], ent->names[0]) == 0)
                    return;
                if (!entry->next) {
                    entry->next = ent;
                    ent->prev = entry;
                    return;
                }
                entry = entry->next;
            }
        }
    }
}

extern XPR_UPS_Entry xpr_ups_driver_root;
extern XPR_UPS_Entry xpr_ups_driver_system_network[];
extern const int xpr_ups_driver_system_network_count;

extern XPR_UPS_Entry xpr_ups_driver_system_information[];
extern const int xpr_ups_driver_system_information_count;

extern XPR_UPS_Entry xpr_ups_driver_camera_image[];
extern const int xpr_ups_driver_camera_image_count;

static void XPR_UPS_RegisterAll(void)
{
    XPR_UPS_Register(&xpr_ups_driver_root, 1);
    XPR_UPS_Register(xpr_ups_driver_system_network,
                     xpr_ups_driver_system_network_count);
    XPR_UPS_Register(xpr_ups_driver_system_information,
                     xpr_ups_driver_system_information_count);
    XPR_UPS_Register(xpr_ups_driver_camera_image,
                     xpr_ups_driver_camera_image_count);
    // register other....
}

int XPR_UPS_Init(void)
{
    root_json = XPR_JSON_LoadFileName("./configuration.json");
    if (!root_json)
        return XPR_ERR_UPS_UNEXIST;
    XPR_UPS_RegisterAll();
    return XPR_ERR_SUCCESS;
}

int XPR_UPS_Fini(void)
{
    XPR_JSON_DumpFileName(root_json, "./configuration.json");
    XPR_JSON_DecRef(root_json);
    root_json = 0;
    return XPR_ERR_SUCCESS;
}

/// @brief
int XPR_UPS_Register(XPR_UPS_Entry ents[], int count)
{
    int i = 0;
    for (; i < count; i++)
        XPR_UPS_RegisterSingle(&ents[i]);
    return XPR_ERR_SUCCESS;
}

/// @brief
int XPR_UPS_UnRegister(XPR_UPS_Entry ents[], int count)
{
    return XPR_ERR_SUCCESS;
}

int XPR_UPS_SetString(const char* key, const char* value, int size)
{
    CHECK_KV(key, value);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

int XPR_UPS_SetStringVK(const char* value, int size, const char* key,
                        ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KVS(key, value, size);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_STRING, value,
                           size);
}


int XPR_UPS_GetString(const char* key, char* value, int* size)
{
    CHECK_KVS(key, value, size);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

int XPR_UPS_GetStringVK(char* value, int* size, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KVS(key, value, size);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_STRING, value,
                           size);
}


int XPR_UPS_SetInteger(const char* key, int value)
{
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_INT, &value, 0);
}

int XPR_UPS_SetIntegerVK(int value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_INT, &value, 0);
}

int XPR_UPS_GetInteger(const char* key, int* value)
{
    CHECK_KV(key, value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_INT, value, 0);
}

int XPR_UPS_GetIntegerVK(int* value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KV(key, value);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_INT, value, 0);
}

int XPR_UPS_SetInt64(const char* key, int64_t value)
{
    if (!key)
        return -1;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_INT64, &value, 0);
}

int XPR_UPS_SetInt64VK(int64_t value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return  XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_INT64, &value, 0);
}

int XPR_UPS_GetInt64(const char* key, int64_t* value)
{
    CHECK_KV(key, value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_INT64, value, 0);
}

int XPR_UPS_GetInt64VK(int64_t* value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KV(key, value);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_INT64, value, 0);
}

int XPR_UPS_SetFloat(const char* key, float value)
{
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}

int XPR_UPS_SetFloatVK(float value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}

int XPR_UPS_GetFloat(const char* key, float* value)
{
    CHECK_KV(key, value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_GetFloatVK(float* value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KV(key, value);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_SetDouble(const char* key, double value)
{
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}

int XPR_UPS_SetDoubleVK(double value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}


int XPR_UPS_GetDouble(const char* key, double* value)
{
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_GetDoubleVK(double* value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KV(key, value);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_SetBoolean(const char* key, int value)
{
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, &value, 0);
}

int XPR_UPS_SetBooleanVK(int value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    if (!key)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, &value, 0);
}

int XPR_UPS_GetBoolean(const char* key, int* value)
{
    CHECK_KV(key, value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, value, 0);
}

int XPR_UPS_GetBooleanVK(int* value, const char* key, ...)
{
    va_list ap;
    char buffer[1024];
    CHECK_KV(key, value);
    if (strlen(key) > 1024)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_BOOLEAN, value, 0);
}

int XPR_UPS_ReadData(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key,
                     void* buffer, int* size)
{
    int len = 0;
	int result = XPR_ERR_OK;
	const char* s = NULL;

	switch (ent->type) {
		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
			*(int*)buffer = XPR_JSON_BooleanValue(json);
			break;
		case XPR_UPS_ENTRY_TYPE_BLOB:
			// not support yet...
			break;
		case XPR_UPS_ENTRY_TYPE_INT:
			*(int*)buffer = XPR_JSON_IntegerValue(json);	
			break;
		case XPR_UPS_ENTRY_TYPE_INT64:
			*(int64_t*)buffer = XPR_JSON_Integer64Value(json);
			break;
		case XPR_UPS_ENTRY_TYPE_REAL:
			*(double*)buffer = XPR_JSON_RealValue(json);
			break;
		case XPR_UPS_ENTRY_TYPE_STRING:
			s = XPR_JSON_StringValue(json);
			len = strlen(s);
			if (len >= *size) {
				result = XPR_ERR_UPS_NOMEM;
				break;
			}	
			strcpy_s(buffer, *size, s);
			*size = len;
			break;
		default:
			return XPR_ERR_ERROR;
	}
	return result;
}

int XPR_UPS_WriteData(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key,
                      const void* data, int size)
{
	int result = XPR_ERR_OK;

	switch (ent->type) {
		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
            //result = XPR_JSON_BooleanSet(js, *(int*)buffer);
			break;
		case XPR_UPS_ENTRY_TYPE_BLOB:
			// not support yet...
			break;
		case XPR_UPS_ENTRY_TYPE_INT:
			result = XPR_JSON_IntegerSet(json, *(int*)data);	
			break;
		case XPR_UPS_ENTRY_TYPE_INT64:
			result = XPR_JSON_Integer64Set(json, *(int64_t*)data);
			break;
		case XPR_UPS_ENTRY_TYPE_REAL:
			result = XPR_JSON_RealSet(json, *(double*)data);
			break;
		case XPR_UPS_ENTRY_TYPE_STRING:
			result = XPR_JSON_StringSet(json, (char *)data);
			break;
		default:
			return XPR_ERR_ERROR;
	}
	// TODO: save to file
	return result;
}

int XPR_UPS_Delete(const char* key)
{
    return XPR_ERR_OK;
}

int XPR_UPS_Exists(const char* key)
{
    return XPR_FALSE;
}

const char* XPR_UPS_FirstKey(void)
{
    return 0;
}

const char* XPR_UPS_NextKey(const char* key)
{
    return 0;
}

void XPR_UPS_BeginGroup(const char* group)
{
}

void XPR_UPS_EndGroup(const char* group)
{
}

int XPR_UPS_Export(const char* url)
{
    return XPR_ERR_OK;
}

int XPR_UPS_Import(const char* url)
{
    return XPR_ERR_OK;
}

int XPR_UPS_Pack(void)
{
    return XPR_ERR_OK;
}

int XPR_UPS_Sync(void)
{
    return XPR_ERR_OK;
}

