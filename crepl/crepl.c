#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

void error(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int read_line(char *strin) {
  // print promt
  printf(">> ");
  int ret = fgets(strin, MAX_LINE_SIZE, stdin);
  if(ret == 0) {
    error("fgets()");
  }
  return ret;
}

int is_func() {}

int is_expr() {}

int main() {
  char strin[MAX_LINE_SIZE] = {0};
  while (read_line(strin)) {
    if (strcmp(strin, "exit\n")) {
      exit(EXIT_SUCCESS);
    } else if (strcmp(strin, "\n")) {
      continue;
    } else if (is_func(strin)) {
    } else if (is_expr(strin)) {
    }
  }
}
