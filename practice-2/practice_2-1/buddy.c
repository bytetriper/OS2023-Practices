#include "buddy.h"
#include <math.h>
#define NULL ((void *)0)
//#define DEBUG
struct buddy_node *map[MAX_PAGE_NUMBER];
struct buddy_sys *buddy = NULL;
void init_buddy_node(struct buddy_node *node, int rank, void *p)
{
    node->rank = rank;
    node->p = p;
    node->status = FREE;
    node->buddy = NULL;
    node->fa = NULL;
}
struct buddy_node *get_buddy_node(int rank, void *p)
{
    struct buddy_node *node = (struct buddy_node *)malloc(sizeof(struct buddy_node));
    init_buddy_node(node, rank, p);
    return node;
}
void DEBUG_NODE_INFO(struct buddy_node *node)
{
    printf("node:%p rank:%d, p:%p, status:%s, buddy_p:%p, fa:%p\n", node, node->rank, node->p, node->status == FREE ? "FREE" : "BUSY", node->buddy->p, node->fa);
}
struct buddy_node *split_node(struct buddy_node *node)
{
    if (node->rank == 0)
        fail("split_node: rank = 0", "split_node", __LINE__);
    struct buddy_node *left = get_buddy_node(node->rank - 1, node->p);
    struct buddy_node *right = get_buddy_node(node->rank - 1, node->p + (PAGE_SIZE << (node->rank - 1)));
    node->status = OCCUPIED; // splited node is occupied
    left->buddy = right;
    right->buddy = left;
    left->fa = node;
    right->fa = node;
    LinkedList_pop(buddy->page_list[node->rank], node->pos);
    map[(node->p - buddy->base_p) / PAGE_SIZE] = NULL;
    left->pos = LinkedList_push_item(buddy->page_list[node->rank - 1], left);
    right->pos = LinkedList_push_item(buddy->page_list[node->rank - 1], right);
    map[(left->p - buddy->base_p) / PAGE_SIZE] = left;
    map[(right->p - buddy->base_p) / PAGE_SIZE] = right;
    return left;
}
void try_merge_node(struct buddy_node *node)
{
    //printf("try merge node rank:%d\n", node->rank);
    //DEBUG_NODE_INFO(node);
    //DEBUG_NODE_INFO(node->buddy);
    node->status = FREE;
    node->pos = LinkedList_push_item(buddy->page_list[node->rank], node);
    while (node->buddy != NULL && node->buddy->status == FREE)
    {
        //printf("recursively merge node rank:%d\n", node->rank);
        //DEBUG_NODE_INFO(node);
        //DEBUG_NODE_INFO(node->buddy);
        LinkedList_pop(buddy->page_list[node->rank], node->pos);
        LinkedList_pop(buddy->page_list[node->rank], node->buddy->pos);
        node->pos = node->buddy->pos = NULL;
        node->fa->pos = LinkedList_push_item(buddy->page_list[node->rank + 1], node->fa);
        struct buddy_node *next_node = node->fa;
        map[(node->p - buddy->base_p) / PAGE_SIZE] = NULL;
        map[(node->buddy->p - buddy->base_p) / PAGE_SIZE] = NULL;
        free(node->buddy);
        free(node);

        node = next_node;
        node->status = FREE;
        map[(node->p - buddy->base_p) / PAGE_SIZE] = node;
    }
}
void init_buddy_sys(struct buddy_sys *buddy, int n)
{
#ifdef OUTPUT
    printf("initing buddy system...\n");
#endif
    buddy->maxn = ceil(log2(n));
    buddy->page_num = n;
#ifdef OUTPUT
    printf("rank=%d maxrank = %d\n", n, buddy->maxn);
#endif
    for (int i = 0; i <= buddy->maxn; i++)
    {
        buddy->page_list[i] = (struct LinkedList *)malloc(sizeof(struct LinkedList));
        LinkedList_Init(buddy->page_list[i]);
    }
    for (int i = 0; i < n; i++)
    {
        map[i] = NULL;
    }
}
void free_buddy_sys(struct buddy_sys *buddy)
{
    for (int i = 0; i < buddy->maxn; i++)
    {
        while (!LinkedList_empty(buddy->page_list[i]))
        {
            LinkedList_popFirst(buddy->page_list[i]);
        }
        free(buddy->page_list[i]);
    }
    free(buddy);
}
int init_page(void *p, int pgcount)
{
    buddy = (struct buddy_sys *)malloc(sizeof(struct buddy_sys));
    buddy->base_p = p;
    init_buddy_sys(buddy, pgcount);

    struct buddy_node *node = get_buddy_node(buddy->maxn, p);
    node->pos = LinkedList_push_item(buddy->page_list[buddy->maxn], node);
    return OK;
}

void *alloc_pages(int rank) // max operation time is O(logn)
{
    --rank;
    if (rank > buddy->maxn || rank < 0)
    {
#ifdef DEBUG
        fail("rank out of range", "alloc_pages", __LINE__);
#endif
        return (void *)-EINVAL;
    }
    if (LinkedList_empty(buddy->page_list[rank]))
    {
        int min_avail_rank = rank;
        while (min_avail_rank <= buddy->maxn && LinkedList_empty(buddy->page_list[min_avail_rank]))
        {
            min_avail_rank++;
        }
        if (min_avail_rank > buddy->maxn)
        {
#ifdef DEBUG
            fail("no available memory", "alloc_pages", __LINE__);
#endif

            return (void *)-ENOSPC;
        }
        struct buddy_node *node = (struct buddy_node *)LinkedList_head_item(buddy->page_list[min_avail_rank]);
        while (node->rank > rank)
        {
            node = split_node(node);
        }
        LinkedList_pop(buddy->page_list[node->rank], node->pos);
        node->status = OCCUPIED;
        if (node->p == NULL)
        {
#ifdef DEBUG
            fail("node->p is NULL", "alloc_pages", __LINE__);
#endif
        }
        return node->p;
    }
    else
    {
        struct buddy_node *node = (struct buddy_node *)LinkedList_head_item(buddy->page_list[rank]);
        LinkedList_pop(buddy->page_list[node->rank], node->pos);
        node->status = OCCUPIED;
        if (node->p == NULL)
        {
#ifdef DEBUG
            fail("node->p is NULL", "alloc_pages", __LINE__);
#endif
        }
        return node->p;
    }
    fail("should not reach here", "alloc_pages", __LINE__);
}

int return_pages(void *p)
{
    if (p < buddy->base_p || p > buddy->base_p + (buddy->page_num-1) * PAGE_SIZE)
    {
#ifdef DEBUG
        fail("p out of range", "return_pages", __LINE__);
#endif
        return -EINVAL;
    }
    if ((p-buddy->base_p)%PAGE_SIZE!=0)
    {
#ifdef DEBUG
        fail("p not aligned", "return_pages", __LINE__);
#endif
        return -EINVAL;
    }
    struct buddy_node *node = map[(p - buddy->base_p) / PAGE_SIZE];
    if (node == NULL)
    {    
#ifdef DEBUG
        fail("p not allocated", "return_pages", __LINE__);
#endif
        return -EINVAL;
    }
    if (node->status == FREE)
    {
#ifdef DEBUG
        fail("returning a free page", "return_pages", __LINE__);
#endif
        return -EINVAL;
    }
    node->status = FREE;
    //DEBUG_NODE_INFO(node);
    try_merge_node(node);
    return OK;
}

int query_ranks(void *p)
{
    if (p < buddy->base_p || p >= buddy->base_p + buddy->page_num * PAGE_SIZE)
        return -EINVAL;
    struct buddy_node *node = map[(p - buddy->base_p) / PAGE_SIZE];
    if (node == NULL)
        return -EINVAL;
    return node->rank + 1;
}

int query_page_counts(int rank) // assume only counting free pages on the rank
{
    --rank;
    if (rank > buddy->maxn)
        return -EINVAL;
    return buddy->page_list[rank]->size;
}
