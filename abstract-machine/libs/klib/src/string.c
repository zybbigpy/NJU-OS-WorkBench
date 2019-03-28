#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t size;
  for (size = 0; s[size] != '\0'; size++)
    ;

  return size;
}

char *strcpy(char *dst, const char *src) {
  size_t i;
  for (i = 0; src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  dst[i] = '\0';

  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  for (; i < n; i++) {
    dst[i] = '\0';
  }

  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i;
  for (i = 0; src[i] != '\0'; i++) {
    dst[dst_len + i] = src[i];
  }
  dst[dst_len + i] = '\0';

  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    if (*s1 != *s2) return *s1 - *s2;
    s1++;
    s2++;
  }

  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  while (*s1 && *s2 && i < n) {
    if (*s1 != *s2) return *s1 - *s2;
    s1++;
    s2++;
    i++;
  }
  if (i == n) {
    return 0;
  } else {
    return *s1 - *s2;
  }
}

void *memset(void *v, int c, size_t n) {
  while (n--) {
    *(uint8_t *)v++ = (uint8_t)c;
  }

  return v;
}

void *memcpy(void *out, const void *in, size_t n) {
  while (n--) {
    *(uint8_t *)out++ = *(uint8_t *)in++;
  }

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  while (n--) {
    uint8_t char1 = *(uint8_t *)s1++;
    uint8_t char2 = *(uint8_t *)s2++;
    if (char1 < char2) {
      return -1;
    } else if (char1 > char2) {
      return 1;
    }
  }

  return 0;
}

#endif
