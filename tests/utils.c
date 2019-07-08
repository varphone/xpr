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

static void test_xpr_path_basename()
{
    static const char* ps[] = {
        "/",
        "//",
        "/a",
        "/ ",
        "/short",
        "//short",
        "/root/dir/name",
        "/root/dir/name ",
        "/root/dir/ name ",
        "/root/dir/",
        "/root/dir//",
        "/root/dir/"
        "verylonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglo"
        "nglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong"
        "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglo"
        "nglonglonglonglonglonglong",
    };
    static int nps = sizeof(ps) / sizeof(ps[0]);

    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < nps; i++) {
        char buf[256];
        int err = xpr_path_basename(ps[i], buf, 256);
        if (err == XPR_ERR_OK)
            printf("[%2d] %20s = \"%s\"\n", i, ps[i], buf);
        else
            printf("[%2d] %20s parse error: %08X\n", i, ps[i], err);
    }
}

static void test_xpr_path_last_dirname()
{
    static const char* ps[] = {
        "/",
        "//",
        "/ ",
        "// "
        "/aaa",
        "/aaa ",
        "/aaa/bbbb/",
        "/aaa/bbbb/ ",
        "/aaa/bbbb//",
        "/aaa/bbbb/ccccc",
        "/aaa/bbbb/ccccc ",
        "/aaa/verylonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglo"
        "nglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong"
        "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglo"
        "nglonglonglonglonglonglong/",
        "/aaa/verylonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglo"
        "nglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong"
        "longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglo"
        "nglonglonglonglonglonglong/ccccc",
    };
    static int nps = sizeof(ps) / sizeof(ps[0]);

    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < nps; i++) {
        char buf[256];
        int err = xpr_path_last_dirname(ps[i], buf, 256);
        if (err == XPR_ERR_OK)
            printf("[%2d] %20s = \"%s\"\n", i, ps[i], buf);
        else
            printf("[%2d] %20s parse error: %08X\n", i, ps[i], err);
    }
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

static const char* kLowerCaseStrs[] = {"hello", "world", "hello,world", NULL};
static const char* kUpperCaseStrs[] = {"Hello", "World", "Hello,World", NULL};
#define kNumLowerCaseStrs _countof(kLowerCaseStrs)
#define kNumUpperCaseStrs _countof(kUpperCaseStrs)

static void test_xpr_stridx()
{
    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < kNumLowerCaseStrs; i++) {
        const char* s1 = kLowerCaseStrs[i];
        printf("[L/L] %20s index=%d\n", s1, xpr_stridx(s1, kLowerCaseStrs));
        const char* s2 = kUpperCaseStrs[i];
        printf("[U/U] %20s index=%d\n", s2, xpr_stridx(s2, kUpperCaseStrs));
        const char* s3 = kLowerCaseStrs[i];
        printf("[L/U] %20s index=%d\n", s3, xpr_stridx(s3, kUpperCaseStrs));
        const char* s4 = kUpperCaseStrs[i];
        printf("[U/L] %20s index=%d\n", s4, xpr_stridx(s4, kLowerCaseStrs));
    }
}

static void test_xpr_striidx()
{
    printf("### %s\n", __FUNCTION__);
    for (int i = 0; i < kNumLowerCaseStrs; i++) {
        const char* s1 = kLowerCaseStrs[i];
        printf("[L/L] %20s index=%d\n", s1, xpr_striidx(s1, kLowerCaseStrs));
        const char* s2 = kUpperCaseStrs[i];
        printf("[U/U] %20s index=%d\n", s2, xpr_striidx(s2, kUpperCaseStrs));
        const char* s3 = kLowerCaseStrs[i];
        printf("[L/U] %20s index=%d\n", s3, xpr_striidx(s3, kUpperCaseStrs));
        const char* s4 = kUpperCaseStrs[i];
        printf("[U/L] %20s index=%d\n", s4, xpr_striidx(s4, kLowerCaseStrs));
    }
}

static const char* kCaseStrs[] = {
    "hello,world!",
    "Hello,World!",
    "HELLO,WORLD!",
};
#define kNumCaseStrs _countof(kCaseStrs)

static void test_xpr_tolower()
{
    printf("### %s\n", __FUNCTION__);
    char buf[1024];
    for (int i = 0; i < kNumCaseStrs; i++) {
        const char* s = kCaseStrs[i];
        printf("[L] %s ==> %s\n", s, xpr_tolower(s, buf, sizeof(buf)));
    }
}

static void test_xpr_toupper()
{
    printf("### %s\n", __FUNCTION__);
    char buf[1024];
    for (int i = 0; i < kNumCaseStrs; i++) {
        const char* s = kCaseStrs[i];
        printf("[U] %s ==> %s\n", s, xpr_toupper(s, buf, sizeof(buf)));
    }
}

int main(int argc, char** argv)
{
    test_xpr_foreach_s();
    test_xpr_path_basename();
    test_xpr_path_last_dirname();
    test_xpr_s2dvec2();
    test_xpr_s2fvec2();
    test_xpr_s2ivec2();
    test_xpr_s2res();
    test_xpr_stridx();
    test_xpr_striidx();
    test_xpr_tolower();
    test_xpr_toupper();
    XPR_SYS_WaitKey(10 * XPR_SYS_CTS_UNIT);
    return 0;
}
