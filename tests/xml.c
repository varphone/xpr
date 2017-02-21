#include <stdio.h>
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

int main(int argc, char** argv)
{
    test1();
    return 0;
}
