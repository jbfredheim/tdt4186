// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

uint64 MAX_PAGES = 0;
uint64 FREE_PAGES = 0;

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
    struct run *next;
    int ref_count;
};

struct
{
    struct spinlock lock;
    struct run *freelist;
} kmem;

void kinit()
{
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
    MAX_PAGES = FREE_PAGES;
}

void freerange(void *pa_start, void *pa_end) {
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE) {
        struct run *r = (struct run*)p;
        r->ref_count = 0;  // Initialize reference count to 0
        r->next = kmem.freelist;
        kmem.freelist = r;
        FREE_PAGES++;
    }
}

void incref(void *pa)
{
    struct run *r;

    r = (struct run *)pa;
    acquire(&kmem.lock);
    r->ref_count++;
    release(&kmem.lock);
}

void decref(void *pa)
{
    struct run *r;

    r = (struct run *)pa;
    acquire(&kmem.lock);
    r->ref_count--;
    release(&kmem.lock);
}

int getrefcnt(void *pa)
{
    struct run *r;

    r = (struct run *)pa;
    return r->ref_count;
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
    if (MAX_PAGES != 0)
        assert(FREE_PAGES < MAX_PAGES);
    struct run *r;

    if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");
        

    r = (struct run *)pa;

    acquire(&kmem.lock);
    if (r->ref_count > 0){
      r->ref_count--; // decrement the reference count
      if(r->ref_count <= 0)
      if (r->ref_count <= 0) { //if the pte is no longer in use
        r->next = kmem.freelist; 
        kmem.freelist = r;
        FREE_PAGES++;
        memset(pa, 1, PGSIZE);
      }
    }
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
    assert(FREE_PAGES > 0);
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r) {
        kmem.freelist = r->next;
        r->ref_count = 1;
    }
    release(&kmem.lock);

    if (r)
        memset((char *)r, 5, PGSIZE); // fill with junk
    FREE_PAGES--;
    return (void *)r;
}
