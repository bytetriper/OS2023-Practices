#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include "utils.h"
inline void LinkedList_Init(struct LinkedList *list)
{
    list->size = 0;
    list->head = (struct Node *)malloc(sizeof(struct Node));
    list->tail = (struct Node *)malloc(sizeof(struct Node));
    list->tail->pre = list->head;
    list->head->nxt = list->tail;
}
inline struct Node *LinkedList_push(struct LinkedList *list, struct Node *node)
{
    ++list->size;
    struct Node *t = list->tail->pre; // ASSERT the list to be not empty
    t->nxt = node;
    node->pre = t;
    node->nxt = list->tail;
    list->tail->pre = node;
    return node;
}
inline struct Node *LinkedList_push_item(struct LinkedList *list, void *item)
{
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->item = item;
    ++list->size;
    struct Node *t = list->tail->pre; // ASSERT the list to be not empty
    t->nxt = node;
    node->pre = t;
    node->nxt = list->tail;
    list->tail->pre = node;
    return node;
}
inline void *LinkedList_head_item(struct LinkedList *list)
{
    if (list->size == 0) // empty
    {
        return NULL;
    }
    return list->head->nxt->item;
}
inline int LinkedList_empty(struct LinkedList *list)
{
    return list->size == 0 ? 1 : 0;
}
inline struct Node *LinkedList_head(struct LinkedList *list)
{
    if (list->size == 0) // empty
    {
        return NULL;
    }
    return list->head->nxt;
}
inline void LinkedList_popFirst(struct LinkedList *list)
{
    if (list->size == 0) // empty
    {
        printf("LinkedList PopFirst Error");
        assert(1 == 0); // throw error
    }
    list->head->nxt = list->head->nxt->nxt;
    list->head->nxt->pre = list->head;
    --list->size;
    return;
}
inline void LinkedList_pop(struct LinkedList *list, struct Node *node)
{
    if (node->pre != NULL)
    {
        node->pre->nxt = node->nxt;
    }
    if (node->nxt != NULL)
    {
        node->nxt->pre = node->pre;
    }
    --list->size;
}
inline struct LinkedList *New_LinkedList()
{
    struct LinkedList *list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    LinkedList_Init(list);
    return list;
}
inline void fail(const char *message, const char *function, int line)
{
    printf("[x] Test failed at %s: %d: %s\n", function, line, message);
    exit(-1);
}