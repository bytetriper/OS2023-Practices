// ? Loc here: header modification to adapt pthread_barrier
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
pthread_mutex_t lock;
pthread_barrier_t barrier;
// 2 Locs here: declare mutex and barrier
void *thread1(void* dummy){
    int i;
    pthread_barrier_wait(&barrier);
    // 1 Loc: mutex operation
    pthread_mutex_lock(&lock);
    // 1 Loc: barrier operation
    
    printf("This is thread 1!\n");
    for(i = 0; i < 20; ++i){
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
    // 1 Loc: mutex operation
    pthread_mutex_unlock(&lock);

    return NULL;
}

void *thread2(void* dummy){
    int i;
    pthread_barrier_wait(&barrier);

    // 1 Loc: mutex operation
    pthread_mutex_lock(&lock);

    // 1 Loc: barrier operation

    printf("This is thread 2!\n");
    for(i = 0; i < 20; ++i){
        printf("A");
        printf("p");
        printf("p");
        printf("l");
        printf("e");
        printf("?");
    }
    // 1 Loc: mutex operation
    pthread_mutex_unlock(&lock);
    return NULL;
}
int main(){
    pthread_t pid[2];
    int i;
    // 1 Loc: barrier initialization
    pthread_mutex_init(&lock,NULL);
    pthread_barrier_init(&barrier,NULL,3);
    // 3 Locs here: create 2 thread using thread1 and thread2 as function.
    pthread_create(&pid[0],NULL,(void*) thread1,NULL);
    pthread_create(&pid[1],NULL,(void *)thread2,NULL);
    // mutex initialization
    // 1 Loc: barrier operation
    pthread_barrier_wait(&barrier);
    for(i = 0; i < 2; ++i){
        // 1 Loc code here: join thread
        pthread_join(pid[i],NULL);
    }
    pthread_barrier_destroy(&barrier);
    return 0;
}
