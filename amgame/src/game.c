#include <game.h>

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

void show_main_rect(uint32_t color) {
  draw_rect_(main_rect_x * SIDE, main_rect_y * SIDE, SIDE, SIDE, color);
}

void generate_beans() {
  for (int i = 0; i < BEAN_NUM; ++i) {
    beans[i].x = rand() % 30;
    beans[i].y = rand() % 30;
    beans[i].status = true;
  }
}

void draw_picture() {
  for (int i = 0; i < BEAN_NUM; ++i) {
    if (beans[i].status == true) {
      draw_rect_(beans[i].x * SIDE, beans[i].y * SIDE, SIDE, SIDE, 0xffaaff);
    }
  }
  show_main_rect(0xffffff);
}

void update_beans_status() {
  for (int i = 0; i < BEAN_NUM; ++i) {
    if (beans[i].status == true && beans[i].x == main_rect_x &&
        beans[i].y == main_rect_y) {
      beans[i].status = false;
    }
  }
}

void up_date_screen() {
  splash(0);
  draw_picture();
  update_beans_status();
}

int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  generate_beans();
  while (1) {
    read_key_play();
    // draw_picture();
  }
  return 0;
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
        // splash(0);
        // show_main_rect(0xffffffff);
        break;
      case _KEY_DOWN:
        main_rect_y += dy;
        // splash(0);
        // show_main_rect(0xffffffff);
        break;
      case _KEY_LEFT:
        main_rect_x -= dx;
        // splash(0);
        // show_main_rect(0xffffffff);
        break;
      case _KEY_RIGHT:
        main_rect_x += dx;
        // splash(0);
        // show_main_rect(0xffffffff);
        break;
      default:
        break;
    }
    up_date_screen();
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

void splash(uint32_t color) {
  for (int x = 0; x * SIDE <= w; x++) {
    for (int y = 0; y * SIDE <= h; y++) {
      draw_rect_(x * SIDE, y * SIDE, SIDE, SIDE, color);  // white
    }
  }
}
