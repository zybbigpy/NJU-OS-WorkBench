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

typedef int (*func_ptr)();

static int dynamic_lib_id = 0;
static void* dynamic_lib_handlers[MAX_LIB_NUM];

// temp dir for .so and .c files
static char temp_dir[] = "./crepl-tmp-XXXXXX";

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

void* compile(const char* strin) {
  printf("in the compile func\n");
  char c_file_path[MAX_LINE_SIZE];
  char so_file_path[MAX_LINE_SIZE];

  // create file path
  sprintf(c_file_path, "%s/crep%d.c", temp_dir, dynamic_lib_id);
  sprintf(so_file_path, "%s/libcrep%d.so", temp_dir, dynamic_lib_id);

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
    if (ret == -1) {
      error("in gcc compile");
    }
  } else {  // parent: wait for compile finish
    wait(NULL);
  }

  void* handler = dlopen(so_file_path, RTLD_GLOBAL | RTLD_LAZY);
  if (handler) {
    dynamic_lib_handlers[dynamic_lib_id++] = handler;
  }

  return handler;
}

void compute(const char* strin) {
  // wrap the computation
  char c_code[MAX_LINE_SIZE];
  char func_name[MAX_LINE_SIZE];
  sprintf(func_name, "__expr_wrap_%d", dynamic_lib_id);
  sprintf(c_code, "int __expr_wrap_%d() { return %s; }", dynamic_lib_id, strin);

  void* handler = compile(c_code);
  if (handler) {
    func_ptr func = dlsym(handler, func_name);
    printf("%d\n", func());
  }
}

int is_func(const char* strin) {  // only for funcs like int func();
  char prefix[] = "int ";
  int ret = 0;
  if (strncmp(strin, prefix, strlen(prefix)) == 0) ret = 1;
  return ret;
}

int is_expr(const char* strin) {  // two cases: expr or func
  return !is_func(strin);
}

void cleanup() {
  // remove temp dir
  printf("in the clean up func\n");
  char cmd[MAX_LINE_SIZE];
  sprintf(cmd, "rm -rf %s", temp_dir);
  if (system(cmd)) {
    error("error in remove tempdir");
  }

  for (int i = 0; i < dynamic_lib_id; ++i) {
    dlclose(dynamic_lib_handlers[i]);
  }
}

int main() {
  atexit(cleanup);
  if (!mkdtemp(temp_dir)) {
    error("error in mkdtemp()");
  }

  printf(
      "This is crepl v0.1. type `exit` to exit.\n"
      "Attension: MAX LINE SIZE you can type in is 1024.\n");

  char strin[MAX_LINE_SIZE] = {0};
  while (1) {
    read_line(strin);
    if (strcmp(strin, "exit\n") == 0) {
      exit(EXIT_SUCCESS);
    } else if (strcmp(strin, "\n") == 0) {
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
