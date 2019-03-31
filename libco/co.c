#include "co.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STACK_SIZE 4086
#define CO_NUM_MAX 15

#define INIT 0
#define END 1
#define YIELD 2

struct co {
  char name[10];
  int id;

  func_t func;
  void *args;
  void *stack;

  struct co *prev;
  struct co *next;

  jmp_buf ctx;
};  // CO definition

jmp_buf main_ctx;             // main jmp ctx
static struct co *coroutins;  // use linked list to store coroutines
static struct co *current;    // current coroutine
static int co_no = 0;         // total co number

void co_init() {
  coroutins = NULL;
  current = NULL;
}

struct co *co_start(const char *name, func_t func, void *arg) {
  struct co *co = (struct co *)malloc(sizeof(struct co));
  co->args = arg;
  co->func = func;
  co->stack = malloc(sizeof(STACK_SIZE));
  co->id = co_no;
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

  if (setjmp(co->ctx)) {
    func(arg);
    longjmp(main_ctx, END);
  }
  return co;
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
      break;

    case YIELD:
      current = current->next;
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
      break;
  }
  assert(current);
  longjmp(current->ctx, 1);
}

void co_yield() {
  if (setjmp(current->ctx)) {
    return;
  } else {
    longjmp(main_ctx, YIELD);
  }
}