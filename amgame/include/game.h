#include <am.h>
#include <amdev.h>
#include <klib.h>

#define SIDE 16
#define true 1
#define false 0
#define BEAN_NUM 10

// the beans
typedef struct bean {
  int x;
  int y;
  int status;
} Bean;

Bean beans[BEAN_NUM];
int num_beans_left = BEAN_NUM;

// your cordinate at the beginning
int main_rect_x = 5;
int main_rect_y = 6;

// every step size you will move
int dx = 1;
int dy = 1;

// change your color after you ate a bean
#define COLOR_NUM 6
uint32_t color_array[] = {0xC0FF3E, 0x8B008B, 0xFFB5C5,
                          0xEE3A8C, 0xCDB5CD, 0xB3EE3A};
uint32_t color_now = 0xffffff;

// screen size
int w, h;

// game related functions
void generate_beans();
void init_screen();
void clear();
void read_key_play();
void draw_picture();
void update_beans_status();
void draw_main_rect();

static inline void puts_(const char *s) {
  for (; *s; s++) _putc(*s);
}