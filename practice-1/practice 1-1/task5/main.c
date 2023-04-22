// ? Loc here: header modification to adapt pthread_setaffinity_np
#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include<assert.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
void *thread1(void* dummy){
    assert(sched_getcpu() == 0);
    return NULL;
}

void *thread2(void* dummy){
    assert(sched_getcpu() == 1);
    return NULL;
}
int main(){
    pthread_t pid[2];
    int i;
    // ? LoC: Bind core here
    cpu_set_t CPU_SET0;
    cpu_set_t CPU_SET1;
    CPU_ZERO(&CPU_SET1);
    CPU_ZERO(&CPU_SET0);
    CPU_SET(0,&CPU_SET0);
    CPU_SET(1,&CPU_SET1);
    pthread_attr_t t1,t2;
    pthread_attr_init(&t1);
    pthread_attr_init(&t2);
    pthread_attr_setaffinity_np(&t1,sizeof(CPU_SET0),&CPU_SET0);
    pthread_attr_setaffinity_np(&t2,sizeof(CPU_SET1),&CPU_SET1);
    for(i = 0; i < 2; ++i){
        // 1 Loc code here: create thread and save in pid[2]
        pthread_create(&pid[i],i==0?&t1:&t2,i==0?(void *)thread1:(void*)thread2,NULL);
    }
    for(i = 0; i < 2; ++i){
        // 1 Loc code here: join thread
        pthread_join(pid[i],NULL);
    }
    return 0;
}
