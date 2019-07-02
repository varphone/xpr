#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

// Define the data struct
typedef struct Person
{
    // The base must be first
    XPR_ListNodeSL _base;
    // User fields goes here
    int id;
    char name[128];
    char email[128];
} Person;

static void* nodeAlloc(void)
{
    return XPR_Alloc(sizeof(Person));
}

static void nodeFree(void* node)
{
    return XPR_Free(node);
}

static int nodeCompare(void* a, void* b)
{
    Person* pa = (Person*)(a);
    Person* pb = (Person*)(b);
    if (pa->id < pb->id)
        return -1;
    else if (pa->id > pb->id)
        return 1;
    return 0;
}

static int printPerson(void* opaque, void* node)
{
    Person* p = (Person*)(node);
    printf("Person(%04d,\"%s\",\"%s\")\n", p->id, p->name, p->email);
    return 0;
}

static void example(void)
{
    // Create an singly linked list
    XPR_List* list = XPR_ListCreate(XPR_LIST_SINGLY_LINKED, nodeAlloc,
                                    nodeFree, nodeCompare);
    // Add some elements to the list
    for (int i = 0; i < 100; i++) {
        Person* p = nodeAlloc();
        p->id = rand() % 1000;
        sprintf(p->name, "Person%d", p->id);
        sprintf(p->email, "person%d@example.com", p->id);
        XPR_ListAppend(list, p);
    }
    printf("Default orders:\n");
    XPR_ListForEach(list, printPerson, NULL);
    // Reverse the order
    printf("Reversed orders:\n");
    if (XPR_ListReverse(list, NULL) == XPR_ERR_OK) {
        XPR_ListForEach(list, printPerson, NULL);
    }
    // Sort with default order
    printf("Reverted orders:\n");
    if (XPR_ListSort(list, NULL) == XPR_ERR_OK) {
        XPR_ListForEach(list, printPerson, NULL);
    }
    // Remove some elements.
    XPR_ListRemoveAt(list, 3);
    XPR_ListRemoveAt(list, 2);
    XPR_ListRemoveAt(list, 1);
    XPR_ListRemoveAt(list, 0);
    XPR_ListRemoveAt(list, -3);
    XPR_ListRemoveAt(list, -2);
    XPR_ListRemoveAt(list, -1);
    printf("Some elements removed:\n");
    XPR_ListForEach(list, printPerson, NULL);
    printf("First and last elements:\n");
    printPerson(NULL, XPR_ListFirst(list));
    printPerson(NULL, XPR_ListLast(list));
    // Destroy the list
    XPR_ListDestroy(list);
}

void benchmark(void)
{
    srand(time(NULL));
    example();
}