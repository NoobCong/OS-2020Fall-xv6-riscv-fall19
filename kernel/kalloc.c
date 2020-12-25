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
//用于分配物理内存
struct run
{
  struct run *next;
};

struct
{
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

//系统启动时，调用kinit，对内存分配器进行初始化
//kinit主要是调用freerange，而freerange的作用是，
//将end~PHYSTOP之间的内存一页页地加入到free list中进行管理，
//这个free list的头节点，就是之前定义的kmem.freelist。
void kinit()
{
  //printf(" NCPU :%d\n",NCPU);
  for (int i = 0; i < NCPU; i++)
  {
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
  struct run *r;
  char *p;
  pa_start = (char *)PGROUNDUP((uint64)pa_start);
  uint64 per_cpu_size = PGROUNDDOWN(((uint64)pa_end - (uint64)pa_start) / NCPU);
  for (int i = 0; i < NCPU; i++)
  {
    for (p = (char *)pa_start + (i * per_cpu_size); p + PGSIZE <= (char *)pa_start + ((i + 1) * per_cpu_size); p += PGSIZE)
    {
      memset(p, 1, PGSIZE);
      r = (struct run *)p;
      acquire(&kmem[i].lock);
      r->next = kmem[i].freelist;
      kmem[i].freelist = r;
      release(&kmem[i].lock);
    }
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
//kfree的作用是将从起始地址开始的一页物理内存加入到free list中，
//他的参数，即物理内存的地址都应该是按页对齐的，
//他的参数只有两种情况，一是在初始化时，
//从end~phystop之间的，此时的地址，都是经过按页对齐的，
//第二种是由kalloc返回的，因为kalloc返回的地址是在
//freelist中取下来的，所以自然也是按页对齐的
void kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  push_off();
  int cpu_id = cpuid();
  pop_off();
  acquire(&kmem[cpu_id].lock);
  r->next = kmem[cpu_id].freelist;
  kmem[cpu_id].freelist = r;
  release(&kmem[cpu_id].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int cpu_id = cpuid();
  pop_off();
  acquire(&kmem[cpu_id].lock);
  r = kmem[cpu_id].freelist;
  if (r)
    kmem[cpu_id].freelist = r->next;
  release(&kmem[cpu_id].lock);

  if (!r)
  {
    for (int i = 0; i < NCPU; i++)
    {
      acquire(&kmem[i].lock);
      r = kmem[i].freelist;
      if (r)
      {
        kmem[i].freelist = r->next;
        release(&kmem[i].lock);
        break;
      }
      release(&kmem[i].lock);
    }
  }
  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}