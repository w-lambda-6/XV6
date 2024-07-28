#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define PRIME 35
#define R 0
#define W 1


//pl: left pipeline, going into a process 
//pr: right pipeline going out of a process into a child
void child(int* pl);

int 
main ()
{
    int p[2];
    pipe(p);

    if (fork() == 0){
        child(p);
    } else {
        close(p[R]);
        for (int i = 2; i < PRIME + 1; i++)
            write(p[W], &i, sizeof(int));
        close(p[W]);
        wait((int *) 0);
    }
    exit(0);
}

void 
child(int* pl)
{
    int pr[2];
    int n;

    close(pl[W]);
    if (read(pl[R], &n, sizeof(int)) == 0) exit(0);

    pipe(pr);

    if (fork() == 0){
        child(pr);
    } else {
        close(pr[R]);
        int prime = n;
        printf("prime %d\n", prime);
        while(read(pl[R], &n, sizeof(n))){
            if (n%prime!=0)
                write(pr[W], &n, sizeof(n));
        }
        close(pr[W]);
        close(pl[R]);
        wait((int*) 0);
        exit(0);
    }
}