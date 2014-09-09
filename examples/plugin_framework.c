#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xpr/xpr_plugin.h>

int main(int argc, char** argv)
{
    XPR_PluginLoadAll("plugins");
    return 0;
}

