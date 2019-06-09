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

void log_error(char *msg) { perror(msg); }

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
  return write_lock_wait(db->db_file->_fileno) +
         write_lock_wait(db->db_log->_fileno);
}

// success return 0
int kvdb_unlock(kvdb_t *db) {
  return unlock(db->db_file->_fileno) + unlock(db->db_log->_fileno);
}

int kvdb_open_thread_unsafe(kvdb_t *db, const char *filename) {
  char logname[MAX_NAME_LEN];
  sprintf(logname, "%s.log", filename);
  db->db_file = fopen(filename, "a+");
  db->db_file = fopen(logname, "r");

  if (db->db_file == NULL || db->db_log == NULL) {
    log_error("create db or log file error.\n");
    return -1;
  }

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
  assert(db->db_log);
  assert(db->db_file);
  assert(&(db->thread_lock));
  if (!fclose(db->db_file)) {
    log_error("close db file error.\n");
    return -1;
  }
  if (!fclose(db->db_log)) {
    log_error("close db log error.\n");
    return -1;
  }
  if (!pthread_mutex_destroy(&(db->thread_lock))) {
    log_error("destroy db mutex error. \n");
    return -1;
  }
  return 0;
}

int kvdb_put_thread_unsafe(kvdb_t *db, const char *key, const char *value) {
  kvdb_lock(db);
  assert(db);
  assert(db->db_file);
  assert(db->db_log);

  int file_fd = db->db_file->_fileno;
  // int log_fd = db->db_log->_fileno;

  // log begin

  // log commit

  // db write
  size_t key_size = strlen(key);
  size_t val_size = strlen(value);
  if (write(file_fd, &key_size, sizeof(key_size)) != sizeof(key_size)) {
    return -1;
  }
  if (write(file_fd, &val_size, sizeof(val_size)) != sizeof(val_size)) {
    return -1;
  }
  if (write(file_fd, key, strlen(key)) != strlen(key)) {
    return -1;
  }
  if (write(file_fd, value, strlen(value)) != strlen(value)) {
    return -1;
  }
  fsync(file_fd);

  // log

  kvdb_unlock(db);
  return 0;
}

char *kvdb_get_thread_unsafe(kvdb_t *db, const char *key) {
  assert(db);
  assert(db->db_file);

  int file_fd = db->db_file->_fileno;
  size_t key_size = 0;
  size_t val_size = 0;
  char *key_buf, *val_buf;

  if (lseek(file_fd, 0, SEEK_SET) == -1) {
    log_error("error in get_lseek.\n");
    return NULL;
  }

  while (1) {
    ssize_t read_ret = read(file_fd, &key_size, sizeof(key_size));
    if (read_ret == 0) {
      log_error("can not find the key in db. \n");
      return NULL;
    }

    if (read_ret != sizeof(key_size)) {
      log_error("read key size error in get.\n");
      return NULL;
    }

    if (read(file_fd, &val_size, sizeof(val_size)) != sizeof(val_size)) {
      log_error("read val size error in get. \n");
      return NULL;
    }

    key_buf = (char *)malloc(key_size + 1);
    val_buf = (char *)malloc(val_size + 1);

    if (key_buf == NULL) {
      log_error("key buf malloc error in get. \n");
      return NULL;
    }

    if (val_buf == NULL) {
      log_error("val buf malloc error in get. \n");
      free(key_buf);
      return NULL;
    }

    if (read(file_fd, key_buf, key_size) != key_size) {
      log_error("read key error in get. \n");
      free(val_buf);
      free(key_buf);
      return NULL;
    }

    if (read(file_fd, val_buf, val_size) != val_size) {
      log_error("read val error in get. \n");
      free(val_buf);
      free(val_buf);
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

  free(key_buf);
  assert(key_buf == NULL);
  return val_buf;
}