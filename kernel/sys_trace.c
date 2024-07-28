#include "syscall.h"
#include "proc.h"
#include "user/user.h"


uint64_t
sys_trace(void)
{
    int n;

    if (argint(0, &n)<0)
        return -1;
    (myproc())->syscallnum = n; //put the tracing mask into the structure
    return 0;
}