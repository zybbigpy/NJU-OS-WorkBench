#include "kvdb.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int kvdb_open_thread_unsafe(kvdb_t *db, const char *filename);
int kvdb_close_thread_unsafe(kvdb_t *db);
int kvdb_put_thread_unsafe(kvdb_t *db, const char *key, const char *value);
char *kvdb_get_thread_unsafe(kvdb_t *db, const char *key);
int kvdb_lock(kvdb_t *db);
int kvdb_unlock(kvdb_t *db);
int kvdb_check(kvdb_t *db);
int kvdb_recover(kvdb_t *db);

void log_error(char *msg) { perror(msg); }
void log_debug(char *msg) { perror(msg); }

// thread safe kvdb open
int kvdb_open(kvdb_t *db, const char *filename) {
  pthread_mutex_lock(&db->thread_lock);
  int ret = kvdb_open_thread_unsafe(db, filename);
  pthread_mutex_unlock(&db->thread_lock);
  return ret;
}

// thread safe kvdb close
int kvdb_close(kvdb_t *db) {
  pthread_mutex_lock(&db->thread_lock);
  int ret = kvdb_close_thread_unsafe(db);
  pthread_mutex_unlock(&db->thread_lock);
  return ret;
}

// thread safe kvdb put
int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  pthread_mutex_lock(&db->thread_lock);
  int ret = kvdb_put_thread_unsafe(db, key, value);
  pthread_mutex_unlock(&db->thread_lock);
  return ret;
}

// thread safe kvdb get
char *kvdb_get(kvdb_t *db, const char *key) {
  pthread_mutex_lock(&db->thread_lock);
  char *ret = kvdb_get_thread_unsafe(db, key);
  pthread_mutex_unlock(&db->thread_lock);
  return ret;
}

// block read lock
int read_lock_wait(int fd) {
  struct flock lock;
  lock.l_len = 0;
  lock.l_pid = getpid();
  lock.l_start = 0;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;

  return fcntl(fd, F_SETLKW, &lock);
}

// block write lock
int write_lock_wait(int fd) {
  struct flock lock;
  lock.l_len = 0;
  lock.l_pid = getpid();
  lock.l_start = 0;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;

  return fcntl(fd, F_SETLKW, &lock);
}

// unlock
int unlock(int fd) {
  struct flock lock;
  lock.l_len = 0;
  lock.l_pid = getpid();
  lock.l_start = 0;
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;

  return fcntl(fd, F_SETLKW, &lock);
}

// success return 0
int kvdb_lock(kvdb_t *db) {
  return write_lock_wait(db->file_fd) + write_lock_wait(db->log_fd);
}

// success return 0
int kvdb_unlock(kvdb_t *db) { return unlock(db->file_fd) + unlock(db->log_fd); }

int kvdb_open_thread_unsafe(kvdb_t *db, const char *filename) {
  char logname[MAX_NAME_LEN];
  sprintf(logname, "%s.log", filename);
  db->file_fd = open(filename, O_CREAT | O_RDWR, 0644);
  db->log_fd = open(logname, O_CREAT | O_RDWR, 0644);

  if (!kvdb_lock(db)) {
    log_error("lock db error.\n");
    return -1;
  }

  if (!kvdb_check(db)) {
    log_error("check db error\n");
    kvdb_unlock(db);
    return -1;
  }

  if (!kvdb_unlock(db)) {
    log_error("unlock db error.\n");
    return -1;
  }

  return 0;
}

int kvdb_close_thread_unsafe(kvdb_t *db) {
  assert(db);
  assert(&(db->thread_lock));
  if (!close(db->file_fd)) {
    log_error("close db file error.\n");
    return -1;
  }
  if (!close(db->log_fd)) {
    log_error("close db log error.\n");
    return -1;
  }
  if (!pthread_mutex_destroy(&(db->thread_lock))) {
    log_error("destroy db mutex error. \n");
    return -1;
  }
  return 0;
}

#define LOG_EPT 0
#define LOG_BGN 1
#define LOG_COMMIT 2
#define LOG_FREE 3

int kvdb_set_log(int log_fd, int flag) {
  lseek(log_fd, 0, SEEK_SET);
  if (write(log_fd, &flag, sizeof(flag)) != sizeof(flag)) {
    return -1;
  }
  return 0;
}

int kvdb_put_thread_unsafe(kvdb_t *db, const char *key, const char *value) {
  kvdb_lock(db);
  assert(db);

  int file_fd = db->file_fd;
  int log_fd = db->log_fd;

  size_t key_size = strlen(key);
  size_t val_size = strlen(value);

  // log begin
  if (kvdb_set_log(log_fd, LOG_BGN) != 0) {
    log_error("log bgn write error.\n");
    kvdb_unlock(db);
    return -1;
  }
  fsync(log_fd);

  // do logging
  int offset = lseek(file_fd, 0, SEEK_END);
  if (offset == -1) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(log_fd, &offset, sizeof(offset)) != sizeof(offset)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(log_fd, &key_size, sizeof(key_size)) != sizeof(key_size)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(log_fd, &val_size, sizeof(val_size)) != sizeof(val_size)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(log_fd, key, strlen(key)) != strlen(key)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(log_fd, value, strlen(value)) != strlen(value)) {
    kvdb_unlock(db);
    return -1;
  }
  fsync(log_fd);

  // log commit
  if (kvdb_set_log(log_fd, LOG_COMMIT) != 0) {
    log_error("log commit write error.\n");
    kvdb_unlock(db);
    return -1;
  }
  fsync(log_fd);

  // db write
  lseek(file_fd, 0, SEEK_END);

  if (write(file_fd, &key_size, sizeof(key_size)) != sizeof(key_size)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(file_fd, &val_size, sizeof(val_size)) != sizeof(val_size)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(file_fd, key, strlen(key)) != strlen(key)) {
    kvdb_unlock(db);
    return -1;
  }
  if (write(file_fd, value, strlen(value)) != strlen(value)) {
    kvdb_unlock(db);
    return -1;
  }
  fsync(file_fd);

  // log free
  if (kvdb_set_log(log_fd, LOG_FREE) != 0) {
    log_error("log free write error.\n");
    kvdb_unlock(db);
    return -1;
  }
  fsync(log_fd);

  kvdb_unlock(db);
  return 0;
}

char *kvdb_get_thread_unsafe(kvdb_t *db, const char *key) {
  assert(db);

  kvdb_lock(db);

  int file_fd = db->file_fd;
  size_t key_size = 0;
  size_t val_size = 0;
  char *key_buf = NULL;
  char *val_buf = NULL;

  if (lseek(file_fd, 0, SEEK_SET) == -1) {
    log_error("error in get_lseek.\n");
    kvdb_unlock(db);
    return NULL;
  }

  while (1) {
    ssize_t read_ret = read(file_fd, &key_size, sizeof(key_size));
    if (read_ret == 0) {
      log_error("can not find the key in db. \n");
      kvdb_unlock(db);
      return NULL;
    }

    if (read_ret != sizeof(key_size)) {
      log_error("read key size error in get.\n");
      kvdb_unlock(db);
      return NULL;
    }

    if (read(file_fd, &val_size, sizeof(val_size)) != sizeof(val_size)) {
      log_error("read val size error in get. \n");
      kvdb_unlock(db);
      return NULL;
    }

    assert(key_buf == NULL);
    assert(val_buf == NULL);
    key_buf = (char *)malloc(key_size + 1);
    val_buf = (char *)malloc(val_size + 1);
    assert(key_buf);
    assert(val_buf);

    if (read(file_fd, key_buf, key_size) != key_size) {
      log_error("read key error in get. \n");
      free(val_buf);
      free(key_buf);
      kvdb_unlock(db);
      return NULL;
    }

    if (read(file_fd, val_buf, val_size) != val_size) {
      log_error("read val error in get. \n");
      free(val_buf);
      free(val_buf);
      kvdb_unlock(db);
      return NULL;
    }

    if (strcmp(key, key_buf) == 0) {
      val_buf[val_size] = '\0';
      break;
    } else {
      free(val_buf);
      free(key_buf);
    }
  }

  assert(key_buf != NULL);
  free(key_buf);
  kvdb_unlock(db);
  return val_buf;
}

int kvdb_check(kvdb_t *db) {
  assert(db);

  int log_fd = db->log_fd;
  if (lseek(log_fd, 0, SEEK_SET) == -1) {
    return -1;
  }

  int flag = -1;
  flag = read(log_fd, &flag, sizeof(flag));

  if (flag == LOG_BGN) {
    log_error("lose last record.\n");
  }
  if (flag == LOG_COMMIT) {
    log_debug("begin recover the db.\n");
    if (kvdb_recover(db) != 0) {
      log_error("error in db recovery. \n");
      return -1;
    }
  }
  return 0;
}

int kvdb_recover(kvdb_t *db) {
  assert(db);

  int log_fd = db->file_fd;
  int file_fd = db->log_fd;

  int flag, offset;
  int key_size, val_size;
  char *buf;

  // get the last commit info
  lseek(log_fd, 0, SEEK_SET);
  read(log_fd, &flag, sizeof(flag));
  read(log_fd, &offset, sizeof(offset));
  read(log_fd, &key_size, sizeof(key_size));
  read(log_fd, &val_size, sizeof(val_size));
  buf = (char *)malloc(val_size + key_size);
  read(log_fd, buf, key_size + val_size);

  // recover the last commit
  lseek(file_fd, offset, SEEK_SET);
  write(file_fd, &key_size, sizeof(val_size));
  write(file_fd, &val_size, sizeof(val_size));
  write(file_fd, buf, key_size + val_size);
  fsync(file_fd);

  kvdb_set_log(log_fd, LOG_FREE);

  free(buf);
  return 0;
}