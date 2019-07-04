#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_utils.h>

#if defined(_MSC_VER)
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "libxprd.lib")
#else
#pragma comment(lib, "libxpr.lib")
#endif
#endif

static void segment(void* opaque, char* seg)
{
    printf("SEGMENT: [%s]\n", seg);
}

static void test_xpr_foreach_s()
{
    printf("### %s\n", __FUNCTION__);
    xpr_foreach_s("hello world;are you ok?;", -1, ";", segment, NULL);
}

static const char* kDPatterns[] = {
    // No space
    "1,2",
    ".1,.2",
    "1.,2.",
    "1.0,2.0",
    // Left space
    "1 ,2",
    ".1 ,.2",
    "1. ,2.",
    "1.0 ,2.0",
    // Right space
    "1, 2",
    ".1, .2",
    "1., 2.",
    "1.0, 2.0",
    // Both space
    "1 , 2",
    ".1 , .2",
    "1. , 2.",
    "1.0 , 2.0",
    // With brace
    "[1 , 2]",
    "[.1 , .2]",
    "[1. , 2.]",
    "[1.0 , 2.0]",
    "[ 1 , 2 ]",
    "[ .1 , .2 ]",
    "[ 1. , 2. ]",
    "[ 1.0 , 2.0 ]",
    // Large number
    "123456789.123456789,1234567890123456789.1234567890123456789",
};
#define kNumDPattern sizeof(kDPatterns) / sizeof(kDPatterns[0])

static const char* kIPatterns[] = {
    // No space
    "123,234",
    "1234,2345",
    "12345,23456",
    "123456,234567",
    // Left space
    "123 ,234",
    "1234 ,2345",
    "12345 ,23456",
    "123456 ,234567",
    // Right space
    "123, 234",
    "1234, 2345",
    "12345, 23456",
    "123456, 234567",
    // Both space
    "123 , 234",
    "1234 , 2345",
    "12345 , 23456",
    "123456 , 234567",
    // With brace
    "[123 , 234]",
    "[1234 , 2345]",
    "[12345 , 23456]",
    "[123456 , 234567]",
    // Large number
    "1234567890,1234567890123456789",
};
#define kNumIPattern sizeof(kIPatterns) / sizeof(kIPatterns[0])

static void test_xpr_s2dvec2()
{
    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < kNumDPattern; i++) {
        double v[2];
        int err = xpr_s2dvec2(kDPatterns[i], v);
        if (err == XPR_ERR_OK)
            printf("[%2d] %20s = [%lf,%lf]\n", i, kDPatterns[i], v[0], v[1]);
        else
            printf("[%2d] %20s convert error: %08X\n", i, kDPatterns[i], err);
    }
}

static void test_xpr_s2fvec2()
{
    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < kNumDPattern; i++) {
        float v[2];
        int err = xpr_s2fvec2(kDPatterns[i], v);
        if (err == XPR_ERR_OK)
            printf("[%2d] %20s = [%f,%f]\n", i, kDPatterns[i], v[0], v[1]);
        else
            printf("[%2d] %20s convert error: %08X\n", i, kDPatterns[i], err);
    }
}

static void test_xpr_s2ivec2()
{
    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < kNumIPattern; i++) {
        int v[2];
        int err = xpr_s2ivec2(kIPatterns[i], v);
        if (err == XPR_ERR_OK)
            printf("[%2d] %20s = [%d,%d]\n", i, kIPatterns[i], v[0], v[1]);
        else
            printf("[%2d] %20s convert error: %08X\n", i, kIPatterns[i], err);
    }
}

static const char* kResPatterns[] = {
    "123x123",
    "1234x1234",
    "12345x12345",
    "123 x 123",
    "1234 x 1234",
    "12345 x 12345",
    "123X123",
    "1234X1234",
    "12345X12345",
    "123 X 123",
    "1234 X 1234",
    "12345 X 12345",
};
#define kNumResPattern sizeof(kResPatterns) / sizeof(kResPatterns[0])

static void test_xpr_s2res()
{
    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < kNumResPattern; i++) {
        int v[2];
        int err = xpr_s2res(kResPatterns[i], &v[0], &v[1]);
        if (err == XPR_ERR_OK)
            printf("[%2d] %20s = %dx%d\n", i, kResPatterns[i], v[0], v[1]);
        else
            printf("[%2d] %20s convert error: %08X\n", i, kResPatterns[i], err);
    }
}

int main(int argc, char** argv)
{
    test_xpr_foreach_s();
    test_xpr_s2dvec2();
    test_xpr_s2fvec2();
    test_xpr_s2ivec2();
    test_xpr_s2res();
    XPR_SYS_WaitKey(10 * XPR_SYS_CTS_UNIT);
    return 0;
}
