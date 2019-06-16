#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


struct spinlock {
  char *name;
  int locked;
  int cpu_number;
};

typedef struct spinlock spinlock;

void spinlock_init(spinlock *lk, char *name);
void spinlock_lock(spinlock *lk);
void spinlock_unlock(spinlock *lk);

#endif