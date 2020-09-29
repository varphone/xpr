#include <stdint.h>
#include <stdio.h>
#include <xpr/xpr_sha1.h>
#include <xpr/xpr_utils.h>

struct TestItem
{
    const char* text;
    const char* expected;
};

int main(int argc, char** argv)
{
    static const struct TestItem testItems[2] = {
        {"abc", "A9993E364706816ABA3E25717850C26C9CD0D89D"},
        {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
         "84983E441C3BD26EBAAE4AA1F95129E5E54670F1"}};
    char hashOut[XPR_SHA1_HASH_BUF];
    for (int i = 0; i < 2; i++) {
        XPR_SHA1Data(testItems[i].text, strlen(testItems[i].text), hashOut);
        printf("SHA1(\"%s\")=\"%s\"\n", testItems[i].text, hashOut);
        ASSERT(strcmp(hashOut, testItems[i].expected) == 0);
    }
    return 0;
}
