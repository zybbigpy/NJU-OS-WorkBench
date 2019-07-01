#include <alloc.h>
#include <common.h>
#include <klib.h>
#include <spinlock.h>

static uintptr_t pm_start, pm_end;

static void pmm_init();
static void *kalloc(size_t size);
static void kfree(void *ptr);
static char *sbrkptr = NULL;

static Header base;
static Header *freep = NULL;  // linked list for free blocks
static alloc_lock;

MODULE_DEF(pmm){
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end = (uintptr_t)_heap.end;
  // sbrkptr = (char *)((pm_start + sizeof(Header) - 1) & ~(sizeof(Header) -
  // 1));
  sbrkptr = (char *)pm_start;
  spinlock_init(&allloc_lock, "alloc_lock");
}

#define NALLOC 1024
static void *kalloc_unsafe(size_t size);
static void kfree_unsafe(void *ptr);
static Header *more_units(size_t units);
static char *ksbrk(size_t size);

static void *kalloc_unsafe(size_t size) {
  Header *cur;
  Header *prev;

  size_t nunits = (size + sizeof(Header) - 1) / sizeof(Header) + 1;

  if ((prev = freep) == NULL) {
    base.unit_size = 0;
    freep = &base;
    prev = &base;
    base.next = &base;
  }

  for (cur = prev->next;; prev = cur, cur = cur->next) {
    if (cur->unit_size >= nunits) {
      if (cur->unit_size == nunits) {
        prev->next = cur->next;
      } else {
        cur->unit_size -= nunits;
        cur += cur->unit_size;
        cur->unit_size = nunits;
      }
      freep = prev;
      return (void *)(cur + 1);
    }

    // can not find the block
    if (cur == freep) {
      if ((cur = more_units(nunits)) == NULL) {
        return NULL;
      }
    }
  }
}

static Header *more_units(size_t units) {
  if (units < NALLOC) {
    units = NALLOC;
  }

  char *cp = ksbrk(units * sizeof(Header));

  if (cp == (char *)-1) {
    return NULL;
  }

  Header *bp = (Header *)cp;
  bp->unit_size = units;
  kfree((void *)(bp + 1));

  return freep;
}

static void kfree_unsafe(void *ptr) {
  Header *bp = (Header *)ptr - 1;
  Header *cur;

  for (cur = freep; !(bp > cur && bp < cur->next); cur = cur->next) {
    if ((cur >= cur->next) && (bp > cur || bp < cur->next)) {
      break;
    }
  }

  if (bp + bp->unit_size == cur->next) {
    bp->unit_size += cur->next->unit_size;
    bp->next = cur->next->next;
  } else {
    bp->next = cur->next;
  }

  if (cur + cur->unit_size == bp) {
    cur->unit_size += bp->unit_size;
    cur->next = bp->next;
  } else {
    cur->next = bp;
  }

  freep = cur;
}

static char *ksbrk(size_t size) {
  char *old_sbrkptr = sbrkptr;

  if (sbrkptr + size > (char *)pm_end) {
    panic("the memory runs out \n");
    return (void *)-1;
  }

  sbrkptr += size;

  return old_sbrkptr;
}

static void *kalloc(size_t size) {
  spinlock_lock(&alloc_lock);
  void *ret = kalloc_unsafe(size);
  spinlock_unlock(&alloc_lock);
  return ret;
}

static void kfree(void *ptr) {
  spinlock_lock(&alloc_lock);
  kfree_unsafe(ptr);
  spinlock_unlock(&alloc_lock);
}
