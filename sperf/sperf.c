#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <regex.h>


#define MAX_SYSCALL_NUM 512
// deal with error
void error(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

// syscall info
typedef struct SysCallInfo {
  char syscall_name[20];
  int total_time;
}SysCallInfo;

// global syscalls
SysCallInfo syscalls[MAX_SYSCALL_NUM];

// anonymous pipe
int pipefd[2];

// child process
void child_proc(int argc, char *argv[]) {
   // redirect stdout
  dup2(pipefd[1], STDOUT_FILENO);
  close(pipefd[0]);
  close(pipefd[1]);

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
  dup2(pipefd[1], STDIN_FILENO);
  close(pipefd[0]);
  close(pipefd[1]);

  char buf[1024];
  puts("int the parent proc");
  while (fgets(buf, 1024, stdin)) {
    puts(buf);
  }
}

int main(int argc, char *argv[]) {
  pid_t pid = fork();
  if (pid < 0) {
    error("fork");
  }
  if (pid == 0) {
    child_proc(argc, argv);
  } else {
    parent_proc();
  }
  return 0;
}
