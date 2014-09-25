#ifndef XPR_XML_H
#define XPR_XML_H

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

XPR_XML_Node* XPR_XML_LoadBuf(char* buffer);
XPR_XML_Node* XPR_XML_LoadDoc(char* filename);
XPR_XML_Node* XPR_XML_LoadFd(int fd);
void XPR_XML_Close(XPR_XML_Node* node);

XPR_XML_Node* XPR_XML_GetNextSibling(XPR_XML_Node* node);
XPR_XML_Node* XPR_XML_GetPrevSibling(XPR_XML_Node* node);
XPR_XML_Node* XPR_XML_GetParent(XPR_XML_Node* node);
XPR_XML_Node* XPR_XML_GetRoot(XPR_XML_Node* node);
XPR_XML_Node* XPR_XML_GetNS(XPR_XML_Node* node);
XPR_XML_Node* XPR_XML_SetNS(XPR_XML_Node* node, XPR_XML_Node* ns);

XPR_XML_Node* XPR_XML_GetComment(XPR_XML_Node* node, int nth);
int XPR_XML_GetCommentNB(XPR_XML_Node* node);

XPR_XML_Node* XPR_XML_GetChild(XPR_XML_Node* node,char* name, int nth);
int XPR_XML_GetChildNB(XPR_XML_Node* node);

/// @brief Process-Instruction getter function
///
/// This function returns the nth process-instruction of a node
///
XPR_XML_Node* XPR_XML_GetPi(XPR_XML_Node* node, int nth);
int XPR_XML_GetPiNB(XPR_XML_Node* node);

char* XPR_XML_GetName(XPR_XML_Node* node, char* buffer, int size);
char* XPR_XML_GetContent(XPR_XML_Node* node, char* buffer, int* size);
void XPR_XML_SetContent(XPR_XML_Node* node, char* content);

XPR_XML_Node* XPR_XML_GetNodes(XPR_XML_Node* node, int type, char* name, int nth);
int XPR_XML_GetNodesNB(XPR_XML_Node* node, int type);

XPR_XML_Node* XPR_XML_GetAttr(XPR_XML_Node* node, char* name, int nth);
int XPR_XML_GetAttrNB(XPR_XML_Node* node);

XPR_XML_Node* XPR_XML_GetText(XPR_XML_Node* node, int nth);
int XPR_XML_GetTextNB(XPR_XML_Node* node);

/// @brief 执行 XPath 操作
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
XPR_XML_Node** XPR_XML_XPath(XPR_XML_Node* node, char* path, int* nb);

/// @brief 获取节点类型
/// @param [in] node        要操作的节点
/// @return 节点类型值 [#XPR_XML_NodeType]
int XPR_XML_GetType(XPR_XML_Node* node);

/// @brief 获取节点在所有兄弟节点中的索引
/// @param [in] node        要操作的节点
/// @return 返回范围为 1 ~ N 的索引值
int XPR_XML_GetNodePosition(XPR_XML_Node* node);

/// @brief 释放由 XPR_XML 分配的内存
/// @param [in] data        要释放的内存地址
/// @return 无
void XPR_XML_Release(void* data);

XPR_XML_Node* XPR_XML_AddNode(XPR_XML_Node* parent, int position, int type, char* name, char* value);
void XPR_XML_DelNode(XPR_XML_Node* node);

int XPR_XML_CommitChanges(XPR_XML_Node* node, char* dest, char** buffer, int human);

void XPR_XML_Dump(XPR_XML_Node* node);

#ifdef __cplusplus
}
#endif

#endif // XPR_XML_H
