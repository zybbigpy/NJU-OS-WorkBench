#include <am.h>
#include <amdev.h>
#include <klib.h>

#define SIDE 16
#define true 1
#define false 0
#define BEAN_NUM 10

typedef struct bean {
  int x;
  int y;
  int status;
} Bean;

Bean beans[BEAN_NUM];

int main_rect_x = 5;
int main_rect_y = 6;

int dx = 1;
int dy = 1;

static inline void puts_(const char *s) {
  for (; *s; s++) _putc(*s);
}

uint32_t color_array[] = {0xffffff, 0xffccff, 0xbbffff, 0xeeaaff};
int w, h;
int num_beans_left = BEAN_NUM;
void generate_beans();
void init_screen();
void splash();
void read_key_play();
void draw_picture();
void update_beans_status();