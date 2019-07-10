#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_json.h>

#if defined(_MSC_VER)
#  if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "libxprd.lib")
#  else
#pragma comment(lib, "libxpr.lib")
#  endif
#endif

static void print_indent(int n)
{
    putchar('|');
    while (--n > 0)
        putchar('-');
}

static void dump(XPR_JSON* jo)
{
    static int indent = 0;
    XPR_JSON* jn = 0;
    void* itr = XPR_JSON_ObjectIter(jo);
    indent += 4;
    while (itr) {
        print_indent(indent);
        printf("%s\n", XPR_JSON_ObjectIterKey(itr));
        jn = XPR_JSON_ObjectIterValue(itr);
        if (jn)
            dump(jn);
        itr = XPR_JSON_ObjectIterNext(jo, itr);
    }
    indent -= 4;
}

static void Test_Dump()
{
	XPR_JSON* root_json = XPR_JSON_LoadFileName("example.json");
	dump(root_json);
	XPR_JSON_DecRef(root_json);
}

static void Test_JSON_XPath()
{
	XPR_JSON* root = XPR_JSON_LoadFileName("example.json");
	printf("root %p\n", root);
	XPR_JSON* jx = NULL;
	jx = XPR_JSON_XPathGet(root, "/sys/cpu");
	printf("jx %p, %d\n", jx, XPR_JSON_IntegerValue(jx));

	jx = XPR_JSON_XPathGet(root, "/sys/cpus[0]");
	printf("jx %p, %d\n", jx, XPR_JSON_IntegerValue(jx));

	jx = XPR_JSON_String("Hello, New");
	XPR_JSON_XPathSetNew(root, "/sys/mems[4]/total", jx);

	XPR_JSON_XPathSetNew(root, "/sys/mems[3]", XPR_JSON_Null());
	XPR_JSON_XPathSetBoolean(root, "/sys/mems[2]/blocks[2]", 1);
	XPR_JSON_XPathSetInt(root, "/sys/mems[2]/blocks[3]", 123);
	XPR_JSON_XPathSetNew(root, "/sys/mems[2]/blocks[4]", XPR_JSON_Integer(1000));

	dump(root);
	XPR_JSON_DumpFileName(root, "example-dump.json");
	XPR_JSON_DecRef(root);
}


int main(int argc, char* argv[])
{
	Test_Dump();
	Test_JSON_XPath();
	return 0;
}
