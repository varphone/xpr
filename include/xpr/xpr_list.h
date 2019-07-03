/*
 * File: xpr_list.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Double/Singly Linked List
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-02 08:20:17 Tuesday, 2 July
 * Last Modified : 2019-07-02 10:43:04 Tuesday, 2 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2010 - 2019 Varphone Wong, Varphone.com
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 */
#ifndef XPR_LIST_H
#define XPR_LIST_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_LIST_TYPE_TYPE_DEFINED
#define XPR_LIST_TYPE_TYPE_DEFINED
typedef enum XPR_ListType {
    XPR_LIST_INVALID,
    XPR_LIST_DOUBLE_LINKED,
    XPR_LIST_SINGLY_LINKED,
} XPR_ListType;
#endif // XPR_LIST_TYPE_TYPE_DEFINED

#ifndef XPR_LIST_NODE_TYPE_DEFINED
#define XPR_LIST_NODE_TYPE_DEFINED
struct XPR_ListNode {
    struct XPR_ListNode* next;
};
typedef struct XPR_ListNode XPR_ListNode;
#endif // XPR_LIST_NODE_TYPE_DEFINED

#ifndef XPR_LIST_NODE_DL_TYPE_DEFINED
#define XPR_LIST_NODE_DL_TYPE_DEFINED
struct XPR_ListNodeDL {
    struct XPR_ListNodeDL* next;
    struct XPR_ListNodeDL* prev;
};
typedef struct XPR_ListNodeDL XPR_ListNodeDL;
#endif // XPR_LIST_NODE_DL_TYPE_DEFINED

#ifndef XPR_LIST_NODE_SL_TYPE_DEFINED
#define XPR_LIST_NODE_SL_TYPE_DEFINED
typedef XPR_ListNode XPR_ListNodeSL;
#endif // XPR_LIST_NODE_SL_TYPE_DEFINED

#ifndef XPR_LIST_TYPE_DEFINED
#define XPR_LIST_TYPE_DEFINED
struct XPR_List;
typedef struct XPR_List XPR_List;
#endif // XPR_LIST_TYPE_DEFINED

/// Function to allocate an element used in the list.
typedef void* (*XPR_ListNodeAlloc)(void);

/// Function to free an element allocated by #XPR_ListNodeAlloc.
typedef void (*XPR_ListNodeFree)(void*);

/// Function to compare two elements.
/// \return -1              a  < b
/// \return  0              a == b
/// \return  1              a  > b
typedef int (*XPR_ListNodeCompare)(void* a, void* b);

/// Function to process each of the elements in the list.
/// \param [in] opaque      User context data.
/// \param [in] node        The iterated node.
/// \retval 0               Continue the iteration.
/// \retval -1              Break the iteration.
typedef int (*XPR_ListForEachFn)(void* opaque, void* node);

/// Create a new list.
/// \param [in] type        The type of elements to used.
/// \param [in] alloc       The function to allocate an element.
/// \param [in] free        The function to free an element.
/// \param [in] compare     The function to compare two elements.
/// \retval !NULL           The created list pointer.
/// \retval NULL            Failed to create.
XPR_API XPR_List* XPR_ListCreate(XPR_ListType type, XPR_ListNodeAlloc alloc,
                                 XPR_ListNodeFree free,
                                 XPR_ListNodeCompare compare);

/// Destroy a created list and free all elements in the list.
/// \param [in] list        The list to be destroyed.
/// \return
XPR_API void XPR_ListDestroy(XPR_List* list);

/// Append pre-allocated element to list, the owner transfer to the list.
/// \param [in] list        The list to add.
/// \param [in] node        The pre-allocated element.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListAppend(XPR_List* list, void* node);

/// No-locking #XPR_ListAppend
XPR_API int XPR_ListAppendNl(XPR_List* list, void* node);

/// Allocate a new element then append to the list and return the new element.
/// \param [in] list        The list to add.
/// \retval !NULL           The new element added.
/// \retval NULL            Allocate element error or add failed.
XPR_API void* XPR_ListAppendNew(XPR_List* list);

/// No-locking #XPR_ListAppendNew
XPR_API void* XPR_ListAppendNewNl(XPR_List* list);

/// Returns the element at position n in the list.
XPR_API void* XPR_ListAt(XPR_List* list, int pos);

/// No-locking #XPR_ListAt
XPR_API void* XPR_ListAtNl(XPR_List* list, int pos);

/// Remove all elements from the list and free the memory.
/// \param [in] list        The list to clear.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListClear(XPR_List* list);

/// No-locking #XPR_ListClear
XPR_API int XPR_ListClearNl(XPR_List* list);

/// Returns the first element in the list.
/// \param [in] list        The list to access.
/// \retval !NULL           The first element.
/// \retval NULL            The list is empty.
XPR_API void* XPR_ListFirst(XPR_List* list);

/// No-locking #XPR_ListFirst
XPR_API void* XPR_ListFirstNl(XPR_List* list);

/// Applies function fn to each of the elements in the list.
/// \param [in] list        The list to iterate.
/// \param [in] fn          Function to apply.
/// \param [in] opaque      User context data.
/// \return
XPR_API void XPR_ListForEach(XPR_List* list, XPR_ListForEachFn fn,
                             void* opaque);

/// No-locking #XPR_ListForEach
XPR_API void XPR_ListForEachNl(XPR_List* list, XPR_ListForEachFn fn,
                               void* opaque);

/// Returns the last element in the list.
/// \param [in] list        The list to access.
/// \retval !NULL           The last element.
/// \retval NULL            The list is empty.
XPR_API void* XPR_ListLast(XPR_List* list);

/// No-locking #XPR_ListLast
XPR_API void* XPR_ListLastNl(XPR_List* list);

/// Returns the element next to curr in the list.
/// \param [in] list        The list to access.
/// \retval !NULL           The next element.
/// \retval NULL            The curr is the last element.
XPR_API void* XPR_ListNext(XPR_List* list, void* curr);

/// No-locking #XPR_ListNext
XPR_API void* XPR_ListNextNl(XPR_List* list, void* curr);

/// Remove the element from the list and free the memory.
/// \param [in] list        The list to add.
/// \param [in] node        The element to remove and free.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListRemove(XPR_List* list, void* node);

/// No-locking #XPR_ListRemove
XPR_API int XPR_ListRemoveNl(XPR_List* list, void* node);

/// Remove the element at position from the list and free the memory.
/// \param [in] list        The list to add.
/// \param [in] pos         The position of the element to remove and free.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListRemoveAt(XPR_List* list, int pos);

/// No-locking #XPR_ListRemoveAt
XPR_API int XPR_ListRemoveAtNl(XPR_List* list, int pos);

/// Reverses the order of the elements in the list.
/// \param [in] list        The list to reverse.
/// \param [in] compare     The function to compare two elements,
//                          Use the value passed to XPR_ListCreate() if NULL.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListReverse(XPR_List* list, XPR_ListNodeCompare compare);

/// No-locking #XPR_ListReverse
XPR_API int XPR_ListReverseNl(XPR_List* list, XPR_ListNodeCompare compare);

/// Sort the order of the elements in the list.
/// \param [in] list        The list to sort.
/// \param [in] compare     The function to compare two elements,
//                          Use the value passed to XPR_ListCreate() if NULL.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListSort(XPR_List* list, XPR_ListNodeCompare compare);

/// No-locking #XPR_ListSortNl
XPR_API int XPR_ListSortNl(XPR_List* list, XPR_ListNodeCompare compare);

/// Take the element out from the list and not free the memory.
/// \param [in] list        The list to access.
/// \param [in] node        The element to taked.
/// \retval XPR_ERR_OK      Success.
/// \retval XPR_ERR_ERROR   Undefined error.
XPR_API int XPR_ListTake(XPR_List* list, void* node);

/// No-locking #XPR_ListTake
XPR_API int XPR_ListTakeNl(XPR_List* list, void* node);

/*! \example
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>

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
    printf("Person(%d,\"%s\",\"%s\")\n", p->id, p->name, p->email);
    return 0;
}

static void example(void)
{
    // Create an singly linked list
    XPR_List* list = XPR_ListCreate(XPR_LIST_SINGLY_LINKED, nodeAlloc,
                                    nodeFree, nodeCompare);
    // Add some elements to the list
    for (int i = 0; i < 100; i++) {
        Person* p = XPR_ListAppendNew(list);
        p->id = i;
        sprintf(p->name, "Person%d", i);
        sprintf(p->email, "person%d@example.com", i);
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
!*/

#ifdef __cplusplus
}
#endif

#endif // XPR_LIST_H
