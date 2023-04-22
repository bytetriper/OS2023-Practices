#ifndef UTILS_H
#define UTILS_H
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
void LinkedList_popFirst(struct LinkedList *list);
void LinkedList_pop(struct LinkedList *list, struct Node *node);
#endif