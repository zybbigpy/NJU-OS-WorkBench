#include <game.h>

int change_color_flag = false;

void draw_rect_(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h];  // WARNING: allocated on stack
  _DEV_VIDEO_FBCTL_t event = {
      .x = x,
      .y = y,
      .w = w,
      .h = h,
      .sync = 1,
      .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}

int get_sencond_now() {
  _DEV_TIMER_DATE_t date;
  _io_read(_DEV_TIMER, _DEVREG_TIMER_DATE, &date, sizeof(date));
  return date.second;
}

void draw_main_rect() {
  if (change_color_flag) {
    unsigned int seed = get_sencond_now();
    srand(seed);
    color_now = color_array[rand() % COLOR_NUM];
    change_color_flag = false;
  }
  draw_rect_(main_rect_x * SIDE, main_rect_y * SIDE, SIDE, SIDE, color_now);
}

void generate_beans() {
  unsigned int seed = get_sencond_now();
  srand(seed);
  for (int i = 0; i < BEAN_NUM; ++i) {
    beans[i].x = rand() % 30;
    beans[i].y = rand() % 30;
    beans[i].status = true;
  }
}

void draw_picture() {
  for (int i = 0; i < BEAN_NUM; ++i) {
    if (beans[i].status == true) {
      draw_rect_(beans[i].x * SIDE, beans[i].y * SIDE, SIDE, SIDE,
                 0xFFFF00);  // yellow beans
    }
  }
  draw_main_rect();
}

void update_beans_status() {
  for (int i = 0; i < BEAN_NUM; ++i) {
    if (beans[i].status == true && beans[i].x == main_rect_x &&
        beans[i].y == main_rect_y) {
      beans[i].status = false;
      num_beans_left--;
      change_color_flag =
          true;  // after you eat a bean, your color will change :)
    }
  }
}

void update_screen() {
  clear_screen();
  draw_picture();
  update_beans_status();
}

void read_key_play() {
  _DEV_INPUT_KBD_t event = {.keycode = _KEY_NONE};
#define KEYNAME(key) [_KEY_##key] = #key,
  static const char *key_names[] = {_KEYS(KEYNAME)};
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    switch (event.keycode) {
      case _KEY_UP:
        main_rect_y -= dy;
        break;
      case _KEY_DOWN:
        main_rect_y += dy;
        break;
      case _KEY_LEFT:
        main_rect_x -= dx;
        break;
      case _KEY_RIGHT:
        main_rect_x += dx;
        break;
      default:
        break;
    }
    update_screen();
    puts_("Key pressed: ");
    puts_(key_names[event.keycode]);
    puts_("\n");
  }
}

void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

void clear_screen(uint32_t color) {
  for (int x = 0; x * SIDE <= w; x++) {
    for (int y = 0; y * SIDE <= h; y++) {
      draw_rect_(x * SIDE, y * SIDE, SIDE, SIDE, 0);  // clear the whole screen
    }
  }
}

int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  generate_beans();
  while (1) {
    if (num_beans_left > 0) {
      read_key_play();
    } else {
      break;
    }
  }
  puts_("GAME completed, you ate all beans!");
  return 0;
}