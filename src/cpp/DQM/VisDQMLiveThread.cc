#include "DQM/VisDQMLiveThread.h"

#include <vector>
#include <set>

#include "DQM/DQMNet.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMLocks.h"
#include "DQM/StringAtom.h"
// We do not want to import ROOT stuff in here.
# define VISDQM_NO_ROOT 1
#include "DQM/VisDQMTools.h"

#include "classlib/utils/Regexp.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/Time.h"

#include "boost/shared_ptr.hpp"

extern void buildParentNames(StringAtomList & /* to */, const StringAtom & /* from */);
extern StringAtomTree stree;

using namespace lat;

/** A background thread assisting #VisDQMLiveSource.  Listens to
    changes in live DQM data in upstream sources and maintains a
    up-to-date summary relevant for the web service. */
VisDQMLiveThread::VisDQMLiveThread(VisDQMSource *owner, bool verbose,
                                   const std::string &host, int port)
    : DQMBasicNet("VisDQMLiveThread"),
      owner_(owner)
{
  // FIXME: from client: buildStreamerInfo(streamers_);
  pthread_rwlock_init(&itemlock_, 0);
  pthread_cond_init(&reqrecv_, 0);
  logme()
      << "INFO: DQM live thread started"
      << ", listening for data from "
      << host << ":" << port << "\n";

  debug(verbose);
  delay(500);
  listenToCollector(host, port);
  start();
}


bool VisDQMLiveThread::onMessage(Bucket *msg, Peer *p,
                                 unsigned char *data, size_t len)
{
  // Let base class handle the actual message.
  bool ret = DQMBasicNet::onMessage(msg, p, data, len);

  // If it was end of full list update, refresh local version.
  // This helps us keep items locked the minimum possible time.
  if (! ret || len != 4*sizeof(uint32_t))
    return ret;

  uint32_t words[4];
  memcpy(words, data, sizeof(words));
  if (words[1] != DQM_REPLY_LIST_END || ! words[3])
    return ret;

  // Get a write lock on the items.
  WRLock gate(&itemlock_);
  std::string path;
  PeerMap::iterator pi, pe;
  ObjectMap::iterator oi, oe;
  VisDQMItems::iterator di, de;

  // First mark all replica objects no longer known.
  for (di = items_.begin(), de = items_.end(); di != de; ++di)
    di->second->flags |= DQM_PROP_DEAD;

  // Scan still known objects, and update replica if necessary.
  for (pi = peers_.begin(), pe = peers_.end(); pi != pe; ++pi)
    for (oi = pi->second.objs.begin(), oe = pi->second.objs.end(); oi != oe; ++oi)
    {
      Object &o = const_cast<Object &>(*oi);
      path.clear();
      path += *o.dirname;
      if (! path.empty())
        path += '/';
      path += o.objname;

      StringAtom name(&stree, path);
      boost::shared_ptr<VisDQMItem> &i = items_[name];
      if (! i)
      {
        i.reset(new VisDQMItem);
        i->flags = VisDQMIndex::SUMMARY_PROP_TYPE_INVALID;
        i->version = 0;
        i->name = name;
        i->plotter = owner_;
        buildParentNames(i->parents, i->name);
      }

      if (o.flags != i->flags)
      {
        o.flags &= ~DQM_PROP_NEW;
        i->flags = o.flags;
        i->version = o.version;
        if ((i->flags & DQM_PROP_TYPE_MASK) <= DQM_PROP_TYPE_SCALAR)
          i->data = o.scalar;
      }
    }

  // Finally scan replicas again to remove old dead objects.
  for (di = items_.begin(), de = items_.end(); di != de; )
    if (di->second->flags & DQM_PROP_DEAD)
      items_.erase(di++);
    else
      ++di;

  // Done, return message processing status.
  return ret;
}

void
VisDQMLiveThread::fetch(const std::string &name,
                        std::string &streamers, Object &retobj)
{
  streamers = streamers_; // FIXME: from peer;

  // If the object is already there and is not stale, return immediately.
  lock();
  if (Object *o = findObject(0, name))
  {
    o->lastreq = Time::current().ns();
    retobj = *o;
  }
  unlock();

  if ((retobj.rawdata.size() || retobj.scalar.size())
      && ! (retobj.flags & DQM_PROP_STALE))
    return;

  // Build an internal object request.
  DataBlob req(3*sizeof(uint32_t) + name.size());
  uint32_t header[3] = { (uint32_t)req.size(),
                         DQM_MSG_GET_OBJECT,
                         (uint32_t)name.size() };
  memcpy(&req[0], header, sizeof(header));
  memcpy(&req[sizeof(header)], &name[0], name.size());

  // Build a fake request/reply structure with a phatom peer.
  Bucket reply;
  reply.next = 0;

  Peer phantom;
  phantom.peeraddr = "(internal request)";
  phantom.socket = 0;
  phantom.sendq = 0;
  phantom.sendpos = 0;
  phantom.mask = 0;
  phantom.source = false;
  phantom.update = false;
  phantom.updated = false;
  phantom.updates = 0;
  phantom.waiting = 0;
  phantom.automatic = 0;

  // Issue the fake object request as if it was from a real peer,
  // and then wait for the response.  If we get the response right
  // here and now, queue it to the phantom responding peer.  Note
  // that we use the base class lock here for everything, including
  // as the condition variable mutex, to not miss notifications.
  lock();
  onMessage(&reply, &phantom, &req[0], req.size());
  if (! reply.data.empty())
  {
    Bucket **prev = &phantom.sendq;
    while (*prev)
      prev = &(*prev)->next;

    *prev = new Bucket;
    (*prev)->next = 0;
    (*prev)->data.swap(reply.data);
  }

  // Wait for the response to arrive.  Note that we use the base
  // class lock as the mutex for the condition variable, so we
  // avoid missing notifications and will exit this loop with the
  // lock held in order for discard() and findObject() not begin
  // until releaseFromWait() and sendObjectToPeer() have completed.
  // Do not go to sleep if/after we have lost all live peers.
  while (! phantom.sendq && peers_.size())
    pthread_cond_wait(&reqrecv_, &lock_);

  // Got the object.  We don't need the reply packet since
  // we also know the object by now (if it exists).
  discard(phantom.sendq);
  if (Object *o = findObject(0, name))
    retobj = *o;
  unlock();
}

void VisDQMLiveThread::scan(VisDQMItems &result, VisDQMEventNum &current,
                            VisDQMRegexp *rxmatch, Regexp *rxsearch,
                            bool *alarm)
{
  RDLock gate(&itemlock_);
  VisDQMEventNumList eventnums;
  VisDQMItems::iterator di, de;

  // Scan known objects.  Match remaining valid objects against the
  // caller's filters and add matching ones to the result set.
  result.resize(result.size() + items_.size());
  for (di = items_.begin(), de = items_.end(); di != de; ++di)
  {
    VisDQMItem &i = *di->second;
    if (isIntegerType(i.flags) && i.name.string().find("/EventInfo/i") != std::string::npos)
      getEventInfoNum(i.name.string(), i.data.c_str(), eventnums);
    else if (isRealType(i.flags) && i.name.string().find("/EventInfo/ru") != std::string::npos)
      getEventInfoNum(i.name.string(), i.data.c_str(), eventnums);

    if (! fastmatch(rxmatch, i.name)
        || (rxsearch && rxsearch->search(i.name.string()) < 0)
        || (alarm && ((i.flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) != 0) != (*alarm == true)))
      continue;

    result[i.name] = di->second;
  }

  // Pick greatest run/lumi/event number combo seen.
  setEventInfoNums(eventnums, current);
}

void VisDQMLiveThread::json(const std::string &rootpath,
                            bool fulldata,
                            bool /* lumisect */,
                            std::string &result,
                            double &stamp,
                            const std::string &mename,
                            std::set<std::string> &dirs)
{
  std::vector<DQMNet::Object> objs;
  PeerMap::iterator pi, pe;
  ObjectMap::iterator oi, oe;
  std::set<std::string>::iterator di, de;
  std::vector<DQMNet::Object>::iterator ni, ne;

  // Stuff streamers first, in case it is missing.
  if (result.find("streamerinfo") == std::string::npos)
    result += StringFormat("%1{ \"streamerinfo\": \"%2\" }\n")
        .arg(result.empty() ? "" : ", ")
        .arg(fulldata ? hexlify(streamers_) : std::string());


  // Scan objects and subdirectories under rootpath.
  lock();
  for (pi = peers_.begin(), pe = peers_.end(); pi != pe; ++pi)
  {
    objs.reserve(objs.size() + pi->second.objs.size());
    for (oi = pi->second.objs.begin(), oe = pi->second.objs.end(); oi != oe; ++oi)
    {
      Object &o = const_cast<Object &>(*oi);
      if (rootpath == *o.dirname)
      {
        // Copy 'o' to result. We need to copy the directory name
        // pointer too because it is unsafe to dereference the
        // pointer once we release lock on the peers.
        const std::string *dir = &*dirs.insert(*o.dirname).first;
        // If we filter by ME name, get rid of this ME in case we
        // have no match
        if (!mename.empty() && mename != o.objname)
          continue;
        objs.push_back(o);
        objs.back().dirname = dir;
      }
      else if (isSubdirectory(rootpath, *o.dirname))
      {
        size_t begin = (rootpath.empty() ? 0 : rootpath.size()+1);
        size_t slash = o.dirname->find('/', begin);
        dirs.insert(std::string(*o.dirname, begin, slash - begin));
      }
    }
  }
  unlock();

  // Format sub-directories, only in case we were not
  // explicitely asked for a single MonitorElement: in this
  // case omit directory listing.
  if (mename.empty())
    for (di = dirs.begin(), de = dirs.end(); di != de; ++di)
      result += StringFormat(", { \"subdir\": %1 }\n").arg(stringToJSON(*di));

  // Format objects to json, with full data if requested.
  double nulllim[3][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };
  double nullstat[3] = { 0, 0, 0 };
  uint32_t nullbins[3] = { 0, 0, 0 };
  DQMNet::QReports qreports;
  DQMNet::Object o;
  std::string junk;
  std::string path;
  std::string qstr;

  result.reserve(result.size() + objs.size() * 1000);
  for (ni = objs.begin(), ne = objs.end(); ni != ne; ++ni)
  {
    o.rawdata.clear();
    if (fulldata)
    {
      path.clear();
      path.reserve(ni->dirname->size() + ni->objname.size() + 1);
      path += *ni->dirname;
      if (! path.empty())
        path += '/';
      path += ni->objname;
      fetch(path, junk, o);
    }
    stamp = std::max(stamp, ni->version * 1e-9);
    objectToJSON(ni->objname,
                 ni->scalar.c_str(),
                 ni->qdata.c_str(),
                 o.rawdata,
                 0,
                 ni->flags,
                 ni->tag,
                 0,
                 nullbins,
                 nullstat,
                 nullstat,
                 nulllim,
                 qreports,
                 qstr,
                 result);
  }
}
