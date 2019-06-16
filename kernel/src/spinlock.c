#include <am.h>
#include <common.h>
#include <klib.h>
#include <spinlock.h>

// reference is spinlock.c from xv6 source code

static int intr_number = 0;
static int intr_status_save;
static void pushcli();
static void popcli();
static int holding_lock(spinlock *lk);

void spinlock_init(spinlock *lk, char *name) {
  // there is noe cpu0
  lk->cpu_number = 0;
  lk->name = name;
  lk->locked = 0;
}

void spinlock_lock(spinlock *lk) {
  // check lock holding
  if (holding_lock(lk)) {
    panic("holding_lock\n");
  }

  pushcli();
  while (_atomic_xchg(&lk->locked, 1) != 0)
    ;

  // get the lock
  lk->cpu_number = _cpu();
}

void spinlock_unlock(spinlock *lk) {
  // check lock holding
  if (!holding_lock(lk)) {
    panic("!holding_lock\n");
  }

  lk->cpu_number = 0;

  _atomic_xchg(&lk->locked, 0);

  popcli();
}

void pushcli() {
  int intr_status_now = _intr_read();
  // close interupt
  _intr_write(0);

  if (intr_number = 0) {
    intr_status_save = intr_status_now;
  }

  intr_number++;
}

void popcli() {
  int intr_status_now = _intr_read();
  if (intr_status_now == 1) {
    panic("popcli interruptible\n");
  }
  // open interupt
  --intr_number;
  if (intr_number < 0) {
    panic("popcli\n");
  }
  if ((intr_number == 0) && (intr_status_save == 0)) {
    _intr_write(1);
  }
}

int holding_lock(spinlock *lk) {
    pushcli();
    int ret = lk->locked && (lk->cpu_number == _cpu());
    popcli();
    return ret;
}