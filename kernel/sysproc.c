#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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

uint64
sys_mmap(){
  uint64 addr, length, offset;
  int prot, flags, fd;
  struct file* file;

  try(argaddr(0, &addr), return -1)
  try(argaddr(1, &length), return -1)
  try(argint(2, &prot), return -1)
  try(argint(3, &flags), return -1)
  try(argfd(4, &fd, &file), return -1)
  try(argaddr(5, &offset), return -1)

  struct proc* p = myproc();
  if (addr || offset)
    return -1;
  if (!file->writable && (prot & PROT_WRITE) && (flags & MAP_SHARED))
    return -1;

  int unuse_idx = -1;
  uint64 sta_addr = get_mmap_space(length, p->mmap_vmas, &unuse_idx);

  if (unuse_idx == -1)
    return -1;
  if (sta_addr <= p->sz)  // mmap if there if there is no more memory left
    return -1;
  struct mmap_vma* cur_vma = &p->mmap_vmas[unuse_idx];
  cur_vma->file = file;
  cur_vma->in_use = 1;
  cur_vma->prot = prot;
  cur_vma->flags = flags;
  cur_vma->sta_addr = sta_addr;
  cur_vma->sz = length;
  filedup(file);          // increase the reference count
  return cur_vma->sta_addr; 
}

uint64 
sys_munmap(){
  uint64 addr;
  uint64 len;
  try(argaddr(0, &addr), return -1);
  try(argaddr(1, &len), return -1);
  return munmap(addr, len);
}