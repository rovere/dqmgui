#ifndef DQM_VISDQMARCHIVEWATCH_H
#define DQM_VISDQMARCHIVEWATCH_H

#include "classlib/iobase/Filename.h"
#include "classlib/iobase/IOStatus.h"
#include "classlib/utils/Time.h"

#include <pthread.h>

/** A background thread assisting #VisDQMArchiveSource.  Prompts
    the source to revalidate its index data every so often. */
class VisDQMArchiveWatch
{
  pthread_mutex_t	lock_;
  pthread_cond_t	stopsig_;
  pthread_t		thread_;
  Filename		path_;
  bool			stop_;
  bool			changed_;
  Time			mtime_;

public:
  VisDQMArchiveWatch(const Filename &path)
    : thread_((pthread_t) -1),
      path_(path),
      stop_(false),
      changed_(false),
      mtime_(0)
    {
      pthread_mutex_init(&lock_, 0);
      pthread_cond_init(&stopsig_, 0);
      checkForUpdate();
      pthread_create(&thread_, 0, &run, this);
    }

  bool
  running(void)
    {
      return thread_ != (pthread_t) -1;
    }

  void
  stop(void)
    {
      pthread_mutex_lock(&lock_);
      stop_ = true;
      pthread_cond_signal(&stopsig_);
      pthread_mutex_unlock(&lock_);
      pthread_join(thread_, 0);
      thread_ = (pthread_t) -1;
    }

  bool
  changed(void)
    {
      pthread_mutex_lock(&lock_);
      bool changed = changed_;
      pthread_mutex_unlock(&lock_);
      return changed;
    }

  void
  clear(void)
    {
      pthread_mutex_lock(&lock_);
      changed_ = false;
      pthread_mutex_unlock(&lock_);
    }

private:
  static void *
  run(void *arg)
    { return ((VisDQMArchiveWatch *)arg)->dorun(); }

  void *
  dorun(void)
    {
      pthread_mutex_lock(&lock_);
      while (! stop_)
      {
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
	if (! stop_)
	  checkForUpdate();
      }

      pthread_mutex_unlock(&lock_);
      return 0;
    }

  void
  checkForUpdate(void)
    {
      IOStatus st;
      Filename gen(path_, "generation");
      if (st.fill(gen.name()) && st.m_mtime != mtime_)
      {
	changed_ = true;
	mtime_ = st.m_mtime;
      }
    }
};

#endif  // DQM_VISDQMARCHIVEWATCH_H
