#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  printf("Hello, World!\n");
  // int i;
  // for (i = 0; i < argc; i++) {
  //   assert(argv[i]); // always true
  //   printf("argv[%d] = %s\n", i, argv[i]);
  // }
  // assert(!argv[argc]); // always true
  // return 0;
  DIR *dir;
  struct dirent *ptr;
  dir = opendir("/proc");
  if (dir) {
    while ((ptr = readdir(dir)) != NULL) {
      printf("dir name is:%s \n ", ptr->d_name);
    }
  } else {
    perror("open dir fail\n");
  }

  return 0;
}
