// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&refcnt_lock);
  if (--PG_REFCNT(pa) <= 0){ // reduce the number of references first, if less then 0 then truly free
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  release(&ref_lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    PG_REFCNT(r) = 1;            // one process using this page upon init
  }
  return (void*)r;
}


// Allocate physical memory for valid COW pages
int
cowalloc(pagetable_t pgtbl, uint64 va)
{
  pte_t* pte = walk(pgtbl, va, 0);
  uint64 perm = PTE_FLAGS(*pte);

  if (pte == 0) return -1;
  uint64 prev_sta = PTE2PA(*pte);       // the prev_sta here is the parent's page table originally
                                        // used by the page
                                        // we use sta here because the address is aligned with the page
                                        // sta represents the start of a page
  uint64 newpage = kalloc();
  if (!newpage) return -1;
  uint64 va_sta = PGROUNDDOWN(va);      // current page 

  perm &= (~PTE_C);                     // no longer valid COW after physical memory allocation
  perm |= PTE_W;                        // can write after allocation

  // must be this order
  // as after unvmmap, the physical pages
  // of the parent process may be released
  // and memmove will just copy invalid data
  memmove(newpage, prev_sta, PGSIZE);   // copy the parent's page table over
  unvmmap(pgtbl, va_sta, 1, 1);         // unmap from the parent's page table

  if (mappages(pgtbl, va_sta, PGSIZE, (uint64)newpage, perm) < 0){
    kfree(newpage);
    return -1;
  }
  return 0;
}

// lock added to avoid multiple processes calling kfree()
// at the same time reducing references at the same time 
// causing incorrect results
void 
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refcnt_lock, "ref cnt"); 
  freerange(end, (void*)PHYSTOP);
}