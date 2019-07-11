#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_xml.h>

static void test1(void)
{
    int i = 0;
    int nb = 0;
    int len = 0;
    XPR_XML_Node* root = XPR_XML_LoadDoc("doc.xml");
    XPR_XML_Node* child = 0;
    child = XPR_XML_GetChild(root, NULL, 0);
    nb = XPR_XML_GetChildNB(root);
    XPR_XML_Dump(root);
    while (child) {
        printf("number of childs = %d, first child = %p (%s)\n", nb, child, XPR_XML_GetName(child, NULL, 0));
        XPR_XML_Release(XPR_XML_RELEASE_LAST);
        for (i = 0; i < nb; i++) {
        }
        nb = XPR_XML_GetChildNB(child);
        child = XPR_XML_GetChild(child, NULL, 0);
    }
    // XPR_XML_GetContent allocate a buffer and store the content in it if no buffer was given
    printf("root content = '%s'\n", XPR_XML_GetContent(root, NULL, &len));
    // release the last allocated buffer even if no pointer is maintained by the user
    XPR_XML_Release(XPR_XML_RELEASE_LAST);
    // here no memory leak can occur.

    XPR_XML_Close(root);
}

static void test_XPR_XML_SaveBuffer()
{
    XPR_XML_Node* doc = XPR_XML_NewDoc();
    XPR_XML_Node* root = XPR_XML_AddElement(doc, "Root");
    XPR_XML_AddBoolean(root, "Boolean", 1);
    XPR_XML_AddBoolean(root, "Boolean", 0);
    XPR_XML_AddDouble(root, "Double", -3.141592654 - 9999999999);
    XPR_XML_AddDouble(root, "Double", 0.0);
    XPR_XML_AddDouble(root, "Double", 3.141592654 + 9999999999);
    XPR_XML_AddFloat(root, "Float", -3.1415926f);
    XPR_XML_AddFloat(root, "Float", 0.0f);
    XPR_XML_AddFloat(root, "Float", 3.1415926f);
    XPR_XML_AddInt(root, "Int", -1234567890);
    XPR_XML_AddInt(root, "Int", 0);
    XPR_XML_AddInt(root, "Int", 1234567890);
    XPR_XML_AddInt64(root, "Int64", 0x8000000000000000LL);
    XPR_XML_AddInt64(root, "Int64", 0);
    XPR_XML_AddInt64(root, "Int64", 0x7fffffffffffffffLL);
    XPR_XML_AddString(root, "String", "Hello, <<XPR XML>>!");
    XPR_XML_AddFormat(root, "Format", "[%d,%d,%d]", 1, 2, 3);
    char* buffer = NULL;
    if (XPR_XML_SaveBuffer(root, &buffer)) {
        printf("%s\n", buffer);
        free(buffer);
    }
    XPR_XML_Release(XPR_XML_RELEASE_ALL);
    XPR_XML_Close(doc);
}

static void test_XPR_XML_SaveFile()
{
    XPR_XML_Node* doc = XPR_XML_NewDoc();
    XPR_XML_Node* root = XPR_XML_AddElement(doc, "Root");
    XPR_XML_AddBoolean(root, "Boolean", 1);
    XPR_XML_AddBoolean(root, "Boolean", 0);
    XPR_XML_AddDouble(root, "Double", -3.141592654 - 9999999999);
    XPR_XML_AddDouble(root, "Double", 0.0);
    XPR_XML_AddDouble(root, "Double", 3.141592654 + 9999999999);
    XPR_XML_AddFloat(root, "Float", -3.1415926f);
    XPR_XML_AddFloat(root, "Float", 0.0f);
    XPR_XML_AddFloat(root, "Float", 3.1415926f);
    XPR_XML_AddInt(root, "Int", -1234567890);
    XPR_XML_AddInt(root, "Int", 0);
    XPR_XML_AddInt(root, "Int", 1234567890);
    XPR_XML_AddInt64(root, "Int64", 0x8000000000000000LL);
    XPR_XML_AddInt64(root, "Int64", 0);
    XPR_XML_AddInt64(root, "Int64", 0x7fffffffffffffffLL);
    XPR_XML_AddString(root, "String", "Hello, <<XPR XML>>!");
    XPR_XML_AddFormat(root, "Format", "[%d,%d,%d]", 1, 2, 3);
    XPR_XML_SaveFile(root, "doc-new.xml");
    XPR_XML_Release(XPR_XML_RELEASE_ALL);
    XPR_XML_Close(doc);
}

int main(int argc, char** argv)
{
    test1();
    test_XPR_XML_NewDoc();
    test_XPR_XML_SaveFile();
    return 0;
}
