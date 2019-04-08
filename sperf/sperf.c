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

// anonymous pipe
int pipefd[2];

// child process
void child_proc(int argc, char *argv[]) {
  // redirect stdout
  close(pipefd[0]);
  dup2(pipefd[1], STDERR_FILENO);
  
  // close(pipefd[1]);

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
void parent_proc() {
  // redirect stdin
  close(pipefd[1]);
  dup2(pipefd[0], STDIN_FILENO);
  // close(pipefd[0]);
  

  // regular expression
  regex_t reg;
  // regmatch_t mat[2];

  int rt =
      regcomp(&reg, "(\\w+)\\(.*\\)\\s*=.+<([0-9]+\\.[0-9]+)>", REG_EXTENDED);
  if (rt) {
    error("regcomp");
  }

  char buf[BUFFER_LEN];
  puts("int the parent proc");
  while (fgets(buf, BUFFER_LEN, stdin)) {
    puts("here");
    // regexec(&reg, buf, 2, mat, 0);
    // char str1[BUFFER_LEN] = {0};
    // char str2[BUFFER_LEN] = {0};

    // strncpy(str1, buf + mat[0].rm_so, mat[0].rm_eo - mat[0].rm_so);
    // strncpy(str2, buf + mat[1].rm_so, mat[1].rm_eo - mat[1].rm_so);
    // puts(str1);
    // puts(str2);
    // puts(buf);
    // puts("\n");
  }
}

int main(int argc, char *argv[]) {
  pid_t pid = fork();
  if (pid < 0) {
    error("fork");
  }

  int ret = pipe(pipefd);
  if(ret == -1) {
    error("pipe");
  }

  if (pid == 0) {
    child_proc(argc, argv);
  } else {
    parent_proc();
  }
  return 0;
}
