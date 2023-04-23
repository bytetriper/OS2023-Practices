#ifndef UTILS_H
#define UTILS_H
#define ok(expr)                                                         \
    do {                                                                 \
        if (!((expr) || fake_mode)) {                                    \
            printf("[%d]Assertion failed: " #expr " at line %d\n", tCnt, \
                   __LINE__);                                            \
            if (!cont) exit(-1);                                         \
        } else {                                                         \
            printf("[%d]Ok: " #expr " at line %d\n", tCnt, __LINE__);    \
            tCnt++;                                                      \
        }                                                                \
    } while (0)

#define dotOk(expr)                                                      \
    do {                                                                 \
        if (!((expr) || fake_mode)) {                                    \
            printf("X==>FAILED!\n");                                     \
            printf("[%d]Assertion failed: " #expr " at line %d\n", tCnt, \
                   __LINE__);                                            \
            if (!cont) exit(-1);                                         \
        } else {                                                         \
            printf(".");                                                 \
            fflush(stdout);                                              \
            tCnt++;                                                      \
        }                                                                \
    } while (0)

#define dotDone()                         \
    do {                                  \
        printf("\nTotal: %d Ok\n", tCnt); \
    } while (0)

#define finish()                  \
    do {                          \
        printf("\nTest Ends.\n"); \
    } while (0)
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include<stdlib.h>
void fail(const char* message, const char* function, int line);
struct Node
{
    struct Node *pre, *nxt;
    void *item;
};
struct LinkedList // both-direction LinkedList
{
    struct Node *head, *tail;
    int size;
};
struct LinkedList * New_LinkedList();
void LinkedList_Init(struct LinkedList *list);
struct Node * LinkedList_push(struct LinkedList *list, struct Node *node);
struct Node * LinkedList_push_item(struct LinkedList *list, void *item);
void *LinkedList_head_item(struct LinkedList *list);
int LinkedList_empty(struct LinkedList *list);
struct Node *LinkedList_head(struct LinkedList *list);
void *LinkedList_popFirst(struct LinkedList *list);
void LinkedList_pop(struct LinkedList *list, struct Node *node);
#endif