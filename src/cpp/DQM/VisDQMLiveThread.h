#ifndef DQM_VISDQMLIVETHREAD_H
#define DQM_VISDQMLIVETHREAD_H

#include <string>
#include <set>

#include <pthread.h>

#include "classlib/iobase/Socket.h"

#include "DQM/DQMNet.h"
#include "DQM/VisDQMSourceObjects.h"

class VisDQMSource;
struct VisDQMEventNum;
struct VisDQMRegexp;
namespace lat {class Regexp;}

/** A background thread assisting #VisDQMLiveSource.  Listens to
    changes in live DQM data in upstream sources and maintains a
    up-to-date summary relevant for the web service. */
class VisDQMLiveThread : public DQMBasicNet
{
  VisDQMSource		*owner_;
  VisDQMItems		items_;
  pthread_rwlock_t	itemlock_;
  pthread_cond_t	reqrecv_;
  std::string		streamers_;

public:
  VisDQMLiveThread(VisDQMSource *owner, bool verbose,
                   const std::string &host, int port);

  void stop(void)
  {
    shutdown();
  }

  virtual bool onMessage(Bucket *msg, Peer *p, unsigned char *data, size_t len);
  void fetch(const std::string &name, std::string &streamers, Object &retobj);
  void scan(VisDQMItems &result, VisDQMEventNum &current,
            VisDQMRegexp *rxmatch, lat::Regexp *rxsearch, bool *alarm);

  void
      json(const std::string &rootpath,
           bool fulldata,
           bool /* lumisect */,
           std::string &result,
           double &stamp,
           const std::string &mename,
           std::set<std::string> &dirs);

protected:
  // Release an object from wait: wake up fetch() side of this class.
  // Here we need to issue wake up only if the object is null; valid
  // objects are handled in sendObjectToPeer().
  virtual void
      releaseFromWait(Bucket *msg, WaitObject &w, Object *o)
  {
    DQMBasicNet::releaseFromWait(msg, w, o);
    if (! o) pthread_cond_broadcast(&reqrecv_);
  }

  // Handle notification to send an object to downstream client. The
  // only client we can have is actually ourselves: the fetch(). Wake
  // up the receiver side to inform it the object has arrived.
  virtual void
      sendObjectToPeer(Bucket *msg, DQMNet::Object &o, bool data)
  {
    DQMBasicNet::sendObjectToPeer(msg, o, data);
    pthread_cond_broadcast(&reqrecv_);
  }

  // Handle peer removal.  Wake up the receiver side since we may have
  // lost any chance of receiving anything any more.
  virtual void
      removePeer(Peer *p, lat::Socket *s)
  {
    DQMBasicNet::removePeer(p, s);
    pthread_cond_broadcast(&reqrecv_);
  }
};

#endif  // DQM_VISDQMLIVETHREAD_H
