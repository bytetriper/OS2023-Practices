/* YOUR CODE HERE */
#ifndef COROUTINE_H
#define COROUTINE_H
#include <setjmp.h>
#include<pthread.h>
#include "utils.h"
typedef long long cid_t;
#define MAXN 128
#define UNAUTHORIZED -1
#define READY 0
#define WAITING 1
#define FINISHED 3
#define RUNNING 2
#define MAXC 2000
#define STACK_SIZE 4096
struct coroutine
{
    int (*func)(void);  // function of the coroutine
    jmp_buf *buf; // where lives setjump and longjump
    int id;
    struct Node *pos;
    struct LinkedList *WaitList;
    struct coroutine *Previous_RunningCor;
    struct coroutine *father;
    int state; //-1,0,1,2(see #define for details)
    int retNum;
    int *sp;
};
struct Scheduler
{
    pthread_t pid;
    struct coroutine *Cors[MAXC];
    struct coroutine *RunningCors;
    struct LinkedList *AvailableList;
    struct LinkedList *WaitingList;
    struct coroutine *WaitAll_Cor;
    struct LinkedList *DeadList;
    struct coroutine *TmpStorage[2];
    int size,AliveNum;
};
void DEBUG_COROUTINE_INFO();
void coroutine_init(struct coroutine * cor,int (*func)(void));
int co_start(int (*routine)(void));
int co_getid();
int co_getret(int cid);
int co_yield();
int co_waitall();
int co_wait(int cid);
int co_status(int cid);
#endif