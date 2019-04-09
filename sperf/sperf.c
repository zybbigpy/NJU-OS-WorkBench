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
  char syscall_name[64];
  long long sys_time;  // micro secs
  double time_percent;
} SysCallInfo;

// global syscalls
SysCallInfo syscalls[MAX_SYSCALL_NUM];
int syscall_num = 0;

long long time_transfer(char *time) {
  long long ret = 0;
  while (*time) {
    if (*time >= '0' && *time <= '9') {
      ret = ret * 10 + *time - '0';
    }
    ++time;
  }
  return ret;
}

int find_syscall(char *name) {
  int ret = -1;
  for (int i = 0; i < syscall_num; ++i)
    if (strcmp(syscalls[i].syscall_name, name) == 0) ret = i;

  return ret;
}

void calculate_time_percent() {
  long long total_time = 0;
  for (int i = 0; i < syscall_num; ++i) total_time += syscalls[i].sys_time;

  for (int i = 0; i < syscall_num; ++i) 
    syscalls[i].time_percent = syscalls[i].sys_time *100 / total_time;
}

void add_syscall(char *name, char *time) {
  long long time_ = time_transfer(time);
  int find_id = find_syscall(name);
  if (find_id == -1) {
    strcpy(syscalls[syscall_num].syscall_name, name);
    syscalls[syscall_num].sys_time = time_;
    syscall_num++;
    assert(syscall_num < MAX_SYSCALL_NUM);
  } else {
    syscalls[find_id].sys_time += time_;
  }
}

void print_syscall() {
  calculate_time_percent();
  printf("the syscall num is %d. \n", syscall_num);
  for (int i = 0; i < syscall_num; ++i) {
    printf("sycall: %s, time %lld us, percentage %.2f %%\n",
           syscalls[i].syscall_name, syscalls[i].sys_time,
           syscalls[i].time_percent);
  }
}

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
    if (regexec(&reg, buf, 3, mat, 0) == 0) {
      char str1[BUFFER_LEN] = {0};
      char str2[BUFFER_LEN] = {0};

      strncpy(str1, buf + mat[1].rm_so, mat[1].rm_eo - mat[1].rm_so);  // name
      strncpy(str2, buf + mat[2].rm_so, mat[2].rm_eo - mat[2].rm_so);  // time
      add_syscall(str1, str2);
    }
  }
  print_syscall();
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
