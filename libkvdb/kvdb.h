#ifndef __KVDB_H__
#define __KVDB_H__

#include <stdio.h>
#include <pthread.h>

#define MAX_NAME_LEN 1024

struct kvdb {
    int file_fd;
    int log_fd;
    char name[1024];
    pthread_mutex_t thread_lock;
};

typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif
