#include "co.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 4096
#define CO_NUM_MAX 15

#define INIT 0
#define END 1
#define YIELD 2

#if defined(__i386__)
#define SP "%%esp"
#elif defined(__x86_64__)
#define SP "%%rsp"
#endif

struct co {
  char name[10];
  int id;
  int initialized;

  func_t func;
  void *args;
  void *stack;
  void *__stack_backup;

  struct co *prev;
  struct co *next;

  jmp_buf ctx;
};  // CO definition

jmp_buf main_ctx;             // main jmp ctx
static struct co *coroutins;  // use linked list to store coroutines
static struct co *current;    // current coroutine
static int co_no = 0;         // total co number
void *__stack_backup;

void co_init() {
  coroutins = NULL;
  current = NULL;
}

struct co *co_start(const char *name, func_t func, void *arg) {
  struct co *co = (struct co *)malloc(sizeof(struct co));
  co->args = arg;
  co->func = func;
  co->stack = malloc(STACK_SIZE);
  co->id = co_no;
  co->initialized = 0;
  strcpy(co->name, name);

  co_no++;
  assert(co_no < CO_NUM_MAX);

  // add co into in the linked list
  if (!coroutins) {
    co->prev = co;
    co->next = co;
  } else {
    co->prev = coroutins->prev;
    co->next = coroutins;
    coroutins->prev->next = co;
    coroutins->prev = co;
  }
  coroutins = co;

  return co;
}

void co_init_(struct co *co) {
  co->initialized = 1;
  asm volatile("mov " SP ", %0; mov %1, " SP
               : "=g"(co->__stack_backup)
               : "g"((char *)co->stack + STACK_SIZE));
  printf("init [co %d] \n", co->id);
  co->func(co->args);
  // asm volatile("mov %0," SP : : "g"(co->__stack_backup));
  longjmp(main_ctx, END);
}

void co_destroy(struct co *thd) {
  free(thd->stack);
  free(thd);
}

void co_wait(struct co *thd) {
  if (coroutins == NULL) return;

  struct co *co_;
  switch (setjmp(main_ctx)) {
    case INIT:
      current = coroutins;
      co_init_(current);
      break;

    case YIELD:
      current = current->next;
      if (!current->initialized) {
        co_init_(current);
      } else {
        longjmp(current->ctx, 1);
      }

      break;

    case END:
      co_ = current;
      if (co_->next == co_) {
        coroutins = NULL;
        co_destroy(co_);
        return;
      }
      current = current->next;
      co_->prev->next = co_->next;
      co_->next->prev = co_->prev;
      co_destroy(co_);
      if (!current->initialized) {
        co_init_(current);
      } else {
        longjmp(current->ctx, 1);
      }

      break;
  }
  assert(current);
  // longjmp(current->ctx, 1);
}

void co_yield() {
  if (setjmp(current->ctx)) {
    return;
  } else {
    longjmp(main_ctx, YIELD);
  }
}

void co1(void *msg) {
  for (int i = 0; i != 5; ++i) {
    printf("%s\n", (char *)msg);
    co_yield();
  }
}

int main() {
  co_init();
  co_start("T1", co1, "hello");
  co_start("T2", co1, "world");
  co_wait(NULL);
  return 0;
}