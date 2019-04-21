#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE_SIZE 1024
#define MAX_LIB_NUM 256

static int dynamic_lib_id = 0;
static void* dynamic_lib_handlers[MAX_LIB_NUM];

// temp dir for .so and .c files
static char template[] = "./crepl-tmp-XXXXXX";

void error(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

char* read_line(char* strin) {  // getline from stdin
  printf(">> ");
  char* ret = fgets(strin, MAX_LINE_SIZE, stdin);
  if (ret == NULL) {
    error("fgets()");
  }
  return ret;
}

void compile(const char* strin) {
  printf("in the compile func\n");
  char c_file_path[MAX_LINE_SIZE];
  char so_file_path[MAX_LINE_SIZE];

  // create file path
  sprintf(c_file_path, "%s/crep%d.c", template, dynamic_lib_id);
  sprintf(so_file_path, "%s/libcrep%d.so", template, dynamic_lib_id);

  FILE* fp = fopen(c_file_path, "w");
  if (!fp) {
    error("error in open .c file");
  }
  fprintf(fp, "%s", strin);
  fclose(fp);

  pid_t pid = fork();
  if (pid < 0) {
    error("error in fork()");
  }
  if (pid == 0) {  // child: compile
    int ret = execlp("gcc", "gcc", "-fPIC", "-shared",
           "-Wno-implicit-function-declaration", "-o", so_file_path,
           c_file_path, (char*)NULL);
    if(ret  == -1) {
    error("in gcc compile");
    }
  } else { // parent: wait for compile finish
    wait(NULL);
  }

  void* handler = dlopen(so_file_path, RTLD_GLOBAL | RTLD_LAZY);
  assert(handler == NULL);
  dynamic_lib_handlers[dynamic_lib_id++] = handler;
}

void compute(const char* strin) {}

int is_func(const char* strin) {  // only for funcs like int func();
  printf("the strin is %s\n", strin);
  char prefix[] = "int ";
  size_t len = strlen(prefix);
  printf(" the prefix len is %d\n",len);

  int ret = 0;
  if (strncmp(strin, prefix, strlen(prefix)) == 0) ret = 1;
  return ret;
}

int is_expr(const char* strin) {  // two cases: expr or func
  return !is_func(strin);
}

void cleanup() {
  char cmd[MAX_LINE_SIZE];
  sprintf(cmd, "rm -r %s", template);
  if (system(cmd)) {
    error("error in remove tempdir");
  }
}

int main() {
  // atexit(cleanup);
  if (!mkdtemp(template)) {
    error("error in mkdtemp()");
  }
  // printf(" the temp dir name is %s\n", template);

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
      printf("in the fuction\n");
      compile(strin);
    } else if (is_expr(strin)) {
      printf("in the expression \n");
      compute(strin);
    }
  }
}
