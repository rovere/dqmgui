#include "DQM/VisDQMArchiveWatch.h"


bool VisDQMArchiveWatch::running(void) {
  return thread_ != (pthread_t) -1;
}

void VisDQMArchiveWatch::stop(void) {
  pthread_mutex_lock(&lock_);
  stop_ = true;
  pthread_cond_signal(&stopsig_);
  pthread_mutex_unlock(&lock_);
  pthread_join(thread_, 0);
  thread_ = (pthread_t) -1;
}

bool VisDQMArchiveWatch::changed(void) {
  pthread_mutex_lock(&lock_);
  bool changed = changed_;
  pthread_mutex_unlock(&lock_);
  return changed;
}

void VisDQMArchiveWatch::clear(void) {
  pthread_mutex_lock(&lock_);
  changed_ = false;
  pthread_mutex_unlock(&lock_);
}

void * VisDQMArchiveWatch::run(void *arg) {
  return ((VisDQMArchiveWatch *)arg)->dorun();
}

void * VisDQMArchiveWatch::VisDQMArchiveWatch::dorun(void) {
  pthread_mutex_lock(&lock_);
  while (!stop_) {
    timespec delay;
#if _POSIX_TIMERS > 0
    clock_gettime(CLOCK_REALTIME, &delay);
#else
    timeval tv;
    gettimeofday(&tv, 0);
    delay.tv_sec = tv.tv_sec;
    delay.tv_nsec = tv.tv_usec * 1000;
#endif
    delay.tv_sec += 30;
    pthread_cond_timedwait(&stopsig_, &lock_, &delay);
    if (!stop_)
      checkForUpdate();
  }

  pthread_mutex_unlock(&lock_);
  return 0;
}

void VisDQMArchiveWatch::checkForUpdate(void) {
  IOStatus st;
  Filename gen(path_, "generation");
  if (st.fill(gen.name()) && st.m_mtime != mtime_) {
    changed_ = true;
    mtime_ = st.m_mtime;
  }
}
