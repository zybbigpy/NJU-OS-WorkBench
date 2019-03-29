#include <stdarg.h>
#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char *itoa(int value, char *result) {
  char *ptr = result, *ptr1 = result, tmp_char;
  int tmp_value;

  do {
    tmp_value = value;
    value /= 10;
    *ptr++ = "9876543210123456789"[9 + (tmp_value - value * 10)];
  } while (value);

  // Apply negative sign
  if (tmp_value < 0) *ptr++ = '-';
  *ptr-- = '\0';
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr-- = *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}

char *utoa(unsigned value, char *result, int base) {
  // check that the base if valid
  if (base < 2 || base > 36) {
    *result = '\0';
    return result;
  }

  char *ptr = result, *ptr1 = result, tmp_char;
  unsigned tmp_value;

  do {
    tmp_value = value;
    value /= base;
    *ptr++ =
        "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxy"
        "z"[35 + (tmp_value - value * base)];
  } while (value);

  *ptr-- = '\0';
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr-- = *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}

// put one char in the buffer
static inline void _checked_put(char *out, size_t *counter, size_t size,
                                char c) {
  if (*counter < size) out[*counter] = c;
  ++*counter;
}

static size_t vsnprintf(char *out, size_t size, const char *format,
                        va_list args) {
  char buffer[16];
  size_t counter = 0;
  for (const char *i = format; *i; ++i)
    if (*i != '%')
      _checked_put(out, &counter, size, *i);
    else
      switch (*++i) {
        case 'c':
          _checked_put(out, &counter, size, (char)va_arg(args, int));
          break;
        case 'd':
          itoa(va_arg(args, int), buffer);
          for (char *j = buffer; *j; ++j) _checked_put(out, &counter, size, *j);
          break;
        case 's':
          for (char *j = va_arg(args, char *); *j; ++j)
            _checked_put(out, &counter, size, *j);
          break;
        case 'x':
          utoa(va_arg(args, unsigned), buffer, 16);
          for (char *j = buffer; *j; ++j) _checked_put(out, &counter, size, *j);
          break;
        default:
          _checked_put(out, &counter, size, (uint8_t)*i);
          break;
      }
  if (counter < size) out[counter] = '\0';

  return counter;
}

static inline void puts_(const char *s) {
  for (; *s; s++) _putc(*s);
}

int vprintf(const char *fmt, va_list args) {
  va_list copy;
  va_copy(copy, args);  // make a copy before reading on
  size_t size = vsnprintf(NULL, 0, fmt, copy) + 1;
  va_end(copy);

  char out[size + 1];  // the last one is for '\0'
  vsnprintf(out, size + 1, fmt, args);
  puts_(out);

  return size;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  va_list copy;
  va_copy(copy, ap);  // make a copy before reading on
  int size = vsnprintf(NULL, 0, fmt, copy) + 1;
  va_end(copy);
  vsnprintf(out, size, fmt, ap);

  return size;
}

// more usr friendly
int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vprintf(fmt, args);
  va_end(args);

  return ret;
}

// more usr friendly
int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int size = vsprintf(out, fmt, args);
  va_end(args);

  return size;
}

// more usr friendly
int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int size = vsnprintf(out, n, fmt, args);
  va_end(args);

  return size;
}

#endif
