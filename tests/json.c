#include <stdio.h>
#include <xpr/xpr_json.h>

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

int main(int argc, char* argv[])
{
    XPR_JSON* root_json = XPR_JSON_LoadFileName("./configuration.json");
    dump(root_json);
    XPR_JSON_DecRef(root_json);
    return 0;
}
