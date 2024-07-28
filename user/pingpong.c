#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
main()
{
    int cpPipe[2], pcPipe[2];
    char buf[1];

    if(pipe(cpPipe)==-1||pipe(pcPipe)==-1){
        exit(1);
    }

    write(pcPipe[1], buf, 1);

    if (fork()==0){
        close(pcPipe[1]); //only reads, doesn't write
        read(pcPipe[0], buf, 1);
        printf("%d: received ping\n", getpid());
        close(pcPipe[0]);
        write(cpPipe[1], buf, 1); //write to the parent process
        exit(0);
    } else if (fork() > 0){
        close(cpPipe[1]);
        read(cpPipe[0], buf, 1);
        printf("%d: received pong\n", getpid());
        close(cpPipe[0]);
        exit(0);
    } else {
        exit(1);
    }
}