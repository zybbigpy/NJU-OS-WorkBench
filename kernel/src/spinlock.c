#include <am.h>
#include <klib.h>
#include <spinlock.h>

int spinlock_init(spinlock *lk, char *name) {
  lk->cpu_number = 0;
  lk->name = name;
  lk->locked = 0;
}

int spinlock_lock(spinlock *lk) {
  while (_atomic_xchg(&lk->locked, 1) != 0)
    ;

  lk->cpu_number = _cpu();
}

int spinlock_unlock(spinlock *lk) {

    lk->cpu_number = 0;
    _atomic_xchg(&lk->locked, 0);
}