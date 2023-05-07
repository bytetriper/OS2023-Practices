/* YOUR CODE HERE */
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "utils.h"
#include "coroutine.h"
#define INITED 0
#define UNINITED 1
//#define DEBUG
//  should be converted into TLS(Thread Local Storage) later
static __thread struct Scheduler *Manager = NULL;

void init_Manager()
{
    Manager = (struct Scheduler *)malloc(sizeof(struct Scheduler));
    Manager->size = 0;
    Manager->AliveNum = 0;
    Manager->AvailableList = New_LinkedList();
    Manager->WaitingList = New_LinkedList();
    Manager->DeadList = New_LinkedList();
    Manager->WaitAll_Cor = NULL;
    Manager->RunningCors = NULL;
    for (int i = 0; i < MAXC; i++)
    {
        Manager->Cors[i] = NULL;
    }
    struct coroutine *Main = (struct coroutine *)malloc(sizeof(struct coroutine));
    coroutine_init(Main, NULL);
    Main->id = 0;
    Main->state = RUNNING;
    Manager->RunningCors = Main;
    Manager->Cors[(Manager->size)++] = Main;
    Manager->AliveNum++;
}
void coroutine_init(struct coroutine *cor, int (*func)(void))
{
    cor->func = func;
    cor->buf = (jmp_buf *)malloc(sizeof(jmp_buf));
    cor->WaitList = New_LinkedList();
    cor->Previous_RunningCor = NULL;
    cor->pos = NULL;
    cor->state = READY;
    cor->retNum = -1;
    cor->father = NULL;
}
void check_init()
{
    if (Manager == NULL)
    {
#ifdef DEBUG
        printf("[Init]Manager is not initialized.\n");
#endif
        init_Manager();
    }
}
int check_is_father(struct coroutine *cor, struct coroutine *father)
{
    while (cor->father != NULL)
    {
        if (cor->father == father)
        {
            return 1;
        }
        cor = cor->father;
    }
    return 0;
}
int Clean_Up(int retnum)
{
    struct coroutine *cor = Manager->RunningCors;
    cor->retNum = retnum;
    cor->state = FINISHED;
    /*clean up the mess*/
#ifdef DEBUG
    printf("Coroutine %d finished.Cleaning the mess.\n", cor->id);
#endif
    //free(cor->buf);
    //free(cor->sp);
    // free(cor->pos);
    cor->pos = LinkedList_push_item(Manager->DeadList, cor);
    Manager->AliveNum--;
    // let all the coroutines waiting for this one to be finished to be ready
    while (!LinkedList_empty(cor->WaitList))
    {
        struct Node *node = LinkedList_head(cor->WaitList);
        LinkedList_popFirst(cor->WaitList);
        struct coroutine *cor2 = (struct coroutine *)(node->item);
        // use pos to delete the node in the waiting list
        LinkedList_pop(Manager->WaitingList, cor2->pos);
        cor2->pos = LinkedList_push(Manager->AvailableList, cor2->pos);
        cor2->state = READY;
    }
    // if there is a coroutine waiting for all the coroutines to be finished, and all the coroutines are finished, then let it be ready
    if (Manager->WaitAll_Cor != NULL && Manager->AliveNum == 1)
    {
        Manager->WaitAll_Cor->pos = LinkedList_push_item(Manager->AvailableList, Manager->WaitAll_Cor);
        Manager->WaitAll_Cor->state = READY;
        Manager->WaitAll_Cor = NULL;
    }
    // select a new cor to run
    if (LinkedList_empty(Manager->AvailableList))
    {
        fail("no available coroutines", "co_start", __LINE__);
        return -1;
    }
    else if (cor->Previous_RunningCor->state == READY)
    {
        Manager->RunningCors = cor->Previous_RunningCor;
        LinkedList_pop(Manager->AvailableList, Manager->RunningCors->pos);
        //free(cor->Previous_RunningCor->pos);
        Manager->RunningCors->state = RUNNING;
        longjmp(*(Manager->RunningCors->buf), 1);
    }
    else
    {
        struct Node *node = LinkedList_head(Manager->AvailableList);
        if (node == NULL)
        {
            if (Manager->AliveNum != 0)
            {
                fail("no available coroutines while some coroutine isn't end", "co_start", __LINE__);
                return -1;
            }
#ifdef DEBUG
            printf("all task finished");
#endif
            return cor->id - 1;
        }
        LinkedList_popFirst(Manager->AvailableList);
        struct coroutine *cor = (struct coroutine *)(node->item);
        //free(node); // free the node==free the pos
        Manager->RunningCors = cor;
        Manager->RunningCors->state = RUNNING;
        longjmp(*(Manager->RunningCors->buf), 1);
    }
    return cor->id - 1;
}
int co_start(int (*routine)(void))
{
    check_init();
    if (Manager->size >= MAXC)
    {
        printf("Too many coroutines.\n");
        exit(-1);
        return -1;
    }
    struct coroutine *NewCor = (struct coroutine *)malloc(sizeof(struct coroutine));
    coroutine_init(NewCor, routine);
    NewCor->father = Manager->RunningCors;
    NewCor->id = Manager->size;
    Manager->Cors[(Manager->size)++] = NewCor;
    Manager->AliveNum++;
#ifdef DEBUG
    printf("Coroutine %d created.\n", NewCor->id);
#endif
    if (setjmp(*(Manager->RunningCors->buf)) == 0)
    {
        Manager->RunningCors->state = READY;
        NewCor->Previous_RunningCor = Manager->RunningCors;
        Manager->RunningCors->pos = LinkedList_push_item(Manager->AvailableList, Manager->RunningCors);
        Manager->RunningCors = NewCor;
        Manager->RunningCors->state = RUNNING;
        NewCor->sp = malloc(STACK_SIZE << 2);
        int *sp_top = (int *)(NewCor->sp + STACK_SIZE) ;
        asm volatile("movq %0, %%rsp; call *%1;movl %%eax, %%edi;call Clean_Up;"
                    :   
                     : "r"(sp_top),
                       "r"(routine));
        //asm volatile("ret;");
        // Manager->RunningCors->retNum = routine();
    }
    else
        return NewCor->id - 1;
}
int co_getid()
{
    check_init();
    if (Manager->RunningCors == NULL)
    {
        return -1;
    }
    return Manager->RunningCors->id - 1;
}
void DEBUG_COROUTINE_INFO(int cid)
{
    // if cid==-1,then output all the state of coroutines,otherwise output the state of the coroutine with id=cid
    if (cid == -1)
    {
        for (int i = 0; i < Manager->size; i++)
        {
            printf("coroutine %d: ", i);
            if (i == 0)
                printf("(Main)");
            switch (Manager->Cors[i]->state)
            {
            case READY:
                printf("READY");
                break;
            case RUNNING:
                printf("RUNNING");
                break;
            case WAITING:
                printf("WAITING");
                break;
            case FINISHED:
                printf("FINISHED");
                break;
            }
            printf("\n");
            printf("retNum=%d\n", Manager->Cors[i]->retNum);
            printf("WaitList:\n");
            for (struct Node *node = LinkedList_head(Manager->Cors[i]->WaitList); node != NULL && node != Manager->Cors[i]->WaitList->tail; node = node->nxt)
            {
                struct coroutine *cor = (struct coroutine *)(node->item);
                printf("%d ", cor->id);
            }
            printf("\npos=%p", Manager->Cors[i]->pos);
            printf("\nsp=%p", Manager->Cors[i]->sp);
            printf("\nbuf=%p", Manager->Cors[i]->buf);
            printf("\nfunc=%p\n", Manager->Cors[i]->func);
        }
    }
    else
    {
        printf("coroutine %d: ", cid);
        switch (Manager->Cors[cid]->state)
        {
        case READY:
            printf("READY");
            break;
        case RUNNING:
            printf("RUNNING");
            break;
        case WAITING:
            printf("WAITING");
            break;
        case FINISHED:
            printf("FINISHED");
            break;
        }
        printf("  retNum=%d\n", Manager->Cors[cid]->retNum);
        printf(" WaitList:\n");
        for (struct Node *node = LinkedList_head(Manager->Cors[cid]->WaitList); node != NULL && node != Manager->Cors[cid]->WaitList->tail; node = node->nxt)
        {
            struct coroutine *cor = (struct coroutine *)(node->item);
            printf("%d ", cor->id);
        }
        printf("pos=%p\n", Manager->Cors[cid]->pos);
        printf("sp=%p\n", Manager->Cors[cid]->sp);
        printf("buf=%p\n", Manager->Cors[cid]->buf);
        printf("func=%p\n", Manager->Cors[cid]->func);
    }
}
int co_getret(int cid)
{
    check_init();
    ++cid;
    if (cid < 0 || cid >= Manager->size)
    {
        return UNAUTHORIZED;
    }
    if (check_is_father(Manager->Cors[cid], Manager->RunningCors) == 0)
    {
        return UNAUTHORIZED;
    }
    if (Manager->Cors[cid]->state != FINISHED)
    {
#ifdef DEBUG
        printf("coroutine %d: ", cid);
#endif
        // fail("coroutine not finished", "co_getret", __LINE__);
        return UNAUTHORIZED;
    }
    return Manager->Cors[cid]->retNum;
}
int co_yield ()
{
    check_init();
    if (Manager->RunningCors == NULL)
    {
        fail("no coroutine running", "co_yield", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == FINISHED)
    {
        fail("coroutine already finished", "co_yield", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == WAITING)
    {
        fail("coroutine already waiting", "co_yield", __LINE__);
        return -1;
    }
    if (LinkedList_empty(Manager->AvailableList)) // no available coroutines
    {
        return 0;
    }
    if (Manager->RunningCors->state == RUNNING)
    {
        Manager->RunningCors->state = READY;
    }
    struct coroutine *NextCor = LinkedList_head_item(Manager->AvailableList);
    LinkedList_popFirst(Manager->AvailableList);
    Manager->RunningCors->pos = LinkedList_push_item(Manager->AvailableList, Manager->RunningCors);
    if (!setjmp(*(Manager->RunningCors->buf)))
    {
#ifdef DEBUG
        printf("switch from coroutine %d to coroutine %d\n", Manager->RunningCors->id, NextCor->id);
#endif
        Manager->RunningCors = NextCor;
        NextCor->state = RUNNING;
        longjmp(*(NextCor->buf), 1);
    }
    else
    {
        return 0;
    }
    return 0;
}
int co_waitall()
{
    check_init();
    if (Manager->RunningCors == NULL)
    {
        fail("coroutine is waiting", "co_waitall", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == FINISHED)
    {
        fail("coroutine is waiting", "co_waitall", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == WAITING)
    {
        fail("coroutine is waiting", "co_waitall", __LINE__);
        return -1;
    }

    struct coroutine *NextCor = LinkedList_head_item(Manager->AvailableList);
    if (Manager->WaitAll_Cor != NULL)
    {
        fail("Already Have a Waitall coroutine", "co_waitall", __LINE__);
        return -1;
    }
    if (NextCor == NULL)
    {
        if (Manager->AliveNum == 1) // only itself survives
        {
#ifdef DEBUG
            printf("[co_waitall]only itself survives:coroutine %d\n", Manager->RunningCors->id);
#endif
            return 0;
        }
        fail("No Available Coroutine While some are waiting", "co_waitall", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == RUNNING)
    {
        Manager->RunningCors->state = WAITING;
    }
    LinkedList_popFirst(Manager->AvailableList);
    //free(NextCor->pos);
    if (!setjmp(*(Manager->RunningCors->buf)))
    {
#ifdef DEBUG
        printf("[co_waitall]switch from coroutine %d to coroutine %d\n", Manager->RunningCors->id, NextCor->id);
#endif
        Manager->WaitAll_Cor = Manager->RunningCors;
        Manager->RunningCors = NextCor;
        NextCor->state = RUNNING;
        longjmp(*(NextCor->buf), 1);
    }
    else
    {
        return 0;
    }
    return 0;
}
int co_wait(int cid)
{
    check_init();
    ++cid;
    if (Manager->RunningCors == NULL)
    {
        fail("coroutine is NULL", "co_wait", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == FINISHED)
    {
        fail("coroutine is FINISHED", "co_wait", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == WAITING)
    {
        fail("coroutine is waiting", "co_wait", __LINE__);
        return -1;
    }

    if (cid < 0 || cid >= Manager->size)
    {
        fail("coroutine id out of range", "co_wait", __LINE__);
        return -1;
    }
    if (Manager->Cors[cid]->state == FINISHED)
    {
        // fail("coroutine already finished", "co_wait", __LINE__);
        return 0;
    }
    if (Manager->Cors[cid]->state == RUNNING)
    {
        fail("waiting for a running coroutine", "co_wait", __LINE__);
        return -1;
    }
    if (Manager->RunningCors->state == RUNNING)
    {
        Manager->RunningCors->state = READY;
    }
    if (Manager->Cors[cid]->state == READY || Manager->Cors[cid]->state == WAITING)
    {
        Manager->RunningCors->pos = LinkedList_push_item(Manager->Cors[cid]->WaitList, Manager->RunningCors);
        Manager->RunningCors->state = WAITING;
        struct coroutine *NextCor = LinkedList_head_item(Manager->AvailableList);
        if (NextCor == NULL)
        {
            printf("coroutine %d: ", Manager->RunningCors->id);
            DEBUG_COROUTINE_INFO(-1);
            fail("No Available Coroutine", "co_wait", __LINE__);
            return -1;
        }
        LinkedList_popFirst(Manager->AvailableList);
        //free(NextCor->pos);
        if (!setjmp(*(Manager->RunningCors->buf)))
        {
#ifdef DEBUG
            printf("[co_wait]switch from coroutine %d(wait for %d) to coroutine %d\n", Manager->RunningCors->id, cid, NextCor->id);
#endif
            Manager->RunningCors = NextCor;
            NextCor->state = RUNNING;
            longjmp(*(NextCor->buf), 1);
        }
        else
        {
            return 0;
        }
    }
    fail("never should've been here", "co_wait", __LINE__);
    return 0;
}
int co_status(int cid)
{
    check_init();
    ++cid;
    if (cid < 0 || cid >= Manager->size)
    {
        return -1;
    }
    return Manager->Cors[cid]->state;
}