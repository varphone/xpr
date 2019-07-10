#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Entry {
    const char** names;
    const char** descs;
    const char* category;
    const char* root;
    int type;
    struct Entry* prev;
    struct Entry* next;
    struct Entry* subs;
};

#define ENTRY_END { 0, 0, 0, 0, 0, 0, 0 }

static const char* names[] = {"n1", "n2", "n3", 0};
static const char* descs[] = {"d1", "d2", "d3", 0};
static struct Entry ents[] = {
    {
        names,
        descs,
        "ups/builtin", "/sys/usr", 0,
        0, 0, 0,
    },
    ENTRY_END
};

static void dumpEntry(struct Entry* ent)
{
    int i = 0;
    const char* n = 0;
    const char* d = 0;
    printf("[category: %s, root: %s, type: %d]\n{\n", ent->category, ent->root, ent->type);
    while (ent->names) {
        n = ents->names[i];
        d = ents->descs[i];
        if (!n)
            break;
        printf("    %s (%s)\n", n, d);
        i++;
    }
    printf("}\n");
}

static void reg(struct Entry ents[], int count)
{
    int i = 0;
    for (; i<count; i++) {
        dumpEntry(&ents[i]);
/*
        printf("ents[%d] = %p, %d\n", i, &ents[i], count);

*/
    }
}

static void runTest(void)
{
    printf("sizeof(struct Entry) = %d\n", (int)sizeof(struct Entry));
    printf("sizeof(ents[]) = %d\n", (int)sizeof(ents));
    printf("ents[0].names = %p (%s, %s, %s, %s)\n",
           ents[0].names, ents[0].names[0], ents[0].names[1],
           ents[0].names[2], ents[0].names[3]);
    reg(&ents[0], 1);
    reg(ents, 2);
}

int main(int argc, char** argv)
{
    runTest();
    return 0;
}

