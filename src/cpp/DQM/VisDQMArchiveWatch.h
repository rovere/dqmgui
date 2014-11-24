#ifndef DQM_VISDQMARCHIVEWATCH_H
#define DQM_VISDQMARCHIVEWATCH_H

#include <pthread.h>

#include "classlib/utils/TimeInfo.h"
#include "classlib/iobase/Filename.h"
#include "classlib/iobase/IOStatus.h"

using namespace lat;

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

  bool running(void);
  void stop(void);
  bool changed(void);
  void clear(void);

 private:
  static void * run(void *arg);
  void * dorun(void);
  void checkForUpdate(void);
};

#endif // DQM_VISDQMARCHIVEWATCH_H
