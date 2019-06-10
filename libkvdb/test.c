#include "kvdb.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

int main() {
  kvdb_t db;

  kvdb_open(&db, "good.db");
  kvdb_put(&db, "nice", "os");
  kvdb_put(&db, "os", "a-mazing-course");
  char* value1 = kvdb_get(&db, "nice");
  printf("key is %s, and the value is %s.\n", "nice", value1);
  char* value2 = kvdb_get(&db, "os");
  printf("key is %s, and the value is %s.\n", "os", value2);

  int fd = (&db)->log_fd;
  lseek(fd, 0, SEEK_SET);
  free(value1);
  free(value2);
  //   int pid = fork();
  //   if (pid == 0) {
  //     kvdb_put(&db, "os", "an-amazing-course");
  //     return 0;
  //   }

  //   kvdb_put(&db, "nice", "os");
  //   char* value1 = kvdb_get(&db, "nice");
  //   printf("key is %s, and the value is %s.\n", "nice", value1);
  //   char* value2 = kvdb_get(&db, "os");
  //   printf("key is %s, and the value is %s.\n", "os", value2);
  //   free(value1);
  //   free(value2);
  kvdb_close(&db);
  return 0;
}