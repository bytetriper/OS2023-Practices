/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  Blocks are never coalesced or reused.  The size of
 * a block is found at the first aligned word before the block (we need
 * it for realloc).
 *
 * This code is correct and blazingly fast, but very bad usage-wise since
 * it never frees anything.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define SIZE_PTR(p) ((size_t *)(((char *)(p)) - SIZE_T_SIZE))
#define MAX_SIZE 32
#define INIT_SIZE 4096
#define NEXT(p) (p + 1)
#define PREV(p) (p + 2)
#define GET_START(p) ((unsigned int *)(p)-MAX_SIZE)
#define HEADER_LEN 3
#define SKIP_HEADER(p) (p + HEADER_LEN)
// the first bit of the header-byte is used to indicate whether the block is free or not
#define GET_STATE(p) (p & 1)
// the other 31 bits are used to store the size of the block(which is at least 1-byte aligned)
#define GET_POS(p) (p & 0xfffffffe)
// see https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c/994709#994709 for more details
#define HACK_GET_HIGH(p) asm volatile ( "\tbsr %1, %0\n"\
      : "=r"(y)\
      : "r" (x)\
  );
#define GET_HIGH(p) (1+(int)(log2(p)))

#define USE 1
#define FREE 0
/*mem_sbrk() is O(1) here,incredibly fast*/
// use a segregated free list
/*
 * mm_init - Called when a new trace starts.
 */
struct info_set
{
  unsigned int *start;
  unsigned int *end;
  char *free_list[MAX_SIZE];
  unsigned int size;
};
struct info_set info;
void init_info_set(struct info_set *info)
{
  info->start = NULL;
  info->end = NULL;
  info->size = INIT_SIZE;
  for (int i = 0; i < MAX_SIZE; i++)
  {
    info->free_list[i] = NULL;
  }
}
void alloc_block(int size,int state,int *p)
{
  *p = size | state;
  * NEXT(p) = * PREV(p) = NULL;
}
void alloc_block_compl(int size,int state,int *p,int *prev,int *next)
{
  *p = size | state;
  * NEXT(p) = next;
  * PREV(p) = prev;
}
//unchecked
inline void list_push(int idx,int *p)
{
  *NEXT(p) = info.free_list[idx];
  *PREV(p) = NULL;
  if (info.free_list[idx] != NULL)
  {
    *PREV(info.free_list[idx]) = p;
  }
  info.free_list[idx] = p;
}
//unchecked
inline void split_block(int size,int *p)
{
  list_pop(GET_HIGH(GET_POS(*p)),p);
  int *new = SKIP_HEADER(p) + size;
  alloc_block(size, USE, p);
  list_push(GET_HIGH(GET_POS(*p) - size - HEADER_LEN), new);
}
int mm_init(void) // init a segregated free list with up to 32 1-byte block(memory up to 2^32)
{
  init_info_set(&info);
  //1 for header,1 for next, 1 for prev
  info.start = mem_sbrk(info.size + 3 + MAX_SIZE << 2) + MAX_SIZE;
  info.end = info.start + info.size;
  for (int i = 0; i < MAX_SIZE; i++)
  {
    info.free_list[i] = GET_START(info.start) + i;
    *(info.free_list[i]) = NULL;
  }
  *(info.free_list[GET_HIGH(info.size)]) = info.start;
  alloc_block_compl(info.size, FREE, info.start, info.free_list[GET_HIGH(info.size)],NULL);
  return 0;
}
//unchecked
inline void list_pop(int idx,int *p)
{
  int *next = *NEXT(p);
  int *prev = *PREV(p);
  if (next != NULL)
  {
    *PREV(next) = prev;
  }
  if (prev != NULL)
  {
    *NEXT(prev) = next;
  }
  else
  {
    info.free_list[idx] = next;
  }
}
/*
 * malloc - Allocate a block by incrementing the brk pointer.
 *      Always allocate a block whose size is a multiple of the alignment.
 */
void *malloc(size_t size)
{
  int idx=GET_HIGH(size);
  int *p=info.free_list[idx];
  while(p==NULL && idx<MAX_SIZE)
  {
    idx++;
    p=info.free_list[idx];
  }
  if (p == MAX_SIZE)
  {
    DEBUG("No enough memory!");
    return NULL;
  }
  int *avail=p;
  list_pop(avail,p);

}

/*
 * free - free a block by setting the state to free and adding it to the corresponding free list
 */
void free(void *ptr)
{
  
}

/*
 * realloc - Change the size of the block by mallocing a new block,
 *      copying its data, and freeing the old block.  I'm too lazy
 *      to do better.
 */
void *realloc(void *oldptr, size_t size)
{
}

/*
 * calloc - Allocate the block and set it to zero.
 */
void *calloc(size_t nmemb, size_t size)
{
}

/*
 * mm_checkheap - There are no bugs in my code, so I don't need to check,
 *      so nah!
 */
void mm_checkheap(int verbose)
{
}
