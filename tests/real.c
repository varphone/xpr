#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLT_INT(s,e,r)    (unsigned int)(((unsigned int)(s) << 31) | (((unsigned int)(e) << 2) << 22) | (unsigned int)(r))

static void df(float* f)
{
    char* c = (char*)f;
    printf("%.6f, %02hhx %02hhx %02hhx %02hhx\n", *f, c[0], c[1], c[2], c[3]);
}

static void dfb(float* f)
{
    unsigned int i = *(unsigned int*)f;
    printf("%.6f, s: %x, e: %x, r: %x\n", *f, i >> 31, (i >> 23) & 0xff, i & 0x007fffff);
}

int main(int argc, char** argv)
{
    unsigned int i = 0xffffffff;
    float* f = (float*)&i;
    float f0 = 0.0;
    float f1 = 1.0;
    float f2 = -1.0;
    float f3 = 1.999999;
    float f4 = 999999.999999;
    float f5 = 1000000.999999;
    float f6 = 10000000.999999;
    float f7 = 2.0;
    float f8 = 2.999999;
    float f9 = 99.999999;
    float f10 = 999.999999;
    df(&f0);
    df(&f1);
    df(&f2);
    df(&f3);
    df(&f4);
    df(&f5);
    df(&f6);
    i = FLT_INT(1, 127+1, 1);
    df(f);
    i = FLT_INT(0, 127+10, 999);
    df(f);
    i = FLT_INT(0, 127+23, 3);
    df(f);

    dfb(&f0);
    dfb(&f1);
    dfb(&f2);
    dfb(&f3);
    dfb(&f4);
    dfb(&f7);
    dfb(&f8);
    dfb(&f9);
    dfb(&f10);

    printf("float: %g %e, %g %e\n", FLT_MIN, FLT_MIN, FLT_MAX, FLT_MAX);
    printf("double: %f(%e), %f(%e)\n", DBL_MIN, DBL_MIN, DBL_MAX, DBL_MAX);
    return 0;
}