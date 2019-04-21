#include <xpr/xpr_utils.h>
#include <stdio.h>
#include <stdlib.h>

extern void benchmark(void);

int main(int argc, char** argv)
{
    char* env = getenv("DEBUG");
    if (env) {
        xpr_dbg_set_level(strtol(env, NULL, 10));
    }
    benchmark();
    return 0;
}