#include <stdio.h>
#include <xpr/xpr_json.h>

int main(int argc, char* argv[])
{
	XPR_JSON* root_json = XPR_JSON_LoadFileName("./configuration.json");
	/*printf("root_json=%s\n",  XPR_JSON_DumpString(root_json));
	XPR_JSON* js = XPR_JSON_ObjectGet(root_json, "/");
	printf("js_json=%s\n",  XPR_JSON_DumpString(js));
	js = XPR_JSON_ObjectGet(js, "system");
	printf("js_json=%s\n",  XPR_JSON_DumpString(js));
	XPR_JSON_DumpFileName(root_json, "./test.json");
*/
	XPR_JSON_DecRef(root_json);
	return 0;
}
