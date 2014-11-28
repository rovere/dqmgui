#ifndef DQM_VISDQMLOCKS_H
#define DQM_VISDQMLOCKS_H

#include <pthread.h>

#include "boost/python.hpp"

using namespace boost;

class PyReleaseInterpreterLock
{
  PyThreadState *save_;
public:
  PyReleaseInterpreterLock(void) { save_ = PyEval_SaveThread(); }
  ~PyReleaseInterpreterLock(void) { PyEval_RestoreThread(save_); }
};

class Lock
{
  pthread_mutex_t *lock_;
public:
  Lock(pthread_mutex_t *lock) { pthread_mutex_lock(lock_ = lock); }
  ~Lock(void)                 { pthread_mutex_unlock(lock_); }
};

class RDLock
{
  pthread_rwlock_t *lock_;
public:
  RDLock(pthread_rwlock_t *lock) { pthread_rwlock_rdlock(lock_ = lock); }
  ~RDLock(void)                  { pthread_rwlock_unlock(lock_); }
};

class WRLock
{
  pthread_rwlock_t *lock_;
public:
  WRLock(pthread_rwlock_t *lock) { pthread_rwlock_wrlock(lock_ = lock); }
  ~WRLock(void)                  { pthread_rwlock_unlock(lock_); }
};


#endif // DQM_VISDQMLOCKS_H
