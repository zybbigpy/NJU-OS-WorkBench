#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


struct spinlock {
  char *name;
  int locked;
  int cpu_number;
};

typedef struct spinlock spinlock;

int spinlock_init(spinlock *lk, char *name);
int spinlock_lock(spinlock *lk);
int spinlock_unlock(spinlock *lk);

#endif