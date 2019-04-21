#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
#define MAX_LIB_NUM 256

// static int dynamic_lib_id = 0;
// static void* dynamic_lib_handlers[MAX_LIB_NUM];

// temp dir for .so and .c files
// char temp_dir[] = "./temp/cprel_XXXXXX";

void error(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

char* read_line(char* strin) {  // getline from stdin
  // print promt
  printf(">> ");
  char* ret = fgets(strin, MAX_LINE_SIZE, stdin);
  if (ret == NULL) {
    error("fgets()");
  }
  return ret;
}

void compile(const char* strin) {}

void compute(const char* strin) {}

int is_func(const char* strin) {  // only for funcs like int func();
  char prefix[] = "int ";
  int ret = 0;
  if (strncmp(strin, prefix, strlen(prefix)) == 0) ret = 1;
  return ret;
}

int is_expr(const char* strin) {  // two cases: expr or func
  return !is_func(strin);
}

char template[] = "./template-XXXXXX";
int main() {
  char* ret = mkdtemp(template);
  if (!ret) {
    error("error in mkdtemp()");
  }

  char strin[MAX_LINE_SIZE] = {0};
  while (1) {
    read_line(strin);
    if (strcmp(strin, "exit\n") == 0) {
      // printf("in the exit\n");
      exit(EXIT_SUCCESS);
    } else if (strcmp(strin, "\n")) {
      // printf("in the newline\n");
      continue;
    } else if (is_func(strin)) {
      // printf("in the fuction\n");
      compile(strin);
    } else if (is_expr(strin)) {
      // printf("in the expression \n");
      compute(strin);
    }
  }
}
