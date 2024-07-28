#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"
#define MAX_LEN 100

int 
main(int argc, char *argv[])
{
    char bf;
    char params[MAXARG][MAX_LEN]; //used to store all the input(including xargs) for all the lines
    char *args[MAXARG];           //used to store arguments passed-in to xargs in just one line
    char *cmd = argv[1];          //the command to be run on xargs

    while(1){
        int count = argc - 1; //current number of arguments, there will be more coming
        memset(params, 0, MAXARG*MAX_LEN);
        for (int i = 1; i < argc; i++){
            strcpy(params[i-1], argv[i]);
        }

        int cursor = 0; 
        int flag = 0;   //used to indicate there is an argument before the space, to handle multiple space problems
        int read_res;

        while ((read_res = read(0, &bf, sizeof(char))) > 0 && bf!='\n'){
            if (bf == ' ' && flag == 1){
                count++;

                cursor = 0; flag = 0;
            } else if (bf != ' '){
                params[count][cursor++] = bf;
                flag = 1;
            }
        }
        //encounters EOF of input or new line
        if (read_res <= 0 || bf == '\n') {
            break;
        }
        for (int i = 0; i < MAXARG-1; i++){
            args[i] = params[i];
        }
        args[MAXARG-1]=0;
        if (fork()==0){
            exec(cmd, args);
            exit(0);
        } else {
            wait((int*) 0);
        }
    }
    exit(0);
}