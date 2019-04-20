#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

void error(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

char* read_line(char* strin) {
  // print promt
  printf(">> ");
  char* ret = fgets(strin, (int)MAX_LINE_SIZE, stdin);
  if (ret == NULL) {
    error("fgets()");
  }
  return ret;
}

int is_func() { return 0; }

int is_expr() { return 0; }

int main() {
  char strin[MAX_LINE_SIZE] = {0};
  while (1) {
    read_line(strin);
    printf("strin is %s\n", strin);
    if (strcmp(strin, "exit\n")) {
      printf("in the exit\n");
      exit(EXIT_SUCCESS);
    } else if (strcmp(strin, "\n")) {
      
      printf("in the ne wline\n");
      continue;
    } else if (is_func(strin)) {
      printf("in the fuction\n");
    } else if (is_expr(strin)) {
      printf("in the expression \n");
    }
  }
}
