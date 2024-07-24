#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// system call that reports which pages have been accessed
// takes in the first page to be checked (base)
// the number of pages to be checked and
// the mask for recording the results of the accesses
int
sys_pgaccess(void){
  //using arg functions to get parameters from users
  pagetable_t u_pt = myproc()->pagetable;
  uint64 fir_addr, mask_addr;
  uint ck_siz;
  // corresponds to the 3 parameters
  uint mask = 0;
  try(argaddr(0, &fir_addr), return -1);
  try(argint(1, &ck_siz), return -1);
  try(argaddr(2, &mask_addr), return -1);

  if (ck_siz > 32)  // mask not big enough to store
    return -1;

  pte_t* fir_pte = walk(u_pt, fir_addr, 0);

  // getting the mask of the ck_siz ptes after the first address
  for(int i = 0; i < ck_siz; i++){
    if((fir_pte[i] & PTE_A) && (fir_pte[i] & PTE_V)){
      mask |= (1<<i);
      fir_pte[i] ^= PTE_A;  // reset the access bit to 0
    }
  }

  // passing the mask to the user side
  try(copyout(u_pt, (uint*)mask_addr, &mask, sizeof(uint)), return -1);
}