#include <game.h>

void init_screen();
void splash();
void read_key();

int main_rect_x = 10;
int main_rect_y = 10;
int dx = 5;
int dy = 5;

void draw_rect(int x, int y, int w, int h, uint32_t color) {
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
  draw_rect(main_rect_x * SIDE, main_rect_y * SIDE, SIDE, SIDE, color);
}

int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  // splash();
  while (1) {
    read_key();
    // splash();
  }
  return 0;
}

void read_key() {
  _DEV_INPUT_KBD_t event = {.keycode = _KEY_NONE};
#define KEYNAME(key) [_KEY_##key] = #key,
  static const char *key_names[] = {_KEYS(KEYNAME)};
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    // if (event.keycode == _KEY_UP) {
    //   splash(0xffffffff);
    // }
    // if (event.keycode == _KEY_DOWN) {
    //   splash(0);
    // }
    switch (event.keycode) {
      case _KEY_UP:
        main_rect_y += dx;
        show_main_rect(0xffffffff);
        break;
      case _KEY_DOWN:
        main_rect_y -= dy;
        show_main_rect(0xffffffff);
        break;
      case _KEY_LEFT:
        main_rect_x -= dx;
        show_main_rect(0xffffffff);
        break;
      case _KEY_RIGHT:
        main_rect_x += dx;
        show_main_rect(0xffffffff);
        break;
      default:
        break;
    }

    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
}

int w, h;

void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}

// void splash(uint32_t color) {
//   for (int x = 0; x * SIDE <= w; x++) {
//     for (int y = 0; y * SIDE <= h; y++) {
//       if ((x & 1) ^ (y & 1)) {
//         draw_rect(x * SIDE, y * SIDE, SIDE, SIDE, color);  // white
//       }
//     }
//   }
// }
