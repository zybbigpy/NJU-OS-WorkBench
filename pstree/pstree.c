#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define MAX_PROC_NUM 2048
#define MAX_FILE_ADDR_LEN 300

/*   the process information is in /proc/[pid]/stat.
**   ref: man 5 proc
*/
typedef struct ProcInfo {
  pid_t pid;
  char comm[20];
  char state;
  pid_t ppid;
  pid_t pgrp;
} ProcInfo;

ProcInfo sys_porcs[MAX_PROC_NUM];

bool IsStrDigt(const char *str) {
  // bool ret = true;
  // for (int i = 0; i < strlen(str); i++) {
  //   if (!isdigit(str[i])) {
  //     ret = false;
  //     break;
  //   }
  // }
  // return ret;
  return isdigit(str[0]);
}

/* the file addr is always /proc/[pid]/stat. */
int FillSysProcInfo(const char *file_addr) {
  
  return 1;
}

int OpenProcDir(const char *dir_addr) {
  DIR *dir;
  struct dirent *ptr;
  dir = opendir(dir_addr);
  if (dir) {
    while ((ptr = readdir(dir)) != NULL) {
      /* create process information */
      if(IsStrDigt(ptr->d_name)) {
        char file_addr[300];
        sprintf(file_addr, "%s%s%s", dir_addr, ptr->d_name, "/stat");
        printf("the file name is %s \n",file_addr);
      }
     
    }
  } else {
    perror("open dir fail\n");
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // printf("Hello, World!\n");
  // int i;
  // for (i = 0; i < argc; i++) {
  //   assert(argv[i]); // always true
  //   printf("argv[%d] = %s\n", i, argv[i]);
  // }
  // assert(!argv[argc]); // always true
  // return 0;
  OpenProcDir("/proc/");
  return 0;
}
