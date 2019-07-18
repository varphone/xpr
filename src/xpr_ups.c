#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_json.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_sync.h>
#include <xpr/xpr_timer.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>

static const XPR_UPS_Blob kDummyBlob = {0,0};
static XPR_Atomic sHasInited = 0;
static XPR_Atomic sHaveChanges = 0;
static XPR_RecursiveMutex sLock;
static XPR_UPS_Entry sRoot =
    XPR_UPS_ENTRY_DIR2("/", "The root of the XPR-UPS system");
static char* sStorage = NULL;
static XPR_JSON* sStorageJson = NULL;
static XPR_Timer* sStorageSyncTimer = NULL;
static __thread XPR_UPS_Entry* sGroupEntry = NULL;

#define XPR_UPS_LOCK() XPR_RecursiveMutexLock(&sLock)
#define XPR_UPS_UNLOCK() XPR_RecursiveMutexUnlock(&sLock)

// Sync settings to storage every 5 seconds
#define XPR_UPS_STORAGE_SYNC_INTERVAL 5000000

// Return `ret` if `exp` == true
#define XPR_RET_IF(exp, retval)                                                \
    if (exp)                                                                   \
        return retval;

// Make a variable key into `keyvar` with `fmt`
#define XPR_UPS_VKEY(keyvar, fmt)                                              \
    va_list ap;                                                                \
    char keyvar[1024];                                                         \
    va_start(ap, fmt);                                                         \
    vsnprintf(keyvar, sizeof(keyvar), fmt, ap);                                \
    va_end(ap);

// Return XPR_TRUE if str ends with '/'
static int slashEnds(const char* str)
{
    if (!str)
        return XPR_FALSE;
    const char* p = str;
    // Move to tailer
    while (*p)
        p++;
    p--; // Rewind to last char
    if (p >= str && *p == '/')
        return XPR_TRUE;
    return XPR_FALSE;
}

// Move to after all '/' leading chars
static const char* skipSlash(const char* key)
{
    if (key) {
        while (*key && *key == '/')
            key++;
    }
    return key;
}

// Release all resources attched to the entry
static void entryCleanup(XPR_UPS_Entry* entry)
{
    XPR_UPS_EntryType type = XPR_UPS_TO_TYPE(entry->type);
    switch (type) {
    case XPR_UPS_ENTRY_TYPE_BLOB:
        // FIXME:
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        if (entry->curVal.str)
            XPR_Freep((void**)(&(entry->curVal.str)));
        if (entry->shaVal.str)
            XPR_Freep((void**)(&(entry->curVal.str)));
        break;
    default:
        break;
    }
}

// Calculate the depth of the entry
static int entryDepth(XPR_UPS_Entry* entry)
{
    int depth = 0;
    while (entry->node.parent) {
        depth++;
        entry = XPR_UPS_TO_ENTRY(entry->node.parent);
    }
    return depth;
}

// Return full name of the entry by travels the tree
static char* entryFullName(XPR_UPS_Entry* entry, char* buf, size_t bufSize,
                           char sep)
{
    int depth = 0;
    int offset = 0;
    XPR_UPS_Entry* temp = entry;
    XPR_UPS_Entry* stack[32];
    while (temp) {
        stack[depth++] = temp;
        temp = XPR_UPS_TO_ENTRY(temp->node.parent);
    }
    depth -= 2;
    if (depth < 0) {
        offset += snprintf(buf, bufSize, "%s", entry->name);
    }
    else {
        for (int n = depth; n >= 0; n--) {
            // No leading seperator for 'a.b.c' style
            if (!(sep == '.' && n == depth))
                offset += snprintf(buf + offset, bufSize - offset, "%c", sep);
            offset +=
                snprintf(buf + offset, bufSize - offset, "%s", stack[n]->name);
        }
    }
    buf[offset] = 0;
    return buf;
}

// Return the name of the type
static const char* entryTypeName(XPR_UPS_EntryType type)
{
    switch (type) {
    case XPR_UPS_ENTRY_TYPE_INIT:
        return "init()";
    case XPR_UPS_ENTRY_TYPE_DIR:
        return "dir";
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        return "bool";
    case XPR_UPS_ENTRY_TYPE_BLOB:
        return "blob";
    case XPR_UPS_ENTRY_TYPE_I32:
        return "i32";
    case XPR_UPS_ENTRY_TYPE_I64:
        return "i64";
    case XPR_UPS_ENTRY_TYPE_F32:
        return "f64";
    case XPR_UPS_ENTRY_TYPE_F64:
        return "f64";
    case XPR_UPS_ENTRY_TYPE_STRING:
        return "str";
    default:
        return "unknown";
    }
}

// Return full name of the unlinked entry by travels the parent
//   or concat the root
static char* entryUnlinkedFullName(XPR_UPS_Entry* entry, XPR_UPS_Entry* parent,
                                   char* buf, int size)
{
    char pfn[256] = {0};
    if (!entry->root && parent) {
        entryFullName(parent, pfn, sizeof(pfn), '.');
        strcat(pfn, ".");
    }
    else if (entry->root) {
        snprintf(pfn, sizeof(pfn), "%s%c", skipSlash(entry->root),
                 slashEnds(entry->root) ? 0 : '/');
        // Convert to a.b.c style
        xpr_replace_char(pfn, '/', '.');
    }
    snprintf(buf, size, "%s%s", pfn, entry->name);
    return buf;
}

// Unlink entry from tree and call finalizer
static void entryUnregister(XPR_UPS_Entry* entry)
{
    XPR_UPS_Entry* prev = NULL;
    XPR_UPS_Entry* next = NULL;
    while (entry) {
        // Unregister childs first if exists
        if (entry->node.childs)
            entryUnregister(XPR_UPS_TO_ENTRY(entry->node.childs));
        // Save the next entry
        prev = XPR_UPS_TO_ENTRY(entry->node.prev);
        next = XPR_UPS_TO_ENTRY(entry->node.next);
        // Call finalizer if exists
        if (entry->fini)
            entry->fini(entry);
        // Break all links
        if (prev)
            prev->node.next = NULL;
        if (next)
            next->node.prev = NULL;
        entry->node.parent = NULL;
        entry->node.prev = NULL;
        entry->node.next = NULL;
        entry->node.childs = NULL;
        // Release all resources attched to the entry
        entryCleanup(entry);
        // Move to next entry
        entry = next;
    }
}

// Copy entry's default value to current
static void entryUseDefaultValue(XPR_UPS_Entry* entry)
{
    switch (XPR_UPS_TO_TYPE(entry->type)) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        entry->curVal.bl = entry->defVal.bl;
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        // FIXME:
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        entry->curVal.i32 = entry->defVal.i32;
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        entry->curVal.i64 = entry->defVal.i64;
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        entry->curVal.f32 = entry->defVal.f32;
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        entry->curVal.f64 = entry->defVal.f64;
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        if (entry->curVal.str)
            XPR_Free(entry->curVal.str);
        entry->curVal.str = XPR_StrDup(entry->defVal.str);
        break;
    default:
        break;
    }
}

// Find sibling entry matched to the name from head
static XPR_UPS_Entry* findSibling(const char* name, XPR_UPS_Entry* head)
{
    if (!name || !head)
        return NULL;
    while (head) {
        if (strcmp(name, head->name) == 0)
            return head;
        head = XPR_UPS_TO_ENTRY(head->node.next);
    }
    return NULL;
}

// Find entry matched to the key on the parent
static XPR_UPS_Entry* findEntry(const char* key, XPR_UPS_Entry* parent)
{
    if (!key && !parent)
        return &sRoot;
    parent = parent ? parent : &sRoot;
    // Fast return root
    if (key[0] == 0 || (key[0] == '/' && key[1] == 0))
        return parent;
    key = skipSlash(key);
    char name[256];
    size_t ns = strlen(key);
    size_t nl = 0;
    size_t nr = 0;
    XPR_UPS_Entry* entry = parent;
    while (entry && nr <= ns) {
        // Split at '/' or '\0'
        if (key[nr] == '/' || key[nr] == '.' || key[nr] == 0) {
            memcpy(name, key + nl, nr - nl);
            name[nr - nl] = 0;
            // If not starts with '/', that is a child
            if (*name != '/' && *name != '.') {
                // Search in childs
                entry = findSibling(name, XPR_UPS_TO_ENTRY(entry->node.childs));
            }
            else {
                // If equals '/', end searching
                if (nr - nl == 1) {
                    // Return NULL if the entry is not a dir
                    if (!XPR_UPS_ENTRY_IS_DIR(entry))
                        entry = NULL;
                    break;
                }
                // Search in childs
                entry = findSibling(name+1, XPR_UPS_TO_ENTRY(entry->node.childs));
            }
            nl = nr;
        }
        nr++;
    }
    return entry;
}

// Find json object matched to the entry
static XPR_JSON* findJsonForEntry(XPR_UPS_Entry* entry)
{
    if (!entry)
        return NULL;
    // Fast return root
    if (entry->name[0] == 0 || (entry->name[0] == '/' && entry->name[1] == 0))
        return sStorageJson;
    int depth = 0;
    XPR_UPS_Entry* temp = entry;
    XPR_UPS_Entry* stack[32];
    while (temp) {
        stack[depth++] = temp;
        temp = XPR_UPS_TO_ENTRY(temp->node.parent);
    }
    depth -= 2; // Rewind to last entry
    XPR_JSON* jx = sStorageJson;
    for (int n = depth; n >= 0; n--) {
        jx = XPR_JSON_ObjectGet(jx, stack[n]->name);
        if (jx == NULL)
            break;
    }
    return jx;
}

// Load json document from storage
static int loadJson(const char* storage)
{
    XPR_JSON* jx = XPR_JSON_LoadFileName(storage);
    if (jx == NULL) {
        DBG(DBG_L2, "XPR_UPS: Mount storage \"%s\" failed!", storage);
        return XPR_ERR_SYS(EIO);
    }
    sStorageJson = jx;
    if (sStorage)
        XPR_Free(sStorage);
    sStorage = XPR_StrDup(storage);
    DBG(DBG_L5, "XPR_UPS: Mounted storage \"%s\" @ %p!", storage, jx);
    return XPR_ERR_OK;
}

// Callback for sync settings to storage
static XPR_TimerReturn storageSync(void* opaque)
{
    // Call sync with locking
    XPR_UPS_Sync();
    // Timer keep on next
    return XPR_TIMER_CONTINUE;
}

// Start the sotrage sync timer
static void startSyncTimer(void)
{
    if (sStorageSyncTimer == NULL) {
        XPR_TimerId timerId = XPR_TimerIdNew();
        sStorageSyncTimer = XPR_TimerCreate(
            timerId, XPR_UPS_STORAGE_SYNC_INTERVAL, storageSync, NULL);
        if (sStorageSyncTimer) {
            XPR_TimerEnable(sStorageSyncTimer);
            int err =
                XPR_TimerQueueAdd(XPR_TimerQueueDefault(), sStorageSyncTimer);
            if (err != XPR_ERR_OK) {
                DBG(DBG_L2,
                    "XPR_UPS: Storage sync timer %p start failed, errno: "
                    "0x%08X",
                    sStorageSyncTimer, err);
                XPR_TimerDestroy(sStorageSyncTimer);
                sStorageSyncTimer = NULL;
            }
            else {
                DBG(DBG_L4, "XPR_UPS: Storage sync timer %p started!",
                    sStorageSyncTimer);
            }
        }
    }
}

// Stop the sotrage sync timer
static void stopSyncTimer(void)
{
    if (sStorageSyncTimer) {
        int err =
            XPR_TimerQueueRemove(XPR_TimerQueueDefault(), sStorageSyncTimer);
        if (err != XPR_ERR_OK) {
            DBG(DBG_L2,
                "XPR_UPS: Storage sync timer %p stop failed, errno: 0x%08X",
                sStorageSyncTimer, err);
        }
        else {
            DBG(DBG_L4, "XPR_UPS: Storage sync timer %p stopped!",
                sStorageSyncTimer);
            sStorageSyncTimer = NULL;
        }
    }
}

// Mount storage (supports multiple format)
static int mountStorage(const char* storage)
{
    int err = XPR_ERR_UPS_NOT_SUPPORT;
    if (xpr_ends_with(storage, ".json"))
        err = loadJson(storage);
    if (err == XPR_ERR_OK)
        startSyncTimer();
    return err;
}

// Release mounted storage resource
static void unmountStorage(void)
{
    stopSyncTimer();
    if (sStorageJson) {
        XPR_JSON_DecRef(sStorageJson);
        sStorageJson = NULL;
    }
}

// Sync settings to storage
static int syncStorage(void)
{
    char f1[512];
    char f2[512];
    // Backup xxx.2 ==> xxx.3
    snprintf(f1, sizeof(f1), "%s.2", sStorage);
    snprintf(f2, sizeof(f2), "%s.3", sStorage);
    XPR_FileCopy(f1, f2);
    DBG(DBG_L5, "XPR_UPS: Backup: '%s' ==> '%s'", f1, f2);
    // Backup xxx.1 ==> xxx.2
    snprintf(f1, sizeof(f1), "%s.1", sStorage);
    snprintf(f2, sizeof(f2), "%s.2", sStorage);
    XPR_FileCopy(f1, f2);
    DBG(DBG_L5, "XPR_UPS: Backup: '%s' ==> '%s'", f1, f2);
    // Backup xxx ==> xxx.1
    snprintf(f2, sizeof(f2), "%s.1", sStorage);
    XPR_FileCopy(sStorage, f2);
    DBG(DBG_L5, "XPR_UPS: Backup: '%s' ==> '%s'", sStorage, f2);
    // Save current
    int err = XPR_ERR_OK;
    if (sStorageJson) {
        err = XPR_JSON_DumpFileName(sStorageJson, sStorage);
        if (err == XPR_ERR_OK)
            DBG(DBG_L5, "XPR_UPS: Sync storage '%s' okay!", sStorage);
        else
            DBG(DBG_L2, "XPR_UPS: Sync storage '%s' failed, error: 0x%08X",
                sStorage, err);
    }
    return err;
}

// Fetch the value from storage for entry
static int storageFetchValue(XPR_UPS_Entry* entry)
{
    XPR_JSON* jx = findJsonForEntry(entry);
    if (jx == NULL) {
        DBG(DBG_L5, "XPR_UPS: Json for entry \"%s\" @ %p not exists",
            entry->name, entry);
        return XPR_ERR_SYS(ENOENT);
    }
    int err = XPR_ERR_OK;
    switch (XPR_UPS_TO_TYPE(entry->type)) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        entry->curVal.bl = XPR_JSON_IntegerValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        entry->curVal.i32 = XPR_JSON_IntegerValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        entry->curVal.i64 = XPR_JSON_Integer64Value(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        entry->curVal.f32 = XPR_JSON_RealValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        entry->curVal.f64 = XPR_JSON_RealValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        if (entry->curVal.str)
            XPR_Free(entry->curVal.str);
        entry->curVal.str = XPR_StrDup(XPR_JSON_StringValue(jx));
        break;
    default:
        DBG(DBG_L2, "XPR_UPS: Fetch type(%s) from storage does not supported!",
            entryTypeName(XPR_UPS_TO_TYPE(entry->type)));
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    }
    return err;
}

// Mark storage has changed
static void storageHaveChanges(void)
{
    XPR_AtomicInc(&sHaveChanges);
}

// Fill the data buffer with regular-zero value
static void clearDataBuffer(XPR_UPS_EntryType type, void* buffer, int* size)
{
    switch (type) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        *(int*)(buffer) = 0;
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        // FIXME:
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        *(int*)(buffer) = 0;
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        *(int64_t*)(buffer) = 0;
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        *(float*)(buffer) = 0.0;
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        *(double*)(buffer) = 0.0;
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        *(char*)(buffer) = 0;
        break;
    default:
        break;
    }
}

// Get data for key by type without locking
static int XPR_UPS_GetDataNl(const char* key, XPR_UPS_EntryType type,
                             void* buffer, int* size)
{
    if (!key || !buffer)
        return XPR_ERR_NULL_PTR;
    if (type == XPR_UPS_ENTRY_TYPE_STRING && !size)
        return XPR_ERR_NULL_PTR;
    XPR_UPS_Entry* entry = findEntry(key, sGroupEntry);
    if (!entry) {
        DBG(DBG_L2, "XPR_UPS: GetData(\"%s\", %s, %p, %p), entry not exists!",
            key, entryTypeName(type), buffer, size);
        clearDataBuffer(type, buffer, size);
        return XPR_ERR_UPS_UNEXIST;
    }
    if (XPR_UPS_ENTRY_IS_INIT(entry))
        return XPR_ERR_UPS_NOT_SUPPORT;
    int err = XPR_ERR_OK;
    if (entry->get)
        err = entry->get(entry, entry->name, buffer, size);
    if (entry->node.parent) {
        XPR_UPS_Entry* parent = XPR_UPS_TO_ENTRY(entry->node.parent);
        if (parent->get)
            err = parent->get(entry, entry->name, buffer, size);
    }
    if (err != XPR_ERR_OK)
        err = XPR_UPS_ReadValue(entry, buffer, size);
    if (err != XPR_ERR_OK)
        err = XPR_UPS_ReadData(entry, buffer, size);
    if (err != XPR_ERR_OK)
        clearDataBuffer(type, buffer, size);
    return err;
}

// Get data for key by type with locking
static int XPR_UPS_GetData(const char* key, XPR_UPS_EntryType type,
                           void* buffer, int* size)
{
    int err = XPR_ERR_OK;
    XPR_UPS_LOCK();
    err = XPR_UPS_GetDataNl(key, type, buffer, size);
    XPR_UPS_UNLOCK();
    return err;
}

// Set data for key by type without locking
static int XPR_UPS_SetDataNl(const char* key, XPR_UPS_EntryType type,
                             const void* data, int size)
{
    if (!key || !data)
        return XPR_ERR_NULL_PTR;
    XPR_UPS_Entry* entry = findEntry(key, sGroupEntry);
    if (!entry) {
        DBG(DBG_L2, "XPR_UPS: SetData(\"%s\", %s, %p, %d), entry not exists!",
            key, entryTypeName(type), data, size);
        return XPR_ERR_UPS_UNEXIST;
    }
    if (XPR_UPS_ENTRY_IS_INIT(entry))
        return XPR_ERR_UPS_NOT_SUPPORT;
    int err = XPR_ERR_OK;
    if (entry->set)
        err = entry->set(entry, key, data, size);
    else if (entry->node.parent) {
        XPR_UPS_Entry* parent = XPR_UPS_TO_ENTRY(entry->node.parent);
        if (parent->set)
            err = parent->set(entry, key, data, size);
    }
    if (err == XPR_ERR_OK)
        err = XPR_UPS_WriteValue(entry, data, size);
    if (err == XPR_ERR_OK && !(entry->type & XPR_UPS_ENTRY_FLAG_NOSTOR))
        err = XPR_UPS_WriteData(entry, data, size);
    return err;
}

// Set data for key by type with locking
static int XPR_UPS_SetData(const char* key, XPR_UPS_EntryType type,
                           const void* data, int size)
{
    int err = XPR_ERR_OK;
    XPR_UPS_LOCK();
    err = XPR_UPS_SetDataNl(key, type, data, size);
    XPR_UPS_UNLOCK();
    return err;
}

static void XPR_UPS_RegisterAll(void)
{
#if defined(HAVE_XPR_UPS_DRIVER_FAKEROOT)
    extern XPR_UPS_Entry __xpr_ups_driver_fakeroot[];
    extern const int __xpr_ups_driver_fakeroot_count;
    XPR_UPS_Register(__xpr_ups_driver_fakeroot,
                     __xpr_ups_driver_fakeroot_count);
#endif
#if defined(HAVE_XPR_UPS_DRIVER_SYS_INFO)
    extern XPR_UPS_Entry __xpr_ups_driver_sys_info[];
    extern const int __xpr_ups_driver_sys_info_count;
    XPR_UPS_Register(__xpr_ups_driver_sys_info,
                     __xpr_ups_driver_sys_info_count);
#endif
#if defined(HAVE_XPR_UPS_DRIVER_SYS_NET)
    extern XPR_UPS_Entry __xpr_ups_driver_sys_net[];
    extern const int __xpr_ups_driver_sys_net_count;
    XPR_UPS_Register(__xpr_ups_driver_sys_net, __xpr_ups_driver_sys_net_count);
#endif
#if defined(HAVE_XPR_UPS_DRIVER_CAM_IMG)
    extern XPR_UPS_Entry __xpr_ups_driver_cam_img[];
    extern const int __xpr_ups_driver_cam_img_count;
    XPR_UPS_Register(__xpr_ups_driver_cam_img, __xpr_ups_driver_cam_img_count);
#endif
    // Register other ...
}

XPR_API int XPR_UPS_Init(const char* storage)
{
    // Guard for threads safe
    if (XPR_AtomicInc(&sHasInited) == 1) {
        XPR_RecursiveMutexInit(&sLock);
        XPR_UPS_LOCK();
        mountStorage(storage);
        XPR_UPS_RegisterAll();
        XPR_UPS_UNLOCK();
        return XPR_ERR_SUCCESS;
    }
    DBG(DBG_L2, "XPR_UPS: Init/Fini counter: %d", XPR_AtomicRead(&sHasInited));
    return XPR_ERR_GEN_BUSY;
}

XPR_API int XPR_UPS_Fini(void)
{
    // Guard for threads safe
    if (XPR_AtomicDec(&sHasInited) == 0) {
        XPR_UPS_LOCK();
        XPR_UPS_UnRegisterAll();
        unmountStorage();
        XPR_UPS_UNLOCK();
        XPR_RecursiveMutexFini(&sLock);
        return XPR_ERR_SUCCESS;
    }
    DBG(DBG_L2, "XPR_UPS: Init/Fini counter: %d", XPR_AtomicRead(&sHasInited));
    return XPR_ERR_GEN_BUSY;
}

XPR_API XPR_UPS_Entry* XPR_UPS_FindEntry(const char* key, XPR_UPS_Entry* parent)
{
    XPR_UPS_Entry* entry = NULL;
    XPR_UPS_LOCK();
    entry = findEntry(key, parent);
    XPR_UPS_UNLOCK();
    return entry;
}

XPR_API int XPR_UPS_Register(XPR_UPS_Entry ents[], int count)
{
    int err = XPR_ERR_OK;
    int fails = 0;
    int ignores = 0;
    int success = 0;
    char fullName[256];
    XPR_UPS_Entry* prev = NULL;
    XPR_UPS_Entry* curr = NULL;
    for (int i = 0; i < count; i++) {
        curr = &ents[i];
        if (!curr->name) {
            ignores++;
            break;
        }
        if (curr->name[0] == '$')
            curr->type |= XPR_UPS_ENTRY_FLAG_NOSTOR;
        if (XPR_UPS_ENTRY_IS_INIT(curr))
            curr->type |= XPR_UPS_ENTRY_FLAG_NOSTOR;
        err = XPR_UPS_RegisterSingle(curr, curr->root ? NULL : prev);
        if (err == XPR_ERR_UPS_EXIST) {
            entryUnlinkedFullName(curr, prev, fullName, sizeof(fullName));
            DBG(DBG_L2, "XPR_UPS: Register \"%s\" ignored, alreay exists!",
                fullName);
            // Save registered entry as current
            if (XPR_UPS_ENTRY_IS_DIR(curr)) {
                curr = XPR_UPS_FindEntry(
                    curr->name,
                    curr->root ? XPR_UPS_FindEntry(curr->root, NULL) : prev);
            }
            ignores++;
        }
        else if (XPR_IS_ERROR(err)) {
            entryUnlinkedFullName(curr, prev, fullName, sizeof(fullName));
            DBG(DBG_L2, "XPR_UPS: Register \"%s\" failed, errno: 0x%08X",
                fullName, err);
            fails++;
        }
        else {
            DBG(DBG_L5, "XPR_UPS: Registered \"%s\" @ %p",
                entryFullName(curr, fullName, sizeof(fullName), '.'), curr);
            success++;
        }
        if (XPR_UPS_ENTRY_IS_DIR(curr))
            prev = curr;
    }
    DBG(DBG_L5,
        "XPR_UPS: Registered entries: fails=%d, ignores=%d, success=%d, "
        "total=%d",
        fails, ignores, success, count);
    return err;
}

XPR_API int XPR_UPS_RegisterAt(XPR_UPS_Entry ents[], int count,
                               const char* root)
{
    if (!root)
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    XPR_UPS_Entry* rootEntry = findEntry(root, NULL);
    if (!rootEntry)
        return XPR_ERR_UPS_UNEXIST;
    for (int i = 0; i < count; i++) {
        int result = XPR_UPS_RegisterSingle(&ents[i], rootEntry);
        DBG(DBG_L5, "XPR_UPS: RegisterAt %s @ %s result: 0x%08X", ents[i].name,
            ents[i].root, result);
    }
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_RegisterSingle(XPR_UPS_Entry* entry, XPR_UPS_Entry* parent)
{
    if (!entry)
        return XPR_ERR_UPS_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_UPS_LOCK();
    if (!parent)
        parent = findEntry(entry->root, NULL);
    // Skip or override if the entry exists
    if (parent) {
        XPR_UPS_Entry* temp = findEntry(entry->name, parent);
        if (temp) {
            if (entry->type & XPR_UPS_ENTRY_FLAG_OVRIDE) {
                DBG(DBG_L2, "XPR_UPS: Override ['%s'@%p] ==> ['%s'@%p]!",
                    temp->name, temp, entry->name, entry);
                entryUnregister(temp);
            }
            else {
                err = XPR_ERR_UPS_EXIST;
                goto done;
            }
        }
    }
    if (!parent) {
        err = XPR_ERR_UPS_SYS_NOTREADY;
        goto done;
    }
    // Link to tree
    XPR_UPS_Node* child = parent->node.childs;
    if (child == NULL) {
        parent->node.childs = XPR_UPS_TO_NODE(entry);
    }
    else {
        while (child->next) {
            child = child->next;
        }
        child->next = XPR_UPS_TO_NODE(entry);
    }
    // Update link nodes
    entry->node.parent = XPR_UPS_TO_NODE(parent);
    entry->node.prev = child;
    entry->node.next = NULL;
    entry->node.childs = NULL;
    // Fetch current value from storage
    if (!(entry->type & XPR_UPS_ENTRY_FLAG_NOSTOR) &&
        !(XPR_UPS_ENTRY_IS_DIR(entry) || XPR_UPS_ENTRY_IS_INIT(entry))) {
        if (storageFetchValue(entry) != XPR_ERR_OK)
            entryUseDefaultValue(entry);
    }
    // Initialize the entry first
    if (entry->init) {
        err = entry->init(entry);
        if (err != XPR_ERR_OK) {
            DBG(DBG_L2,
                "XPR_UPS: Entry %s @ %p initialize error: 0x%08X",
                entry->name, entry, err);
            goto done;
        }
    }
    // Okay
    err = XPR_ERR_OK;
done:
    XPR_UPS_UNLOCK();
    return err;
}

XPR_API int XPR_UPS_UnRegister(XPR_UPS_Entry ents[], int count)
{
    XPR_UPS_LOCK();
    for (int i = 0; i < count; i++) {
        if (ents[i].name == NULL)
            break;
        entryUnregister(&ents[i]);
    }
    XPR_UPS_UNLOCK();
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_UPS_UnRegisterAll(void)
{
    XPR_UPS_LOCK();
    entryUnregister(&sRoot);
    XPR_UPS_UNLOCK();
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_UPS_SetString(const char* key, const char* value, int size)
{
    XPR_RET_IF(!key || !value, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

XPR_API int XPR_UPS_SetStringVK(const char* value, int size, const char* vkey,
                                ...)
{
    XPR_RET_IF(!value || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_SetData(newKey, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

XPR_API int XPR_UPS_GetString(const char* key, char* value, int* size)
{
    XPR_RET_IF(!key || !value || !size, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

XPR_API int XPR_UPS_GetStringVK(char* value, int* size, const char* vkey, ...)
{
    XPR_RET_IF(!value || !size || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_GetData(newKey, XPR_UPS_ENTRY_TYPE_STRING, value, size);
}

XPR_API const char* XPR_UPS_PeekString(const char* key)
{
    XPR_RET_IF(!key, NULL);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, NULL);
    return entry ? entry->curVal.str : NULL;
}

XPR_API const char* XPR_UPS_PeekStringVK(const char* vkey, ...)
{
    XPR_RET_IF(!vkey, NULL);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_PeekString(newKey);
}

XPR_API int XPR_UPS_SetInteger(const char* key, int value)
{
    XPR_RET_IF(!key, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_I32, &value, 0);
}

XPR_API int XPR_UPS_SetIntegerVK(int value, const char* vkey, ...)
{
    XPR_RET_IF(!vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_SetData(newKey, XPR_UPS_ENTRY_TYPE_I32, &value, 0);
}

XPR_API int XPR_UPS_GetInteger(const char* key, int* value)
{
    XPR_RET_IF(!key || !value, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_I32, value, 0);
}

XPR_API int XPR_UPS_GetIntegerVK(int* value, const char* vkey, ...)
{
    XPR_RET_IF(!value || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_GetData(newKey, XPR_UPS_ENTRY_TYPE_I32, value, 0);
}

XPR_API int XPR_UPS_PeekInteger(const char* key)
{
    XPR_RET_IF(!key, 0);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, NULL);
    return entry ? entry->curVal.i32 : 0;
}

XPR_API int XPR_UPS_PeekIntegerVK(const char* vkey, ...)
{
    XPR_RET_IF(!vkey, 0);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_PeekInteger(newKey);
}

XPR_API int XPR_UPS_SetInt64(const char* key, int64_t value)
{
    XPR_RET_IF(!key, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_I64, &value, 0);
}

XPR_API int XPR_UPS_SetInt64VK(int64_t value, const char* vkey, ...)
{
    XPR_RET_IF(!vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_SetData(newKey, XPR_UPS_ENTRY_TYPE_I64, &value, 0);
}

XPR_API int XPR_UPS_GetInt64(const char* key, int64_t* value)
{
    XPR_RET_IF(!key || !value, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_I64, value, 0);
}

XPR_API int XPR_UPS_GetInt64VK(int64_t* value, const char* vkey, ...)
{
    XPR_RET_IF(!value || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_GetData(newKey, XPR_UPS_ENTRY_TYPE_I64, value, 0);
}

XPR_API int64_t XPR_UPS_PeekInt64(const char* key)
{
    XPR_RET_IF(!key, 0);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, NULL);
    return entry ? entry->curVal.i64 : 0;
}

XPR_API int64_t XPR_UPS_PeekInt64VK(const char* vkey, ...)
{
    XPR_RET_IF(!vkey, 0);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_PeekInt64(newKey);
}

XPR_API int XPR_UPS_SetFloat(const char* key, float value)
{
    XPR_RET_IF(!key, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_F64, &value, 0);
}

XPR_API int XPR_UPS_SetFloatVK(float value, const char* vkey, ...)
{
    XPR_RET_IF(!vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_SetData(newKey, XPR_UPS_ENTRY_TYPE_F64, &value, 0);
}

XPR_API int XPR_UPS_GetFloat(const char* key, float* value)
{
    XPR_RET_IF(!key || !value, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_F64, value, 0);
}

XPR_API int XPR_UPS_GetFloatVK(float* value, const char* vkey, ...)
{
    XPR_RET_IF(!value || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_GetData(newKey, XPR_UPS_ENTRY_TYPE_F64, value, 0);
}

XPR_API float XPR_UPS_PeekFloat(const char* key)
{
    XPR_RET_IF(!key, 0.0);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, NULL);
    return entry ? entry->curVal.f32 : 0.0f;
}

XPR_API float XPR_UPS_PeekFloatVK(const char* vkey, ...)
{
    XPR_RET_IF(!vkey, 0.0f);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_PeekFloat(newKey);
}

XPR_API int XPR_UPS_SetDouble(const char* key, double value)
{
    XPR_RET_IF(!key, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_F64, &value, 0);
}

XPR_API int XPR_UPS_SetDoubleVK(double value, const char* vkey, ...)
{
    XPR_RET_IF(!vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_SetData(newKey, XPR_UPS_ENTRY_TYPE_F64, &value, 0);
}

XPR_API int XPR_UPS_GetDouble(const char* key, double* value)
{
    XPR_RET_IF(!key || !value, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_F64, value, 0);
}

XPR_API int XPR_UPS_GetDoubleVK(double* value, const char* vkey, ...)
{
    XPR_RET_IF(!value || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_GetData(newKey, XPR_UPS_ENTRY_TYPE_F64, value, 0);
}

XPR_API double XPR_UPS_PeekDouble(const char* key)
{
    XPR_RET_IF(!key, 0.0);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, NULL);
    return entry ? entry->curVal.f64 : 0;
}

XPR_API double XPR_UPS_PeekDoubleVK(const char* vkey, ...)
{
    XPR_RET_IF(!vkey, 0.0);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_PeekDouble(newKey);
}

XPR_API int XPR_UPS_SetBoolean(const char* key, int value)
{
    XPR_RET_IF(!key, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_SetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, &value, 0);
}

XPR_API int XPR_UPS_SetBooleanVK(int value, const char* vkey, ...)
{
    XPR_RET_IF(!vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_SetData(newKey, XPR_UPS_ENTRY_TYPE_BOOLEAN, &value, 0);
}

XPR_API int XPR_UPS_GetBoolean(const char* key, int* value)
{
    XPR_RET_IF(!key || !value, XPR_ERR_UPS_NULL_PTR);
    return XPR_UPS_GetData(key, XPR_UPS_ENTRY_TYPE_BOOLEAN, value, 0);
}

XPR_API int XPR_UPS_GetBooleanVK(int* value, const char* vkey, ...)
{
    XPR_RET_IF(!value || !vkey, XPR_ERR_UPS_NULL_PTR);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_GetData(newKey, XPR_UPS_ENTRY_TYPE_BOOLEAN, value, 0);
}

XPR_API int XPR_UPS_PeekBoolean(const char* key)
{
    XPR_RET_IF(!key, 0);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, NULL);
    return entry ? entry->curVal.bl : 0;
}

XPR_API int XPR_UPS_PeekBooleanVK(const char* vkey, ...)
{
    XPR_RET_IF(!vkey, 0);
    XPR_UPS_VKEY(newKey, vkey);
    return XPR_UPS_PeekBoolean(newKey);
}

static int readDataFromJson(XPR_UPS_Entry* entry, void* buffer, int* size)
{
    XPR_JSON* jx = findJsonForEntry(entry);
    if (jx == NULL) {
        DBG(DBG_L2, "XPR_UPS: Json for entry \"%s\" @ %p not exists",
            entry->name, entry);
        return XPR_ERR_SYS(ENOENT);
    }
    int err = XPR_ERR_OK;
    switch (XPR_UPS_TO_TYPE(entry->type)) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        *(int*)buffer = XPR_JSON_IntegerValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        *(int*)buffer = XPR_JSON_IntegerValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        *(int64_t*)buffer = XPR_JSON_Integer64Value(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        *(float*)buffer = XPR_JSON_RealValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        *(double*)buffer = XPR_JSON_RealValue(jx);
        break;
    case XPR_UPS_ENTRY_TYPE_STRING: {
        const char* s = XPR_JSON_StringValue(jx);
        int len = strlen(s);
        if (len >= *size) {
            err = XPR_ERR_BUF_FULL;
            break;
        }
        strcpy_s(buffer, *size, s);
        *size = len;
        break;
    }
    default:
        DBG(DBG_L2, "XPR_UPS: Read type(%s) from storage does not supported!",
            entryTypeName(XPR_UPS_TO_TYPE(entry->type)));
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    }
    return err;
}

static int writeDataToJson(XPR_UPS_Entry* entry, const void* data, int size)
{
    XPR_JSON* jx = findJsonForEntry(entry);
    if (jx == NULL) {
        DBG(DBG_L2, "XPR_UPS: Json for entry \"%s\" @ %p not exists",
            entry->name, entry);
        return XPR_ERR_SYS(ENOENT);
    }
    int err = XPR_ERR_OK;
    switch (XPR_UPS_TO_TYPE(entry->type)) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        err = XPR_JSON_IntegerSet(jx, *(int*)data);
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        err = XPR_JSON_IntegerSet(jx, *(int*)data);
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        err = XPR_JSON_Integer64Set(jx, *(int64_t*)data);
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        err = XPR_JSON_RealSet(jx, *(double*)data);
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        err = XPR_JSON_StringSet(jx, (char*)data);
        break;
    default:
        DBG(DBG_L2, "XPR_UPS: Write type(%s) to storage does not supported!",
            entryTypeName(XPR_UPS_TO_TYPE(entry->type)));
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    }
    if (XPR_ERR_OK == err)
        storageHaveChanges();
    return err;
}

XPR_API int XPR_UPS_ReadData(XPR_UPS_Entry* entry, void* buffer, int* size)
{
    int err = XPR_ERR_UPS_SYS_NOTREADY;
    XPR_UPS_LOCK();
    if (sStorageJson)
        err = readDataFromJson(entry, buffer, size);
    XPR_UPS_LOCK();
    return err;
}

XPR_API int XPR_UPS_ReadValue(XPR_UPS_Entry* entry, void* buffer, int* size)
{
    if (!entry || !buffer)
        return XPR_ERR_UPS_NULL_PTR;
    int err = XPR_ERR_OK;
    XPR_UPS_LOCK();
    switch (XPR_UPS_TO_TYPE(entry->type)) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        *(int*)buffer = entry->curVal.bl;
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        *(int*)buffer = entry->curVal.i32;
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        *(int64_t*)buffer = entry->curVal.i64;
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        *(float*)buffer = entry->curVal.f32;
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        *(double*)buffer = entry->curVal.f64;
        break;
    case XPR_UPS_ENTRY_TYPE_STRING: {
        if (size == NULL) {
            err = XPR_ERR_UPS_ILLEGAL_PARAM;
            break;
        }
        const char* s = entry->curVal.str;
        int len = strlen(s);
        if (len >= *size) {
            err = XPR_ERR_SYS(ENOSPC);
            break;
        }
        strcpy_s(buffer, *size, s);
        *size = len;
        break;
    }
    default:
        DBG(DBG_L2, "XPR_UPS: Read type(%s) from entry does not supported!",
            entryTypeName(XPR_UPS_TO_TYPE(entry->type)));
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    }
    XPR_UPS_UNLOCK();
    return err;
}

XPR_API int XPR_UPS_WriteData(XPR_UPS_Entry* entry, const void* data, int size)
{
    int err = XPR_ERR_UPS_SYS_NOTREADY;
    XPR_UPS_LOCK();
    if (sStorageJson)
        err = writeDataToJson(entry, data, size);
    XPR_UPS_UNLOCK();
    return err;
}

XPR_API int XPR_UPS_WriteValue(XPR_UPS_Entry* entry, const void* data, int size)
{
    int err = XPR_ERR_OK;
    XPR_UPS_LOCK();
    switch (XPR_UPS_TO_TYPE(entry->type)) {
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        entry->curVal.bl = *(int*)(data);
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        entry->curVal.i32 = *(int*)(data);
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        entry->curVal.i64 = *(int64_t*)(data);
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        entry->curVal.f32 = *(float*)(data);
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        entry->curVal.f64 = *(double*)(data);
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        if (entry->curVal.str)
            XPR_Free(entry->curVal.str);
        entry->curVal.str = XPR_StrDup((const char*)data);
        break;
    default:
        DBG(DBG_L2, "XPR_UPS: Write type(%s) to entry does not supported!",
            entryTypeName(XPR_UPS_TO_TYPE(entry->type)));
        err = XPR_ERR_UPS_NOT_SUPPORT;
        break;
    }
    XPR_UPS_UNLOCK();
    return err;
}

XPR_API int XPR_UPS_PibString(void* dst, int* dstSize, const char* val,
                              int valSize)
{
    if (!dst || !dstSize)
        return XPR_ERR_UPS_NULL_PTR;
    int err = XPR_ERR_OK;
    if (!val) {
        err = XPR_ERR_SYS(ENODATA);
    }
    else {
        if (valSize < 0)
            valSize = strlen(val);
        if (valSize >= *dstSize) {
            err = XPR_ERR_SYS(ENOSPC);
        }
        else {
            strcpy_s(dst, *dstSize, val);
            *dstSize = valSize;
        }
    }
    // Fill null terminator if failed
    if (err != XPR_ERR_OK) {
        *(char*)(dst) = 0;
    }
    return err;
}

XPR_API int XPR_UPS_PibInteger(void* dst, int* dstSize, int val)
{
    if (!dst)
        return XPR_ERR_UPS_NULL_PTR;
    *(int*)(dst) = val;
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_PibInt64(void* dst, int* dstSize, int64_t val)
{
    if (!dst)
        return XPR_ERR_UPS_NULL_PTR;
    *(int64_t*)(dst) = val;
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_PibFloat(void* dst, int* dstSize, float val)
{
    if (!dst)
        return XPR_ERR_UPS_NULL_PTR;
    *(float*)(dst) = val;
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_PibDouble(void* dst, int* dstSize, double val)
{
    if (!dst)
        return XPR_ERR_UPS_NULL_PTR;
    *(double*)(dst) = val;
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_PibBoolean(void* dst, int* dstSize, int val)
{
    if (!dst)
        return XPR_ERR_UPS_NULL_PTR;
    *(int*)(dst) = val;
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_PibBlob(void* dst, int* dstSize, XPR_UPS_Blob val)
{
    // FIXME:
    return XPR_ERR_UPS_NOT_SUPPORT;
}

XPR_API const char* XPR_UPS_PsvString(XPR_UPS_Entry* curr, const char* name)
{
    XPR_RET_IF(!curr || !name, NULL);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.str : NULL;
}

XPR_API const char* XPR_UPS_PsvStringDV(XPR_UPS_Entry* curr, const char* name,
                                        const char* defVal)
{
    XPR_RET_IF(!curr || !name, defVal);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.str : defVal;
}

XPR_API int XPR_UPS_PsvInteger(XPR_UPS_Entry* curr, const char* name)
{
    XPR_RET_IF(!curr || !name, 0);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.i32 : 0;
}

XPR_API int XPR_UPS_PsvIntegerDV(XPR_UPS_Entry* curr, const char* name,
                                 int defVal)
{
    XPR_RET_IF(!curr || !name, defVal);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.i32 : defVal;
}

XPR_API int64_t XPR_UPS_PsvInt64(XPR_UPS_Entry* curr, const char* name)
{
    XPR_RET_IF(!curr || !name, 0);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.i64 : 0;
}

XPR_API int64_t XPR_UPS_PsvInt64DV(XPR_UPS_Entry* curr, const char* name,
                                   int64_t defVal)
{
    XPR_RET_IF(!curr || !name, defVal);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.i64 : defVal;
}

XPR_API double XPR_UPS_PsvDouble(XPR_UPS_Entry* curr, const char* name)
{
    XPR_RET_IF(!curr || !name, 0.0);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.f64 : 0.0;
}

XPR_API double XPR_UPS_PsvDoubleDV(XPR_UPS_Entry* curr, const char* name,
                                   double defVal)
{
    XPR_RET_IF(!curr || !name, defVal);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.f64 : defVal;
}

XPR_API int XPR_UPS_PsvBoolean(XPR_UPS_Entry* curr, const char* name)
{
    XPR_RET_IF(!curr || !name, XPR_FALSE);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.bl : 0;
}

XPR_API int XPR_UPS_PsvBooleanDV(XPR_UPS_Entry* curr, const char* name,
                                 int defVal)
{
    XPR_RET_IF(!curr || !name, defVal);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.bl : defVal;
}

XPR_API XPR_UPS_Blob XPR_UPS_PsvBlob(XPR_UPS_Entry* curr, const char* name)
{
    XPR_RET_IF(!curr || !name, kDummyBlob);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.bb : kDummyBlob;
}

XPR_API XPR_UPS_Blob XPR_UPS_PsvBlobDV(XPR_UPS_Entry* curr, const char* name,
                                       XPR_UPS_Blob defVal)
{
    XPR_RET_IF(!curr || !name, defVal);
    XPR_UPS_Entry* entry =
        XPR_UPS_FindEntry(name, XPR_UPS_TO_ENTRY(curr->node.parent));
    return entry ? entry->curVal.bb : defVal;
}

XPR_API int XPR_UPS_Delete(const char* key)
{
    return XPR_ERR_UPS_NOT_SUPPORT;
}

XPR_API int XPR_UPS_Exists(const char* key)
{
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(key, sGroupEntry);
    return entry ? XPR_TRUE : XPR_FALSE;
}

XPR_API int XPR_UPS_ExistsVK(const char* vkey, ...)
{
    XPR_UPS_VKEY(newKey, vkey);
    XPR_UPS_Entry* entry = XPR_UPS_FindEntry(newKey, sGroupEntry);
    return entry ? XPR_TRUE : XPR_FALSE;
}

XPR_API const char* XPR_UPS_FirstKey(void)
{
    return NULL;
}

XPR_API const char* XPR_UPS_NextKey(const char* key)
{
    return NULL;
}

XPR_API int XPR_UPS_BeginGroup(const char* group)
{
    sGroupEntry = XPR_UPS_FindEntry(group, NULL);
    return sGroupEntry ? XPR_TRUE : XPR_FALSE;
}

XPR_API void XPR_UPS_EndGroup(const char* group)
{
    sGroupEntry = NULL;
}

// Print the key part of the entry to file stream
static void fprintEntryKey(FILE* fp, XPR_UPS_Entry* entry)
{
    char buf[256];
    fprintf(fp, "# %s\n", entry->desc);
    fprintf(fp, "%s = ", entryFullName(entry, buf, sizeof(buf), '.'));
}

// Print the value part of the entry to file stream
static void fprintEntryValue(FILE* fp, XPR_UPS_Value* val,
                             XPR_UPS_EntryType type)
{
    switch (type) {
    case XPR_UPS_ENTRY_TYPE_INIT:
        fprintf(fp, "-");
        break;
    case XPR_UPS_ENTRY_TYPE_DIR:
        fprintf(fp, "-");
        break;
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        fprintf(fp, "%s", val->bl ? "true" : "false");
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        fprintf(fp, "(%p,%d)", val->bb.data, val->bb.size);
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        fprintf(fp, "%d", val->i32);
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        fprintf(fp, "%ld", val->i64);
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        fprintf(fp, "%f", val->f32);
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        fprintf(fp, "%lf", val->f64);
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        if (val->str)
            fprintf(fp, "\"%s\"", val->str);
        else
            fprintf(fp, "(null)");
        break;
    default:
        break;
    }
}

// Print the entry in flat plain format to file stream
static void fprintEntry(FILE* fp, XPR_UPS_Entry* entry)
{
    if (!(XPR_UPS_ENTRY_IS_DIR(entry) || XPR_UPS_ENTRY_IS_INIT(entry) ||
          entry->type & XPR_UPS_ENTRY_FLAG_NOSTOR)) {
        fprintEntryKey(fp, entry);
        fprintEntryValue(fp, &entry->curVal, XPR_UPS_TO_TYPE(entry->type));
        fprintf(fp, "\n\n");
    }
    if (entry->node.childs)
        fprintEntry(fp, XPR_UPS_TO_ENTRY(entry->node.childs));
    if (entry->node.next)
        fprintEntry(fp, XPR_UPS_TO_ENTRY(entry->node.next));
}

XPR_API int XPR_UPS_Export(const char* fileName)
{
    FILE* fp = fopen(fileName, "wb");
    if (!fp)
        return XPR_ERR_SYS(errno);
    XPR_UPS_LOCK();
    fprintEntry(fp, XPR_UPS_TO_ENTRY(sRoot.node.childs));
    XPR_UPS_UNLOCK();
    fclose(fp);
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_Import(const char* url)
{
    return XPR_ERR_UPS_NOT_SUPPORT;
}

XPR_API int XPR_UPS_Pack(void)
{
    return XPR_ERR_UPS_NOT_SUPPORT;
}

// Print the typed value
static void printValue(XPR_UPS_Value* val, XPR_UPS_EntryType type)
{
    switch (type) {
    case XPR_UPS_ENTRY_TYPE_INIT:
        printf("-");
        break;
    case XPR_UPS_ENTRY_TYPE_DIR:
        printf("-");
        break;
    case XPR_UPS_ENTRY_TYPE_BOOLEAN:
        printf("%s", val->bl ? "true" : "false");
        break;
    case XPR_UPS_ENTRY_TYPE_BLOB:
        printf("(%p,%d)", val->bb.data, val->bb.size);
        break;
    case XPR_UPS_ENTRY_TYPE_I32:
        printf("%d", val->i32);
        break;
    case XPR_UPS_ENTRY_TYPE_I64:
        printf("%ld", val->i64);
        break;
    case XPR_UPS_ENTRY_TYPE_F32:
        printf("%f", val->f32);
        break;
    case XPR_UPS_ENTRY_TYPE_F64:
        printf("%lf", val->f64);
        break;
    case XPR_UPS_ENTRY_TYPE_STRING:
        if (val->str)
            printf("\"%s\"", val->str);
        else
            printf("(null)");
        break;
    default:
        break;
    }
}

// Print indent before the entry information
static void printIndent(int indent, int branch)
{
    for (int i = 0; i <= indent; i++) {
        if (branch && i == indent)
            printf("┊╌╌ ");
        else
            printf("┊   ");
    }
}

// Print the entry and it's childs and siblings
static void printEntry(XPR_UPS_Entry* entry)
{
    int depth = entryDepth(entry);
    XPR_UPS_EntryType type = XPR_UPS_TO_TYPE(entry->type);
    printIndent(depth - 1, 1);
    printf("%-20s{%s, %s}", entry->name, entry->category, entryTypeName(type));
    printf("[");
    printValue(&entry->curVal, type);
    printf(", ");
    printValue(&entry->defVal, type);
    printf(", ");
    printValue(&entry->shaVal, type);
    printf("]\n");
    printIndent(depth - 1, 0);
    printf("^? <<%s>>\n", entry->desc);
    if (entry->node.childs)
        printEntry(XPR_UPS_TO_ENTRY(entry->node.childs));
    if (entry->node.next)
        printEntry(XPR_UPS_TO_ENTRY(entry->node.next));
}

XPR_API int XPR_UPS_PrintAll(void)
{
    XPR_UPS_LOCK();
    printEntry(&sRoot);
    XPR_UPS_UNLOCK();
    return XPR_ERR_OK;
}

XPR_API int XPR_UPS_Sync(void)
{
    int err = XPR_ERR_OK;
    if (XPR_AtomicRead(&sHaveChanges) > 0) {
        XPR_UPS_LOCK();
        XPR_AtomicAssign(&sHaveChanges, 0);
        syncStorage();
        XPR_UPS_UNLOCK();
    }
    return err;
}
