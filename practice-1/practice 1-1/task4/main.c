// ? Loc here: header modification to adapt pthread_cond_t
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#define MAXTHREAD 10
// declare cond_variable: you may define MAXTHREAD variables
pthread_cond_t cond[MAXTHREAD];
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
// ? Loc in thread1: you can do any modification here, but it should be less than 20 Locs
void *thread1(void *dummy)
{
    int i;
    int k=*((int *)dummy);
    pthread_mutex_lock(&lock); 
    if(k>0)
    {   
        pthread_cond_wait(&cond[k-1],&lock);
    }
    printf("This is thread %d!\n", *((int *)dummy));
    for (i = 0; i < 20; ++i)
    {
        printf("H");
        printf("e");
        printf("l");
        printf("l");
        printf("o");
        printf("W");
        printf("o");
        printf("r");
        printf("l");
        printf("d");
        printf("!");
    }
    
   // sleep(0.1);
    pthread_mutex_unlock(&lock); 
    if(k==0) 
        sleep(1);
    pthread_cond_signal(&cond[k]);
    
    //pthread_mutex_unlock(&lock[k+1]);
    return NULL;
}

int main()
{
    pthread_t pid[MAXTHREAD];
    int i;
    // ? Locs: initialize the cond_variables
    for (i = 0; i < MAXTHREAD; ++i)
    {    
        pthread_cond_init(&cond[i], NULL);
    }
    for (i = 0; i < MAXTHREAD; ++i)
    {
        int *thr = (int *)malloc(sizeof(int));
        *thr = i;
        pthread_create(&pid[i],NULL,(void*)thread1,(void*) thr);
        // 1 Loc here: create thread and pass thr as parameter
    }
    for (i = 0; i < MAXTHREAD; ++i)
        // 1 Loc here: join thread
        pthread_join(pid[i],NULL);
        return 0;
}
