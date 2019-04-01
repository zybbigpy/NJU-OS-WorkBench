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
  char stack[STACK_SIZE];
  void *__stack_backup;

  struct co *prev;
  struct co *next;

  jmp_buf ctx;
};  // CO definition

jmp_buf main_ctx;                    // main jmp ctx
static struct co *coroutins = NULL;  // use linked list to store coroutines
static struct co *current = NULL;    // current coroutine
static int co_no = 0;                // total co number

void co_init() {}

struct co *co_start(const char *name, func_t func, void *arg) {
  struct co *co = (struct co *)malloc(sizeof(struct co));
  co->args = arg;
  co->func = func;
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

static void co_init_(struct co *co) {
  co->initialized = 1;
  asm volatile("mov " SP ", %0; mov %1, " SP
               : "=g"(co->__stack_backup)
               : "g"(co->stack + STACK_SIZE));
  //printf("init co [%s], SP is [%p] \n", co->name, co->stack + STACK_SIZE);
  printf("init co[%s]",co->name);
  co->func(co->args);
  // asm volatile("mov %0," SP : : "g"(co->__stack_backup));
  longjmp(main_ctx, END);
}

static void co_destroy(struct co *thd) { free(thd); }

void co_wait(struct co *thd) {
  if (coroutins == NULL) return;

  struct co *co_;
  switch (setjmp(main_ctx)) {
    case INIT:
      current = coroutins;
      co_init_(current);

      break;

    case YIELD:
      printf("current co [%s] before yield [%p]\n", current->name, current);
      current = current->next;
      printf("current co [%s] after yield [%p]\n", current->name, current);
      if (!current->initialized) {
        co_init_(current);
      } else {
        printf("current co [%s] jmp_buf is [%p]\n", current->name,
               current->ctx);
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
}

void co_yield() {
  if (setjmp(current->ctx)) {
    printf("co [%s] in the yield, continune. \n", current->name);
    return;
  } else {
    printf("co [%s] in the yield, befor jmp.\n", current->name);
    longjmp(main_ctx, YIELD);
  }
}

// int g_count = 0;

// static void add_count() { g_count++; }

// static int get_count() { return g_count; }

// static void work_loop(void *arg) {
//   const char *s = (const char *)arg;
//   for (int i = 0; i < 5; ++i) {
//     printf("%s%d  \n", s, get_count());
//     add_count();
//     co_yield();
//   }
// }

// static void work(void *arg) { work_loop(arg); }

// void test() {
//   struct co *co1 = co_start("T1", work, "hello");
//   struct co *co2 = co_start("T2", work, "world");
//   co_wait(co1);
//   co_wait(co2);
// }

// int main() {
//   co_init();
//   test();
//   return 0;
// }