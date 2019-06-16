#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <klib.h>

union header {
  struct {
    union header* next;
    unsigned unit_size; // the unit is sizeof(header)
  };
  long align;
};
typedef union header Header;

#endif