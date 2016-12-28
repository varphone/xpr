#ifndef XPR_XML_H
#define XPR_XML_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// XPR_XML_Release() 特殊参数，用于释放当前线程分配的所有资源
///
/// @sa #XPR_XML_RELEASE_LAST
#define XPR_XML_RELEASE_ALL     (void*)-1

///
/// XPR_XML_Release() 特殊参数，用于释放当前线程最后一次分配的资源
///
/// @remark 使用示例
/// @code
/// {
///     int len = 0;
///     XPR_XML_Node* root = XPR_XML_LoadDoc("doc.xml");
///     // 当不指定 buffer 参数时, XPR_XML_GetContent() 会返回动态分配的缓冲区
///     printf("root content = '%s'\n", XPR_XML_GetContent(root, NULL, 0, &len));
///
///     // 释放最后一次分配的缓冲区
///     XPR_XML_Release(XPR_XML_RELEASE_LAST);
///
///     // 这里不会发生内存泄漏的问题
///
///     XPR_XML_Close(root);
/// }
/// @endcode
///
#define XPR_XML_RELEASE_LAST    (void*)-2

#ifndef XPR_XML_NODETYPE_TYPE_DEFINED
#define XPR_XML_NODETYPE_TYPE_DEFINED
typedef enum XPR_XML_NodeType {
    XPR_XML_NODE_TYPE_ATTR = 0x008,
    XPR_XML_NODE_TYPE_STD = 0x010,
    XPR_XML_NODE_TYPE_ELM = 0x010,
    XPR_XML_NODE_TYPE_TXT = 0x020,
    XPR_XML_NODE_TYPE_CMT = 0x040,
    XPR_XML_NODE_TYPE_PI = 0x080,
    XPR_XML_NODE_TYPE_NS = 0x100,
    XPR_XML_NODE_TYPE_NSDEF = XPR_XML_NODE_TYPE_NS | XPR_XML_NODE_TYPE_ATTR,
    XPR_XML_NODE_TYPE_CDATA = XPR_XML_NODE_TYPE_TXT | 0x200,
    XPR_XML_NODE_TYPE_DOCTYPE = 0x400,
    XPR_XML_NODE_TYPE_ALL = 0x5f8,
} XPR_XML_NodeType;
#endif // XPR_XML_NODETYPE_TYPE_DEFINED

#ifndef XPR_XML_NODE_TYPE_DEFINED
#define XPR_XML_NODE_TYPE_DEFINED
struct XPR_XML_Node;
typedef struct XPR_XML_Node XPR_XML_Node;
#endif // XPR_XMLNODE_TYPE_DEFINED

XPR_API XPR_XML_Node* XPR_XML_LoadBuf(char* buffer);
XPR_API XPR_XML_Node* XPR_XML_LoadDoc(char* filename);
XPR_API XPR_XML_Node* XPR_XML_LoadFd(int fd);
XPR_API void XPR_XML_Close(XPR_XML_Node* node);

XPR_API XPR_XML_Node* XPR_XML_GetNextSibling(XPR_XML_Node* node);
XPR_API XPR_XML_Node* XPR_XML_GetPrevSibling(XPR_XML_Node* node);
XPR_API XPR_XML_Node* XPR_XML_GetParent(XPR_XML_Node* node);
XPR_API XPR_XML_Node* XPR_XML_GetRoot(XPR_XML_Node* node);
XPR_API XPR_XML_Node* XPR_XML_GetNS(XPR_XML_Node* node);
XPR_API XPR_XML_Node* XPR_XML_SetNS(XPR_XML_Node* node, XPR_XML_Node* ns);

XPR_API XPR_XML_Node* XPR_XML_GetComment(XPR_XML_Node* node, int nth);
XPR_API int XPR_XML_GetCommentNB(XPR_XML_Node* node);

XPR_API XPR_XML_Node* XPR_XML_GetChild(XPR_XML_Node* node,char* name, int nth);
XPR_API int XPR_XML_GetChildNB(XPR_XML_Node* node);

/// @brief Process-Instruction getter function
///
/// This function returns the nth process-instruction of a node
///
XPR_API XPR_XML_Node* XPR_XML_GetPi(XPR_XML_Node* node, int nth);
XPR_API int XPR_XML_GetPiNB(XPR_XML_Node* node);

XPR_API char* XPR_XML_GetName(XPR_XML_Node* node, char* buffer, int size);
XPR_API char* XPR_XML_GetContent(XPR_XML_Node* node, char* buffer, int* size);
XPR_API void XPR_XML_SetContent(XPR_XML_Node* node, char* content);

XPR_API XPR_XML_Node* XPR_XML_GetNodes(XPR_XML_Node* node, int type, char* name, int nth);
XPR_API int XPR_XML_GetNodesNB(XPR_XML_Node* node, int type);

XPR_API XPR_XML_Node* XPR_XML_GetAttr(XPR_XML_Node* node, char* name, int nth);
XPR_API int XPR_XML_GetAttrNB(XPR_XML_Node* node);

XPR_API XPR_XML_Node* XPR_XML_GetText(XPR_XML_Node* node, int nth);
XPR_API int XPR_XML_GetTextNB(XPR_XML_Node* node);

///
/// 执行 XPath 操作
///
/// @param [in] node        要执行操作的节点
/// @param [in] path        XPath 表达式
/// @param [in,out] nb      返回符合 XPath 表达式的节点数量
/// @retval NULL 执行失败或无符合内容
/// @retval Other 符合 XPath 表达式的节点列表
/// @note 当返回值不再使用时请调用 XPR_XML_Release() 释放其占用的资源
/// @remark 使用示例
/// @code
/// {
///     int i = 0;
///     int nb = 0;
///     XPR_XML_Node* root = XPR_XML_LoadDoc("doc.xml");
///     XPR_XML_Node** nodes = XPR_XML_XPath(root, "//page/item[lang=en]", &nb);
///     for (i = 0; i < nb; i++) {
///         printf("content = %s\n", XPR_XML_GetContent(nodes[i], NULL, 0));
///         XPR_XML_Release(XPR_XML_RELEASE_LAST);
///     }
///     XPR_XML_Release(nodes);
///     XPR_XML_Close(root);
/// }
/// @endcode
///
XPR_API XPR_XML_Node** XPR_XML_XPath(XPR_XML_Node* node, char* path, int* nb);

///
/// 获取节点类型
///
/// @param [in] node        要操作的节点
/// @return 节点类型值 [#XPR_XML_NodeType]
XPR_API int XPR_XML_GetType(XPR_XML_Node* node);

///
/// 获取节点在所有兄弟节点中的索引
///
/// @param [in] node        要操作的节点
/// @return 返回范围为 1 ~ N 的索引值
XPR_API int XPR_XML_GetNodePosition(XPR_XML_Node* node);

///
/// 释放由 XPR_XML 分配的内存
///
/// @param [in] data        要释放的内存地址
/// @return 无
XPR_API void XPR_XML_Release(void* data);

XPR_API XPR_XML_Node* XPR_XML_AddNode(XPR_XML_Node* parent, int position, int type, char* name, char* value);
XPR_API void XPR_XML_DelNode(XPR_XML_Node* node);

XPR_API int XPR_XML_CommitChanges(XPR_XML_Node* node, char* dest, char** buffer, int human);

XPR_API void XPR_XML_Dump(XPR_XML_Node* node);

///
/// 通过 XPath 获取指定节点的符合表达式的元素
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @return 返回符合表达式的元
XPR_API XPR_XML_Node* XPR_XML_XGetNode(XPR_XML_Node* node, char* path, int nth);

///
/// 通过 XPath 获取指定节点的符合表达式的元素数
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @return 返回符合表达式的元素数
XPR_API int XPR_XML_XGetNodeNB(XPR_XML_Node* node, char* path);

///
/// 通过 XPath 获取指定节点内容
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @param [in] nth         要操作的节点在列表中的位置
/// @param [in,out] value   存放返回值的地址
/// @retval XPR_ERR_OK      获取成功
/// @retval XPR_ERR_ERROR   获取失败
/// @sa XPR_XML_XGetInt()
XPR_API char* XPR_XML_XGetContent(XPR_XML_Node* node, char* path, int nth, char* buffer, int* size);

///
/// 通过 XPath 获取指定节点内容, 并将内容转为 int 数值
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @param [in] nth         要操作的节点在列表中的位置
/// @param [in,out] value   存放返回值的地址
/// @retval XPR_ERR_OK      获取成功
/// @retval XPR_ERR_ERROR   获取失败
/// @remark 使用示例
/// @code
/// {
///    int i = 0;
///    int nb = 0;
///    int value = 0;
///    XPR_XML_Node* root = XPR_XML_LoadDoc("doc.xml");
///    nb = XPR_XML_XGetNB(root, "/root/int/v");
///    printf("matches: %d\n", nb);
///    for (; i < nb; i++) {
///        XPR_XML_XGetInt(root, "/root/int/v", i, &value);
///        printf("[%d] value: %d\n", i, value);
///    }
///    XPR_XML_Close(root);
/// }
/// @endcode
///
XPR_API int XPR_XML_XGetInt(XPR_XML_Node* node, char* path, int nth, int* value);

///
/// 通过 XPath 获取指定节点内容, 并将内容转为 int64_t 数值
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @param [in] nth         要操作的节点在列表中的位置
/// @param [in,out] value   存放返回值的地址
/// @retval XPR_ERR_OK      获取成功
/// @retval XPR_ERR_ERROR   获取失败
/// @sa XPR_XML_XGetInt()
XPR_API int XPR_XML_XGetInt64(XPR_XML_Node* node, char* path, int nth, int64_t* value);

///
/// 通过 XPath 获取指定节点内容, 并将内容转为 float 数值
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @param [in] nth         要操作的节点在列表中的位置
/// @param [in,out] value   存放返回值的地址
/// @retval XPR_ERR_OK      获取成功
/// @retval XPR_ERR_ERROR   获取失败
/// @sa XPR_XML_XGetInt()
XPR_API int XPR_XML_XGetFloat(XPR_XML_Node* node, char* path, int nth, float* value);

///
/// 通过 XPath 获取指定节点内容, 并将内容转为 double 数值
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @param [in] nth         要操作的节点在列表中的位置
/// @param [in,out] value   存放返回值的地址
/// @retval XPR_ERR_OK      获取成功
/// @retval XPR_ERR_ERROR   获取失败
/// @sa XPR_XML_XGetInt()
XPR_API int XPR_XML_XGetDouble(XPR_XML_Node* node, char* path, int nth, double* value);

///
/// 通过 XPath 获取指定节点内容, 并将内容转为 boolean 数值
///
/// @param [in] node        要操作的根节点
/// @param [in] char        要查询的 XPath 表达式
/// @param [in] nth         要操作的节点在列表中的位置
/// @param [in,out] value   存放返回值的地址
/// @retval XPR_ERR_OK      获取成功
/// @retval XPR_ERR_ERROR   获取失败
/// @sa XPR_XML_XGetInt()
XPR_API int XPR_XML_XGetBoolean(XPR_XML_Node* node, char* path, int nth, int* value);

#ifdef __cplusplus
}
#endif

#endif // XPR_XML_H

