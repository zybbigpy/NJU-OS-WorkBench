#include "kvdb.h"

#include <assert.h>
#include <fcntl.h>
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
  char* ret = kvdb_get_thread_unsafe(db, key);
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

int kvdb_lock(kvdb_t *db) {
  return write_lock_wait(db->db_file->_fileno) +
         write_lock_wait(db->db_log->_fileno);
}

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




  kvdb_unlock(db);
  return 0;
}