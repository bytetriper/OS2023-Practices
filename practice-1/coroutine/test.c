#include"coroutine.h"
#include<stdio.h>
#include<assert.h>
__thread int a=1;
int func(void){
    a++;
    printf("%d\n",a);
}
void test(){
    co_start(func);
}
_Atomic int total_coroutine_count = 0;

int test_multithread_coroutine_inner()
{
    //total_coroutine_count++;
    return 1;
}
int test_multithread_coroutine()
{
    // printf("Running: %d, thread: %ld\n", co_getid() + 1, pthread_self());
    const int CNT = 2;
    cid_t coroutine[CNT];
    for (int i = 1; i < CNT; ++i)
    {
        coroutine[i] = co_start(test_multithread_coroutine_inner);
        //printf("coroutine: %d sub:%d iter:%d\n", co_getid() + 1, coroutine[i] + 1, i);
        //co_yield ();
        //if (i > 1)
        //{
        //    // printf("coid:%d cowait: %d\n", co_getid() + 1, i - 1);
        //    co_wait(coroutine[i - 1]);
        //    assert(co_status(coroutine[i - 1]) == FINISHED);
        //}
        //printf("coroutine: %d sub:%d second co_yield\n", co_getid() + 1, coroutine[i] + 1);
        //co_yield ();
        //assert(co_status(co_getid()) == RUNNING);
        //printf("coroutine: %d sub:%d third co_yield\n", co_getid() + 1, coroutine[i] + 1);
        //co_yield ();
    }
    //co_wait(coroutine[CNT - 1]);
    //assert(co_status(coroutine[CNT - 1]) == FINISHED);
    // printf("Coroutine finished: %d\n", co_getid());
    return 1;
}
void *test_multithread_thread(void *ptr)
{
    // printf("Thread: %ld\n", pthread_self());
    const int CNT = 2;
    cid_t coroutine[CNT];
    printf("Coroutine:%d,Tid:%ld\n", co_getid() + 1, pthread_self());
    for (int i = 0; i < CNT; ++i)
    {
        coroutine[i] = co_start(test_multithread_coroutine);
        // printf("Thread: %ld, coroutine: %d\n", pthread_self(), coroutine[i]);
    }
    co_waitall();
    // for (int i = 0; i < CNT; ++i)
    //     DEBUG_COROUTINE_INFO(coroutine[i]);
    //for (int i = 0; i < CNT; ++i)
    //{
    //    assert(co_getret(coroutine[i]) == 1);
    //    assert(co_status(coroutine[i]) == FINISHED);
    //}
    // printf("Thread finished: %ld\n", pthread_self());
}
int test_multithread()
{
    const int CNT = 1;
    pthread_t threads[CNT];
    total_coroutine_count = 0;
    int ret;
    for (int i = 0;i < CNT; ++i)
    {
        ret = pthread_create(threads + i, NULL, test_multithread_thread, NULL);
    }
    for (int i = 0; i < CNT; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    printf("Total coroutine count: %d\n", total_coroutine_count);
    assert(total_coroutine_count == 10000);
    return 0;
}
int main()
{
    co_start(test_multithread_coroutine_inner);
    //const int CNT = 1;
    //cid_t coroutine[CNT];
    //printf("Coroutine:%d,Tid:%ld\n", co_getid() + 1, pthread_self());
    //co_start(test_multithread_coroutine);
    //co_waitall();
    return 0;
}