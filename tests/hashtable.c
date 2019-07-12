#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_hashtable.h>

struct TestItem {
    char* k;
    char* v;
};

static struct TestItem kTestItems[] = {
    {"a", "value a"},
    {"aa", "value aa"},
    {"aaa", "value aaa"},
    {"aaaa", "value aaaa"},
    {"aaaaa", "value aaaaa"},
    {"aaaaaa", "value aaaaaa"},
    {"aaaaaaa", "value aaaaaaa"},
    {"aaaaaaaa", "value aaaaaaaa"},
    {"aaaaaaaaa", "value aaaaaaaaa"},
    {"aaaaaaaaaa", "value aaaaaaaaaa"},
    {"aaaaaaaaaaa", "value aaaaaaaaaaa"},
    {"aaaaaaaaaaaa", "value aaaaaaaaaaaa"},
};
static int kNumTestItems = _countof(kTestItems);

static void test_XPR_HashTable()
{
    printf("### %s\n", __FUNCTION__);
    XPR_HashTable* t = XPR_HashTableCreate(16, NULL);

    for (int i = 0; i < kNumTestItems; i++)
        XPR_HashTableSet(t, kTestItems[i].k, kTestItems[i].v);

    XPR_HashTableErase(t, kTestItems[2].k);
    XPR_HashTableErase(t, kTestItems[5].k);
    XPR_HashTableErase(t, kTestItems[7].k);

    for (int i = 0; i < kNumTestItems; i++)
        printf("'%s' = '%s'\n", kTestItems[i].k,
               (char*)XPR_HashTableGet(t, kTestItems[i].k));

    XPR_HashTableDestroy(t);
}

static void gen_random(char* s, const int len)
{
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
}

static void test_XPR_HashTableRandom()
{
    printf("### %s\n", __FUNCTION__);
    int count = 0;
    XPR_HashTable* t = XPR_HashTableCreate(1024, NULL);
    while (1) {
        char key[64];
        char val[256];
        gen_random(key, rand() % 62 + 1);
        gen_random(val, rand() % 254);
        int err = XPR_HashTableSet(t, key, val);
        if (err != XPR_ERR_OK)
            break;
        printf("[%04d] Set '%s' = '%s'\n", count, key, val);
        count++;
    }
    printf("Number of Set = %d\n", count);
    printf("HashTable Size = %d\n", XPR_HashTableSize(t));
    // Iteration
    void* iter = XPR_HashTableIterFirst(t);
    while (iter) {
        printf("[%p] '%s' = '%s'\n", iter, (char*)XPR_HashTableIterKey(iter),
               (char*)XPR_HashTableIterValue(iter));
        iter = XPR_HashTableIterNext(t, iter);
    }
    XPR_HashTableDestroy(t);
}

int main(int argc, char** argv)
{
    srand(0);
    test_XPR_HashTable();
    test_XPR_HashTableRandom();
    return 0;
}
