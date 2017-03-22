#include <stdio.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_plugin.h>

int main(int argc, char* argv[])
{
	int result = 0;
	printf("=================\n");

	result = XPR_UPS_Init();
	if(result != 0)
		printf("XPR_UPS_Init rsult = s%d\n", result);

	printf("result = %d\n", result);
	printf("PluginLoadAll before!\n");

	XPR_PluginLoadAll("plugins");

	printf("PluginLoadAll over!\n");

	getchar();
	getchar();

	XPR_UPS_Fini();
	return 0;
}
