#include"coroutine.h"
#include<stdio.h>
__thread int a=1;
int func(void){
    a++;
    printf("%d\n",a);
}
void test(){
    co_start(func);
}
int main()
{
    int i;
    pthread_t pid[20];
    for(i=0;i<3;i++)
    {
        pthread_create(&pid[i],NULL,(void*)test,NULL);
    }
    for(i=0;i<3;i++)
    {
        pthread_join(pid[i],NULL);
    }
    return 0;
}