#include <assert.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_SYSCALL_NUM 512
#define BUFFER_LEN 1024

// deal with error
void error(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

// syscall info
typedef struct SysCallInfo {
  char syscall_name[20];
  int total_time;
} SysCallInfo;

// global syscalls
SysCallInfo syscalls[MAX_SYSCALL_NUM];

// child process
void child_proc(int fd, int argc, char *argv[]) {
  // redirect stdout
  dup2(fd, STDERR_FILENO);
  close(fd);

  // get args for strace -T (-T to get time)
  char *args[16];
  args[0] = "strace";
  args[1] = "-T";

  for (int i = 1; i < argc; ++i) args[i + 1] = argv[i];
  args[argc + 1] = NULL;  // end flag of args
  execvp("/usr/bin/strace", args);

  // codes should not arrive here
  error("execv");
}

// parent process
void parent_proc(int fd) {
  // redirect stdin
  dup2(fd, STDIN_FILENO);
  close(fd);

  // regular expression
  regex_t reg;
  regmatch_t mat[3];

  int rt =
      regcomp(&reg, "(\\w+)\\(.*\\)\\s*=.+<([0-9]+\\.[0-9]+)>", REG_EXTENDED);
  if (rt) {
    error("regcomp");
  }

  char buf[BUFFER_LEN];
  assert(fgets(buf, BUFFER_LEN, stdin) != NULL);
  while (fgets(buf, BUFFER_LEN, stdin)) {
    regexec(&reg, buf, 3, mat, 0);
    char str1[BUFFER_LEN] = {0};
    char str2[BUFFER_LEN] = {0};

    strncpy(str1, buf + mat[0].rm_so, mat[0].rm_eo - mat[0].rm_so);
    
    strncpy(str2, buf + mat[1].rm_so, mat[1].rm_eo - mat[1].rm_so);
    puts(str1);
    puts(str2);
  }
}

int main(int argc, char *argv[]) {
  int pipefd[2];

  int ret = pipe(pipefd);
  if (ret == -1) {
    error("pipe");
  }

  pid_t pid = fork();
  if (pid < 0) {
    error("fork");
  }
  if (pid == 0) {
    close(pipefd[0]);
    child_proc(pipefd[1], argc, argv);
  } else {
    close(pipefd[1]);
    parent_proc(pipefd[0]);
  }
  return 0;
}
