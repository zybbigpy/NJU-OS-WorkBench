#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void error(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int pipefd[2];
void child_proc(char *args[]) {
  dup2(pipefd[1], STDOUT_FILENO);
  close(pipefd[0]);
  close(pipefd[1]);
  execvp("/usr/bin/strace", args);
}
void parent_proc() {
  dup2(pipefd[1], STDIN_FILENO);
  close(pipefd[0]);
  close(pipefd[1]);

  char buf[1024];
  while(fgets(buf, 1024, stdin)) {
    puts(buf);
  }
}

int main(int argc, char *argv[]) {
  pid_t pid = fork();
  if (pid < 0) {
    error("fork");
  }
  if (pid == 0) {
    char *args[10];
    args[0] = "strace";
    args[1] = "-T";
    for (int i = 1; i < argc; ++i) args[i + 1] = argv[i];
    child_proc(args);
  } else {
    parent_proc();
  }
  return 0;
}
