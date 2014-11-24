#ifndef DQM_LOCKS_H
#define  DQM_LOCKS_H

class Lock {
  pthread_mutex_t *lock_;
 public:
  explicit Lock(pthread_mutex_t *lock) { pthread_mutex_lock(lock_ = lock); }
  ~Lock(void)                 { pthread_mutex_unlock(lock_); }
};

class RDLock {
  pthread_rwlock_t *lock_;
 public:
  explicit RDLock(pthread_rwlock_t *lock) {
    pthread_rwlock_rdlock(lock_ = lock);
  }
  ~RDLock(void) {
    pthread_rwlock_unlock(lock_);
  }
};

class WRLock {
  pthread_rwlock_t *lock_;
 public:
  explicit WRLock(pthread_rwlock_t *lock) {
    pthread_rwlock_wrlock(lock_ = lock);
  }
  ~WRLock(void) {
    pthread_rwlock_unlock(lock_);
  }
};

#endif // DQM_LOCKS_H
