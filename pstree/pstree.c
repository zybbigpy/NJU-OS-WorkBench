#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define MAX_PROC_NUM 20
#define MAX_FILE_ADDR_LEN 300

/*   the process information is in /proc/[pid]/stat.
**   ref: man 5 proc
*/
typedef struct ProcInfo {
  pid_t pid;
  char comm[50];
  char state[4];
  pid_t ppid;
  pid_t pgrp;
} ProcInfo;

ProcInfo sys_porcs[MAX_PROC_NUM];

/* the file addr is always /proc/[pid]/stat. */
int FillSysProcInfo(const char *file_addr, int proc_index) {

  FILE *fp = fopen(file_addr, "r");
  if (fp) {
    fscanf(fp, "%d%s%s%d%d", &sys_porcs[proc_index].pid,
           sys_porcs[proc_index].comm, sys_porcs[proc_index].state,
           &sys_porcs[proc_index].ppid, &sys_porcs[proc_index].pgrp);
     printf("process info is %d, %s, %s, %d\n", sys_porcs[proc_index].pid,
            sys_porcs[proc_index].comm, sys_porcs[proc_index].state,
            sys_porcs[proc_index].ppid);
    fclose(fp);
  } else {
    perror("open file fail \n");
    return 1;
  }
  return 0;
}

int OpenProcDir(const char *dir_addr) {
  DIR *dir;
  struct dirent *ptr;
  dir = opendir(dir_addr);
  if (dir) {
    while ((ptr = readdir(dir)) != NULL) {
      /* create process information */
      int proc_index = 0;
      if (isdigit(ptr->d_name[0])) {
        char file_addr[300];
        sprintf(file_addr, "%s%s%s", dir_addr, ptr->d_name, "/stat");
        // printf("the file name is %s \n", file_addr);
        int ret = FillSysProcInfo(file_addr, proc_index);
        if (ret) {
          perror("write sys procs array fail. \n");
          return 1;
        } else {
          proc_index++;
          if(proc_index == MAX_PROC_NUM) {
            perror("proc array is full!\n");
            return 1;
          }
        }
      }
    }
  } else {
    perror("open dir fail\n");
    return 1;
  }
  closedir(dir);
  return 0;
}

void PrintVersion() {
  printf(" Welcome, this is a simple pstree, version 0.1 \n");
}

int sorted = 0;

int main(int argc, char *argv[]) {
  // printf("Hello, World!\n");
  // int i;
  // for +(i = 0; i < argc; i++) {
  //   assert(argv[i]); // always true
  //   printf("argv[%d] = %s\n", i, argv[i]);
  // }
  // assert(!argv[argc]); // always true
  // return 0;

  //OpenProcDir("/proc/");

  int opt;
  while ((opt = getopt(argc, argv, "av"))!=-1 ) {
    switch (opt)
    {
      case 'a':
        sorted = 1;
        OpenProcDir("/proc/");
        break;

      case 'v':
        PrintVersion();
        break;
      default:
        PrintVersion();
        break;
    }
  }
  return 0;
}
