#ifndef OS_MM_H
#define OS_MM_H
#define MAX_ERRNO 4095
#define PAGE_SIZE 4096
#define FREE 0
#define OCCUPIED 1
#define OK          0
#define EINVAL      22  /* Invalid argument */    
#define ENOSPC      28  /* No page left */  
#include "utils.h"
#define MAX_PAGE_NUMBER 128*1024
#define IS_ERR_VALUE(x) ((x) >= (unsigned long)-MAX_ERRNO)
static inline void *ERR_PTR(long error) { return (void *)error; }
static inline long PTR_ERR(const void *ptr) { return (long)ptr; }
static inline long IS_ERR(const void *ptr) { return IS_ERR_VALUE((unsigned long)ptr); }

struct buddy_sys
{
    struct LinkedList *page_list[MAX_PAGE_NUMBER];
    void * base_p;
    //maxn=ceil(log2(maxsize))
    int maxn,page_num;

};
struct buddy_node
{
    int rank;
    struct buddy_node *buddy;
    struct buddy_node *fa;
    struct Node *pos;
    int status;
    void *p;
};
int init_page(void *p, int pgcount);
void *alloc_pages(int rank);
int return_pages(void *p);
int query_ranks(void *p);
int query_page_counts(int rank);

#endif