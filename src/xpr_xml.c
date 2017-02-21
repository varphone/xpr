#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_utils.h>
#include <xpr/xpr_xml.h>
#include "deps/roxml/roxml.h"

XPR_XML_Node* XPR_XML_LoadBuf(char* buffer)
{
    return (XPR_XML_Node*)roxml_load_buf(buffer);
}

XPR_XML_Node* XPR_XML_LoadDoc(char* filename)
{
    return (XPR_XML_Node*)roxml_load_doc(filename);
}

XPR_XML_Node* XPR_XML_LoadFd(int fd)
{
    return (XPR_XML_Node*)roxml_load_fd(fd);
}

void XPR_XML_Close(XPR_XML_Node* node)
{
    roxml_close((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetNextSibling(XPR_XML_Node* node)
{
    return (XPR_XML_Node*)roxml_get_next_sibling((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetPrevSibling(XPR_XML_Node* node)
{
    return (XPR_XML_Node*)roxml_get_prev_sibling((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetParent(XPR_XML_Node* node)
{
    return (XPR_XML_Node*)roxml_get_parent((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetRoot(XPR_XML_Node* node)
{
    return (XPR_XML_Node*)roxml_get_root((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetNS(XPR_XML_Node* node)
{
    return (XPR_XML_Node*)roxml_get_ns((node_t*)node);
}

XPR_XML_Node* XPR_XML_SetNS(XPR_XML_Node* node, XPR_XML_Node* ns)
{
    return (XPR_XML_Node*)roxml_set_ns((node_t*)node, (node_t*)ns);
}

XPR_XML_Node* XPR_XML_GetComment(XPR_XML_Node* node, int nth)
{
    return (XPR_XML_Node*)roxml_get_cmt((node_t*)node, nth);
}

int XPR_XML_GetCommentNB(XPR_XML_Node* node)
{
    return roxml_get_cmt_nb((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetChild(XPR_XML_Node* node, char* name, int nth)
{
    return (XPR_XML_Node*)roxml_get_chld((node_t*)node, name, nth);
}

int XPR_XML_GetChildNB(XPR_XML_Node* node)
{
    return roxml_get_chld_nb((node_t*)node);
}

/// @brief Process-Instruction getter function
///
/// This function returns the nth process-instruction of a node
///
XPR_XML_Node* XPR_XML_GetPi(XPR_XML_Node* node, int nth)
{
    return (XPR_XML_Node*)roxml_get_pi((node_t*)node, nth);
}

int XPR_XML_GetPiNB(XPR_XML_Node* node)
{
    return roxml_get_pi_nb((node_t*)node);
}

char* XPR_XML_GetName(XPR_XML_Node* node, char* buffer, int size)
{
    return roxml_get_name((node_t*)node, buffer, size);
}

char* XPR_XML_GetContent(XPR_XML_Node* node, char* buffer, int* size)
{
    return roxml_get_content((node_t*)node, buffer, size ? *size : 0, size);
}

void XPR_XML_SetContent(XPR_XML_Node* node, char* content)
{
    node_t* txt = roxml_get_txt((node_t*)node, 0);
    if (txt != NULL) {
        roxml_del_node(txt);
    }
    roxml_add_node((node_t*)node, 0, ROXML_TXT_NODE, NULL, content);
}

XPR_XML_Node*  XPR_XML_GetNodes(XPR_XML_Node* node, int type, char* name, int nth)
{
    return (XPR_XML_Node*)roxml_get_nodes((node_t*)node, type, name, nth);
}

int XPR_XML_GetNodesNB(XPR_XML_Node* node, int type)
{
    return roxml_get_nodes_nb((node_t*)node, type);
}

XPR_XML_Node* XPR_XML_GetAttr(XPR_XML_Node* node, char* name, int nth)
{
    return (XPR_XML_Node*)roxml_get_attr((node_t*)node, name, nth);
}

int XPR_XML_GetAttrNB(XPR_XML_Node* node)
{
    return roxml_get_attr_nb((node_t*)node);
}

XPR_XML_Node* XPR_XML_GetText(XPR_XML_Node* node, int nth)
{
    return (XPR_XML_Node*)roxml_get_txt((node_t*)node, nth);
}

int XPR_XML_GetTextNB(XPR_XML_Node* node)
{
    return roxml_get_txt_nb((node_t*)node);
}

XPR_XML_Node** XPR_XML_XPath(XPR_XML_Node* node, char* path, int* nb)
{
    if (!node || !path)
        return NULL;
    return (XPR_XML_Node**)roxml_xpath((node_t*)node, path, nb);
}

int XPR_XML_GetType(XPR_XML_Node* node)
{
    return roxml_get_type((node_t*)node);
}

int XPR_XML_GetNodePosition(XPR_XML_Node* node)
{
    return roxml_get_node_position((node_t*)node);
}

void XPR_XML_Release(void* data)
{
    roxml_release(data);
}

XPR_XML_Node* XPR_XML_AddNode(XPR_XML_Node* parent, int position, int type, char* name, char* value)
{
    return (XPR_XML_Node*)roxml_add_node((node_t*)parent, position, type, name, value);
}

void XPR_XML_DelNode(XPR_XML_Node* node)
{
    roxml_del_node((node_t*)node);
}

int XPR_XML_CommitChanges(XPR_XML_Node* node, char* dest, char** buffer, int human)
{
    return roxml_commit_changes((node_t*)node, dest, buffer, human);
}

static void DumpAttr(XPR_XML_Node* node)
{
    int len = 0;
    char* vals[2] = {0};
    if (node) {
        vals[0] = XPR_XML_GetName(node, NULL, 0);
        vals[1] = XPR_XML_GetContent(node, NULL, &len);
        printf(" %s=\"%s\"", vals[0], vals[1]);
        XPR_XML_Release(vals[0]);
        XPR_XML_Release(vals[1]);
        DumpAttr(XPR_XML_GetNextSibling(node));
    }
}

void XPR_XML_Dump(XPR_XML_Node* node)
{
    int len = 0;
    char* vals[4] = {0};
    XPR_XML_Node* n = 0;
    if (node) {
        vals[0] = XPR_XML_GetName(node, NULL, 0);
        n = XPR_XML_GetAttr(node, NULL, 0);
        printf("<%s", vals[0]);
        DumpAttr(XPR_XML_GetAttr(node, NULL, 0));
        printf(">");
        vals[1] = XPR_XML_GetContent(node, NULL, &len);
        if (vals[1]) {
            printf("%s", vals[1]);
            XPR_XML_Release(vals[1]);
        }
        if (XPR_XML_GetChildNB(node) > 0) {
            XPR_XML_Dump(XPR_XML_GetChild(node, NULL, 0));
            printf("\n");
        }
        printf("</%s>\n", vals[0]);
        XPR_XML_Release(vals[0]);
        XPR_XML_Dump(XPR_XML_GetNextSibling(node));
    }
}

XPR_XML_Node* XPR_XML_XGetNode(XPR_XML_Node* node, char* path, int nth)
{
    int nb = 0;
    XPR_XML_Node* ret = 0;
    XPR_XML_Node** nodes = XPR_XML_XPath(node, path, &nb);
    if (nb > nth)
        ret = nodes[nth];
    if (nodes)
        XPR_XML_Release(nodes);
    return ret;
}

int XPR_XML_XGetNodeNB(XPR_XML_Node* node, char* path)
{
    int nb = 0;
    XPR_XML_Node** nodes = XPR_XML_XPath(node, path, &nb);
    if (nodes)
        XPR_XML_Release(nodes);
    return nb;
}

char* XPR_XML_XGetContent(XPR_XML_Node* node, char* path, int nth, char* buffer, int* size)
{
    int nb = 0;
    XPR_XML_Node** nodes = 0;
    if (path)
        nodes = XPR_XML_XPath(node, path, &nb);
    if (nodes && nb > nth) {
        if (buffer)
            XPR_XML_GetContent(nodes[nth], buffer, size);
        else
            buffer = XPR_XML_GetContent(nodes[nth], NULL, NULL);
    }
    else if (!nodes && !path) {
        buffer = XPR_XML_GetContent(node, buffer, size); 
    }
    else {
        buffer = NULL;
    }
    if (nodes)
        XPR_XML_Release(nodes);
    return buffer;
}

int XPR_XML_XGetInt(XPR_XML_Node* node, char* path, int nth, int* value)
{
    int size = 256;
    char buffer[256];
    if (!node || !path || !value)
        return XPR_ERR_ERROR;
    if (XPR_XML_XGetContent(node, path, nth, buffer, &size) == NULL)
        return XPR_ERR_ERROR;
    if (value) {
        *value = strtol(buffer, 0, 0);
    }
    return XPR_ERR_OK;
}

int XPR_XML_XGetInt64(XPR_XML_Node* node, char* path, int nth, int64_t* value)
{
    int size = 256;
    char buffer[256];
    if (!node || !path || !value)
        return XPR_ERR_ERROR;
    if (XPR_XML_XGetContent(node, path, nth, buffer, &size) == NULL)
        return XPR_ERR_ERROR;
    if (value) {
        *value = strtoll(buffer, 0, 0);
    }
    return XPR_ERR_OK;
}

int XPR_XML_XGetFloat(XPR_XML_Node* node, char* path, int nth, float* value)
{
    int size = 256;
    char buffer[256];
    if (!node || !path || !value)
        return XPR_ERR_ERROR;
    if (XPR_XML_XGetContent(node, path, nth, buffer, &size) == NULL)
        return XPR_ERR_ERROR;
    if (value) {
        *value = strtof(buffer, 0);
    }
    return XPR_ERR_OK;
}

int XPR_XML_XGetDouble(XPR_XML_Node* node, char* path, int nth, double* value)
{
    int size = 256;
    char buffer[256];
    if (!node || !path || !value)
        return XPR_ERR_ERROR;
    if (XPR_XML_XGetContent(node, path, nth, buffer, &size) == NULL)
        return XPR_ERR_ERROR;
    if (value) {
        *value = strtod(buffer, 0);
    }
    return XPR_ERR_OK;
}

int XPR_XML_XGetBoolean(XPR_XML_Node* node, char* path, int nth, int* value)
{
    int size = 256;
    char buffer[256];
    char* p = 0;
    if (!node || !path || !value)
        return XPR_ERR_ERROR;
    if (XPR_XML_XGetContent(node, path, nth, buffer, &size) == NULL)
        return XPR_ERR_ERROR;
    p = xpr_skip_blank(buffer);
    if (value) {
        if (p[0] == 't' ||
            p[0] == 'T' ||
            p[0] == 'y' ||
            p[0] == 'Y' ||
            p[0] == '1')
            *value = 1;
        else
            *value = 0;
    }
    return XPR_ERR_OK;
}
