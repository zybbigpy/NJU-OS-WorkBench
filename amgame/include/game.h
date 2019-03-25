#include <am.h>
#include <amdev.h>

#define SIDE 16
#define true 1
#define false 0
#define BEAN_NUM 5

typedef struct bean {
  int x;
  int y;
  int status;
} Bean;

Bean beans[BEAN_NUM];

int main_rect_x = 5;
int main_rect_y = 6;

static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}

void generate_beans();