#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main(){
    
    pid_t pid;
    // OPEN FILES
    int fd;
    fd = open("test.txt" , O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        /* code */
        printf("error in opening file");
        return 1;
    }
    //write 'hello fcntl!' to file

    /* code */
    char str[20] = "hello fcntl!";
    write(fd, str, 12);

    

    // DUPLICATE FD

    /* code */
    int fd2 = dup(fd);
    if(fd2 == -1){
        printf("error in dup");
        return 1;
    }

    // FORK

    pid = fork();

    if(pid < 0){
        // FAILS
        printf("error in fork");
        return 1;
    }
    
    struct flock fl;

    if(pid > 0){
        // PARENT PROCESS
        //set the lock
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 12;
        fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);
        //append 'b'
        str[12] = 'b';
        write(fd, str+12, 1);        
        //unlock
        fl.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &fl);
        sleep(3);
        // printf("%s", str); the feedback should be 'hello fcntl!ba'
        char st1[20];
        // set the read pointer to the beginning of the file
        lseek(fd, 0, SEEK_SET);
        read(fd, st1, 14);
        printf("%s\n", st1);
        exit(0);

    } else {
        // CHILD PROCESS
        sleep(2);
        //get the lock F_GETLK
        fcntl(fd2, F_GETLK, &fl);
        //append 'a'
        str[13] = 'a';
        write(fd2, str+13, 1);
        exit(0);
    }
    close(fd);
    return 0;
}