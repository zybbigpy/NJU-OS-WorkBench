#ifndef __COMMON_H__

#include <kernel.h>
#include <nanos.h>

static inline void puts(const char *s) {
  for (; *s; s++) {
    _putc(*s);
  }
}
#define panic(s) \
  do { \
    puts("AM Panic: "); puts(s); \
    puts(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
    _halt(1); \
  } while(0)

#endif
