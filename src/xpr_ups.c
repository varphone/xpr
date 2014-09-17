#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_json.h>

#define MAX_KEY_LEN	 16
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
XPR_UPS_Entry *XPR_UPS_FindEntry(const char* key, XPR_JSON** json)
{
	int i=0, j=0, len=0, leaf=0, count=0;
	char* saveptr, *s, *name ;
	char* names[MAX_KEY_LEN];
	XPR_JSON *child_json = 0;
	XPR_JSON *parent_json = root_json;

	*json = NULL;
	XPR_UPS_Entry* p = root;
	if(strcmp(key, p->names[0]) ==0) {
		*json = root_json;
		return p;
	}
	len = strlen(key);
	leaf = key[len - 1] == '/' ? 0 : 1 ;
			
	s = malloc(len+1);
	if(!s) 
		return NULL;
	s[len] = '\0';
	strcpy(s, key);

	
	name = strtok_r(s, "/", &saveptr);	
	if(!name) {
		free(s);
		return NULL;
	}
	names[i++] = "/";
	while(name) {
		if(i>=MAX_KEY_LEN) {
			free(s);
			return NULL;
		}
		names[i++] = name;
		name = strtok_r(NULL, "/", &saveptr);
	}
	count = i;
	i = 0;

	while(p && i<count) {
		// 只有是叶子节点的时候，也就是最后一层的时候，才会有多个名字存在需要遍历，其他情况不需要
		if(leaf && (i == count-1)) {
			child_json  =  XPR_JSON_ObjectGet(parent_json, names[i]);
			while(p) {
				for(j = 0, name = (char*)p->names[j];name != 0; j++, name = (char*)p->names[j]) {
					if(strcmp(names[i], name) ==0 && p->type != XPR_UPS_ENTRY_TYPE_DIR) {
						free(s);
						*json = child_json;
						return p;
					}
				}
				p = p->next;
			}
			free(s);
			return NULL;
		}
		// 树形的目录检索
		else if(strcmp(names[i], p->names[0]) == 0) {
			if(i!=0) {
				parent_json =  XPR_JSON_ObjectGet(parent_json, names[i]);
				child_json = parent_json;
			}

			if(++i == count) {
				free(s);
				*json = child_json;
				return p; 
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
	return NULL;
}

static int XPR_UPS_GetData(const char* key, XPR_UPS_EntryType type, void* buffer, int* size)
{	
	XPR_JSON *json = NULL;
	XPR_UPS_Entry *entry=NULL;

	if(!key || !buffer) 
		return -1;

	if(type == XPR_UPS_ENTRY_TYPE_STRING && !size)
		return -1;
		

	entry = XPR_UPS_FindEntry(key, &json);
	if(!entry || !json) 
		return -1;

	if(entry->get) 
		return entry->get(entry, json, key, buffer, size);

	return 0;
}

static int XPR_UPS_SetData(const char* key, XPR_UPS_EntryType type, const void* data, int size)
{	
	XPR_JSON *json = NULL;
	XPR_UPS_Entry *entry=NULL;

	if(!key || !data) 
		return -1;

	entry = XPR_UPS_FindEntry(key, &json);
	if(!entry || !json)
		return -1;

	if(entry->set) 
		return entry->set(entry, json, key, data, size);
	return 0;
}

void XPR_UPS_RegisterSingle(XPR_UPS_Entry* ent)
{
	XPR_JSON *json = NULL;
	XPR_UPS_Entry* entry = NULL;

	if(!ent)
		return;

    if (root == 0) {
        root = ent;
        root->prev = root->next = root->subs = 0;
    }
    else {
		entry = XPR_UPS_FindEntry(ent->root, &json);
		if(!entry) {
			printf("cant not find:%s\n",ent->root);
			return;
		}
		if(!entry->subs) {
			entry->subs = ent;
		}
		else {
			entry = entry->subs;
			while(entry) {
				if(ent->type == XPR_UPS_ENTRY_TYPE_DIR   // 如果目录已存在则直接退出，不做挂载
					&& strcmp(entry->names[0], ent->names[0])==0)
					return;

				if(!entry->next) {
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
	XPR_UPS_Register(xpr_ups_driver_system_network,  xpr_ups_driver_system_network_count);
    XPR_UPS_Register(xpr_ups_driver_system_information, xpr_ups_driver_system_information_count);
    XPR_UPS_Register(xpr_ups_driver_camera_image, xpr_ups_driver_camera_image_count);
	// register other....
}

int XPR_UPS_Init(void)
{ 
	root_json = XPR_JSON_LoadFileName("./configuration.json");
 	if(!root_json) 
		return XPR_ERR_UPS_UNEXIST;

    XPR_UPS_RegisterAll();
    return 0;
}

int XPR_UPS_Fini(void)
{
	XPR_JSON_DumpFileName(root_json, "./configuration.json");
	XPR_JSON_DecRef(root_json);
	root_json=0;
    return 0;
}

/// @brief
int XPR_UPS_Register(XPR_UPS_Entry ents[], int count)
{
    int i = 0;
    for (; i<count; i++)
        XPR_UPS_RegisterSingle(&ents[i]);
    return -1;
}

/// @brief
int XPR_UPS_UnRegister(XPR_UPS_Entry ents[], int count)
{
	return 0;
}

int XPR_UPS_SetString(const char* key, const char* value, int size)
{
	CHECK_KV(key, value);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

int XPR_UPS_SetStringVK(const char* value, int size, const char* key, ...)
{
	va_list ap;
    char buffer[1024];

	CHECK_KVS(key, value, size);
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_STRING, value, size);
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
	if(strlen(key) > 1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}


int XPR_UPS_SetInteger(const char* key, int value)
{
	if(!key)
		return -1;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_INT, &value, 0);
}

int XPR_UPS_SetIntegerVK(int value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	if(!key)
		return -1;

	if(strlen(key)>1024)
		return -1;

	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_INT, &value, 0);
	
}

int XPR_UPS_GetInteger(const char* key, int* value)
{
	CHECK_KV(key,value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_INT, value, 0);
}

int XPR_UPS_GetIntegerVK(int *value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	CHECK_KV(key,value);
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_INT, value, 0);
}

int XPR_UPS_SetInt64(const char* key, int64_t value)
{
	if(!key)
		return -1;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_INT64, &value, 0);
}

int XPR_UPS_SetInt64VK(int64_t value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];
	if(!key)
		return -1;
	if(strlen(key)>1024)
		return -1;
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

	CHECK_KV(key,value);
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_INT64, value, 0);
}

int XPR_UPS_SetFloat(const char* key, float value)
{
	if(!key)
		return -1;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}

int XPR_UPS_SetFloatVK(float value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	if(!key)
		return -1;
	if(strlen(key)>1024)
		return -1;
	va_start(ap, value);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_SetData(buffer, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}

int XPR_UPS_GetFloat(const char* key, float* value)
{
	CHECK_KV(key,value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_GetFloatVK(float* value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	CHECK_KV(key,value);
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
	
}

int XPR_UPS_SetDouble(const char* key, double value)
{
	if(!key)
		return -1;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}

int XPR_UPS_SetDoubleVK(double value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	if(!key)
		return -1;
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_REAL, &value, 0);
}


int XPR_UPS_GetDouble(const char* key, double* value)
{
	if(!key)
		return -1;
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_GetDoubleVK(double* value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	CHECK_KV(key,value);
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_REAL, value, 0);
}

int XPR_UPS_SetBoolean(const char* key, int value)
{
	if(!key)
		return -1;
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, &value, 0);
}

int XPR_UPS_SetBooleanVK(int value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	if(!key)
		return -1;
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
	return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, &value, 0);
}

int XPR_UPS_GetBoolean(const char* key, int* value)
{
	CHECK_KV(key,value);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, value, 0);
}

int XPR_UPS_GetBooleanVK(int* value, const char* key, ...)
{
	va_list ap;
	char buffer[1024];

	CHECK_KV(key,value);
	if(strlen(key)>1024)
		return -1;
	va_start(ap, key);
    vsnprintf(buffer, sizeof(buffer), key, ap);
    va_end(ap);
    return XPR_UPS_GetData(buffer, XPR_UPS_ENTRY_TYPE_BOOLEAN, value, 0);
}

int XPR_UPS_Delete(const char* key)
{
    return 0;
}

int XPR_UPS_Exists(const char* key)
{
    return 0;
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
    return 0;
}

int XPR_UPS_Import(const char* url)
{
    return 0;
}

int XPR_UPS_Pack(void)
{
    return 0;
}

int XPR_UPS_Sync(void)
{
    return 0;
}

