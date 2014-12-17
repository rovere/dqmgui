// Shut up warning about _POSIX_C_SOURCE mismatch, caused by a
// harmless conflict between _GNU_SOURCE and python header files.
//#define NDEBUG 1
#include <unistd.h>
#if _POSIX_C_SOURCE != 200112L
# undef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
#endif
#define VISDQM_NO_ROOT 1
#define DEBUG(n,x)

#include "DQM/Objects.h"
#include "DQM/NatSort.h"
#include "DQM/StringAtom.h"
#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMFile.h"
#include "DQM/VisDQMCache.h"
#include "DQM/VisDQMTools.h"
#include "DQM/DQMNet.h"
#include "DQM/VisDQMRegexp.h"
#include "DQM/VisDQMDrawOptions.h"
#include "DQM/VisDQMEventNum.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMLocks.h"

#include "classlib/utils/Argz.h"
#include "classlib/utils/DebugAids.h"
#include "classlib/utils/TimeInfo.h"
#include "classlib/utils/Signal.h"
#include "classlib/utils/Regexp.h"
#include "classlib/utils/RegexpMatch.h"
#include "classlib/utils/StringOps.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/iobase/SubProcess.h"
#include "classlib/iobase/LocalSocket.h"
#include "classlib/iobase/Filename.h"
#include "classlib/iobase/IOStatus.h"
#include "classlib/iobase/File.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

#include <set>
#include <cfloat>
#include <fstream>
#include <stdexcept>
#include <dlfcn.h>
#include <math.h>
#include <inttypes.h>

#ifndef __GLIBC__
#include <stdio.h>
// Use unique buffer for read and write.
struct membuf
{
  char *data;
  char **addr;
  size_t *len;
  size_t size;
  size_t pos;
};

int fmemread(void *cookie, char *into, int len)
{
  membuf *mbuf = (membuf *) cookie;
  if (mbuf->pos > mbuf->size) return 0;
  size_t nmax = std::min<size_t>(len, mbuf->size - mbuf->pos);
  memcpy(into, mbuf->data + mbuf->pos, nmax);
  mbuf->pos += nmax;
  return nmax;
}

int fmemwrite(void *cookie, const char *from, int len)
{
  /* Copy content from from into cookie's internal buf for
     len bytes. Allocate space in case it is needed. Keeps
     allocating it just to the exact boundary.  Maybe a bulk
     allocation could be more performing? Keep track of the
     new allocated memory and save it back into the pointer
     passed to open_memstream. */

  membuf *mbuf = (membuf *) cookie;
  if (mbuf->pos + len > mbuf->size)
  {
    if (! (mbuf->data = (char*)realloc((void *)mbuf->data,
				       mbuf->pos + len - mbuf->size )))
      return 0;
    *mbuf->addr = mbuf->data;
    mbuf->size  = mbuf->pos + len;
  }
  memcpy (&(mbuf->data[mbuf->pos]), from, len);
  mbuf->pos += len;
  *mbuf->len=mbuf->pos;
  return len;
}

fpos_t fmemseek(void *cookie, fpos_t pos, int whence)
{
  membuf *mbuf = (membuf *) cookie;
  if (whence == SEEK_SET)
    mbuf->pos = pos;
  else if (whence == SEEK_END)
    mbuf->pos = mbuf->size;
  else if (whence == SEEK_CUR)
    mbuf->pos += pos;
  return mbuf->pos;
}

int fmemclose(void *cookie)
{
  /* Demand deletion of allocated memory to the calling
     function!! We basically delete the cookie but not
     the allocated memory.  The pointer to the allocated
     memory is passed back to the calling function in
     write mode which is responsible to delete it. This
     is to keep the same logic used with the open_memstream
     function under linux. */
  free(cookie);
  return 0;
}

FILE *fmemopen(char *buf, size_t n, const char *)
{
  membuf *mbuf = (membuf*)malloc(sizeof(membuf));
  mbuf->data = buf;
  mbuf->size = n;
  mbuf->pos = 0;

  return funopen(mbuf, fmemread, 0, fmemseek, fmemclose);
}

FILE *open_memstream(char **buf, size_t *len, const char * = 0)
{
  membuf *mbuf = (membuf*)malloc(sizeof(membuf));
  mbuf->data = (char*)malloc(1024*100); //100K default
  mbuf->addr = buf;
  *mbuf->addr = mbuf->data;
  mbuf->size=1024*100;
  mbuf->pos = 0;
  mbuf->len = len;
  *mbuf->len = mbuf->pos;
  return funopen(mbuf, 0, fmemwrite, fmemseek, fmemclose);
}
#endif



using namespace lat;
using namespace boost;
namespace ext = __gnu_cxx;
namespace py = boost::python;
typedef std::map<StringAtom, StringAtomList> StringAtomParentList;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma GCC visibility push(hidden)
pthread_rwlock_t parentslock;
StringAtomParentList parents;
StringAtomTree stree(1024*1024);

void
buildParentNames(StringAtomList &to, const StringAtom &from)
{
  pthread_rwlock_rdlock(&parentslock);
  StringAtomParentList::iterator pos = parents.find(from);
  if (pos == parents.end())
  {
    pthread_rwlock_unlock(&parentslock);
    pthread_rwlock_wrlock(&parentslock);
    pos = parents.find(from);
  }

  if (pos == parents.end())
  {
    std::string parentstr;
    const std::string &fromstr = from.string();
    to.reserve(std::count(fromstr.begin(), fromstr.end(), '/')+1);
    parentstr.reserve(fromstr.size());

    for (size_t slash = 0; slash < fromstr.size();
         slash = fromstr.find('/', ++slash))
    {
      parentstr.assign(fromstr, 0, slash);
      to.push_back(StringAtom(&stree, parentstr));
    }
    parents[from] = to;
  }
  else
    to = pos->second;

  pthread_rwlock_unlock(&parentslock);
}
#pragma GCC visibility pop

//<<EOF1 begin of Sources
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include "DQM/VisDQMToJSON.h"
#include "DQM/VisDQMRenderLink.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#include "DQM/VisDQMUnknownSource.h"
#include "DQM/VisDQMOverlaySource.h"
#include "DQM/VisDQMStripChartSource.h"
#include "DQM/VisDQMCertificationSource.h"
#include "DQM/VisDQMLayoutSource.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include "DQM/VisDQMLiveThread.h"

class VisDQMLiveSource : public VisDQMSource
{
  std::string mydataset_;
  VisDQMLiveThread *thread_;
  VisDQMRenderLink *link_;

public:
  VisDQMLiveSource(py::object /* gui */, py::dict opts)
    : mydataset_(py::extract<std::string>(opts["dataset"])),
      thread_(new VisDQMLiveThread(this,
				   py::extract<bool>(opts["verbose"]),
				   py::extract<std::string>(opts["dqmhost"]),
				   py::extract<int>(opts["dqmport"]))),
      link_(VisDQMRenderLink::instance())
    {}

  ~VisDQMLiveSource(void)
    {
      if (thread_)
	exit();
    }

  virtual const char *
  plotter(void) const
    {
      return "live";
    }

  virtual const char *
  jsoner(void) const
    {
      return "underconstruction";
    }

  virtual void
  getdata(const VisDQMSample & /* sample */,
	  const std::string &path,
	  std::string &streamers,
	  DQMNet::Object &obj)
    {
      streamers.clear();
      clearobj(obj);
      thread_->fetch(path, streamers, obj);
    }

  virtual void
  getattr(const VisDQMSample & /* sample */,
	  const std::string &path,
	  VisDQMIndex::Summary &attrs,
	  std::string &xstreamers,
	  DQMNet::Object &xobj)
    {
      clearattr(attrs);
      thread_->fetch(path, xstreamers, xobj);
    }

  py::tuple
  plot(const std::string &path, py::dict opts)
    {
      VisDQMSample sample(SAMPLE_ANY, -1);
      std::map<std::string,std::string> options;
      bool imageok = false;
      std::string imagedata;
      std::string imagetype;
      std::string streamers;
      DQMNet::Object obj;
      copyopts(opts, options);

      {
        PyReleaseInterpreterLock nogil;
        getdata(sample, path, streamers, obj);
	imageok = link_->render(imagedata, imagetype, path, options,
				&streamers, &obj, 1, STDIMGOPTS);
      }

      if (imageok)
	return py::make_tuple(imagetype, imagedata);
      else
	return py::make_tuple(py::object(), py::object());
    }

  void
  exit(void)
    {
      if (thread_)
      {
        thread_->stop();
        delete thread_;
        thread_ = 0;
      }
    }

  virtual void
  prepareSession(py::dict session)
    {
      if (! session.has_key("dqm.sample.type"))
      {
	session["dqm.sample.runnr"] = 0L;
	session["dqm.sample.dataset"] = mydataset_;
	session["dqm.sample.importversion"] = 0;
	session["dqm.sample.type"] = long(SAMPLE_LIVE);
      }
    }

  virtual void
  scan(VisDQMItems &result,
       const VisDQMSample &sample,
       VisDQMEventNum &current,
       VisDQMRegexp *rxmatch,
       Regexp *rxsearch,
       bool *alarm,
       std::string *,
       VisDQMRegexp *)
    {
      if (sample.type == SAMPLE_LIVE)
	thread_->scan(result, current, rxmatch, rxsearch, alarm);
    }

  virtual void
  json(const VisDQMSample &sample,
       const std::string &rootpath,
       bool fulldata,
       bool lumisect,
       std::string &result,
       double &stamp,
       const std::string &name,
       std::set<std::string> &dirs)
    {
      if (sample.type == SAMPLE_LIVE || sample.type == SAMPLE_ANY)
	thread_->json(rootpath, fulldata, lumisect, result, stamp, name, dirs);
    }

  virtual void
  samples(VisDQMSamples &samples)
    {
      samples.push_back(VisDQMSample());
      VisDQMSample &s = samples.back();
      s.type = SAMPLE_LIVE;
      s.runnr = 0;
      s.dataset = mydataset_;
      s.origin = this;
      s.time = Time::current().ns();
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
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

class VisDQMArchiveSource : public VisDQMSource
{
  typedef shared_ptr<VisDQMFile>		VisDQMFilePtr;
  typedef std::vector<size_t>			IdMap;
  typedef std::vector<VisDQMIndex::Sample>	SampleList;
  typedef std::map<uint32_t, VisDQMFilePtr>	FileMap;

  Regexp		rxonline_;
  pthread_rwlock_t	lock_;
  bool			retry_;
  Filename		path_;
  VisDQMArchiveWatch	watch_;
  VisDQMCache		cache_;
  VisDQMIndex		index_;
  SampleList		samples_;
  StringAtomTree	vnames_; // (10000);
  StringAtomTree	dsnames_; // (100000);
  StringAtomTree	objnames_; // (2500000);
  StringAtomTree	streamers_; // (100);
  FileMap		infoFiles_;
  FileMap		dataFiles_;
  VisDQMRenderLink	*link_;

  // Helper utility to close a DQM file before destroying it.
  static void
  closeDQMFile(VisDQMFile *f)
    {
      if (f && f->path())
	f->close();
    }

  // Utility function to reset current data contents.  Must be called
  // with write lock held.  Does not terminate transaction.
  void
  reset(void)
    {
      dataFiles_.clear();
      infoFiles_.clear();
      streamers_.clear();
      objnames_.clear();
      dsnames_.clear();
      vnames_.clear();
      samples_.clear();
      watch_.clear();
      retry_ = false;
    }

  // Load the master catalogue from an index.
  void
  loadIndex(void)
    {
      // FIXME: Try doing the index re-read in the watcher thread
      // and simply swap 'current' values in place here, so we do
      // not become unnecessarily blocked waiting for the I/O while
      // serving HTTP requests.  We do have to block when file
      // open errors have occurred however.
      VisDQMFile *master = 0;
      index_.beginRead(master);
      for (VisDQMFile::ReadHead rdhead(master, IndexKey(0, 0));
	   ! rdhead.isdone(); rdhead.next())
      {
	void *begin;
	void *end;
	IndexKey key;

	rdhead.get(&key, &begin, &end);
	if (key.catalogIndex() == VisDQMIndex::MASTER_SAMPLE_RECORD)
	  samples_.push_back(*(const VisDQMIndex::Sample *)begin);
	else
	  break;
      }

      readStrings(dsnames_, master, IndexKey(0, VisDQMIndex::MASTER_DATASET_NAME));
      readStrings(vnames_, master, IndexKey(0, VisDQMIndex::MASTER_CMSSW_VERSION));
      readStrings(objnames_, master, IndexKey(0, VisDQMIndex::MASTER_OBJECT_NAME));
      readStrings(streamers_, master, IndexKey(0, VisDQMIndex::MASTER_TSTREAMERINFO));
      index_.finishRead();
    }

  // Report a file read failure.  Returns true if reading should
  // be abandoned: more than 10 tries have failed to open a file.
  bool
  fileReadFailure(int ntries, const char *message)
    {
      bool abandon = (ntries < 0 || ntries > 10);
      loginfo()
	<< (ntries >= 0 ? ntries : 1)
	<< " transient error" << (ntries > 1 ? "s" : "")
	<< " while reading data, " << (abandon ? "giving up" : "retrying")
	<< "; error was: " << message << std::endl;
      pthread_rwlock_wrlock(&lock_);
      retry_ = true;
      pthread_rwlock_unlock(&lock_);
      return abandon;
    }

  // Prepare to read from the index. Reloads the index if there have
  // been past file read errors, or if the index has changed.
  void
  maybeReloadIndex(void)
    {
      // First determine if we should retry index open.
      pthread_rwlock_rdlock(&lock_);
      bool retry = retry_;
      pthread_rwlock_unlock(&lock_);

      // If we've seen file read errors, force index refresh.  But
      // avoid storming-heard several thread reload by checking
      // once again after we obtain the exclusive write lock.
      if (retry)
      {
	WRLock wrgate(&lock_);
	if (retry_)
	{
	  reset();
	  loadIndex();
	}
      }

      // If the index changed we should reload, so do so now.
      // Again avoid storming herd repeated updates.  Note that if
      // the previous block updated index, it cleared the watcher.
      if (watch_.changed())
      {
	WRLock wrgate(&lock_);
	if (watch_.changed())
	{
	  reset();
	  loadIndex();
	}
      }
    }

  // Open a possibly cached data file.  This must be called with a
  // read lock on "this" held; it will upgrade the lock to a write
  // lock if that is necessary to open the file.
  VisDQMFilePtr
  open(uint16_t kind, const uint32_t *file)
    {
      FileMap &files = (kind == VisDQMIndex::MASTER_FILE_INFO
			? infoFiles_ : dataFiles_);
      FileMap::iterator pos = files.find(file[kind]);
      if (pos == files.end())
      {
	pthread_rwlock_unlock(&lock_);
	pthread_rwlock_wrlock(&lock_);
	pos = files.find(file[kind]);
      }

      if (pos == files.end())
      {
	VisDQMFilePtr ptr(index_.open(kind,
				      file[kind] >> 16,
				      file[kind] & 0xffff,
				      VisDQMFile::OPEN_READ),
			  &closeDQMFile);
	files.insert(FileMap::value_type(file[kind], ptr));
	return ptr;
      }
      else
	return pos->second;
    }

  SampleList::iterator
  findSample(const VisDQMSample &sample)
    {
      SampleList::iterator i;
      SampleList::iterator e;
      for (i = samples_.begin(), e = samples_.end(); i != e; ++i)
	if (i->numObjects > 0
	    && i->runNumber == sample.runnr
	    && sample.dataset == dsnames_.key(i->datasetNameIdx))
	  break;

      return i;
    }

public:
  VisDQMArchiveSource(py::object /* gui */, py::dict opts)
    : rxonline_(py::extract<std::string>(opts["rxonline"])),
      retry_(false),
      path_(Filename(py::extract<std::string>(opts["index"]))),
      watch_(path_),
      cache_(300*1024*1024),
      index_(path_, &cache_),
      vnames_(10000),
      dsnames_(100000),
      objnames_(2500000),
      streamers_(100),
      link_(VisDQMRenderLink::instance())
    {
      rxonline_.study();
      pthread_rwlock_init(&lock_, 0);
    }

  ~VisDQMArchiveSource(void)
    {
      if (watch_.running())
	watch_.stop();
    }

  virtual const char *
  plotter(void) const
    {
      return "archive";
    }

  virtual const char *
  jsoner(void) const
    {
      return "archive";
    }

  void
  exit(void)
    {
      if (watch_.running())
	watch_.stop();
    }

  static inline void makeBinLabel(char * buff, uint64_t value)
    {
      sprintf(buff, "%" PRIu64, value);
    }

  virtual void
  getdata(const VisDQMSample &sample,
	  const std::string &path,
	  std::string &streamers,
	  DQMNet::Object &obj)
    {
      streamers.clear();
      clearobj(obj);

      // Keep retrying until we can successfully read the data.
      for (int ntries = 1; true; ++ntries)
      {
	try
	{
	  // Reload index if necessary.
	  maybeReloadIndex();

	  // Locate the requested sample.
	  RDLock rdgate(&lock_);
	  SampleList::const_iterator si = findSample(sample);
	  uint32_t objindex = StringAtom(&objnames_, path, StringAtom::TestOnly).index();

	  // Check if we found what we are looking for.  If yes, try
	  // opening the data file and return the streamed data for
	  // the requested object; if this fails we'll end up in a
	  // catch statement which will loop around.
	  if (si != samples_.end() && objindex != 0)
	  {
	    // FIXME: handle keys other than run summary?
	    IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, objindex);
	    IndexKey key;
	    void *begin;
	    void *end;

            {
	      VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
	      VisDQMFile::ReadHead rdhead(file.get(), keyidx);
	      if (! rdhead.isdone())
	      {
		rdhead.get(&key, &begin, &end);
		if (key == keyidx)
		{
		  VisDQMIndex::Summary *s = (VisDQMIndex::Summary *)begin;
	          const char *data = (s->dataLength ? (const char *) (s+1) : 0);
	          const char *qdata = (s->qtestLength ? (const char *) (s+1) + s->dataLength : 0);
	          obj.flags = s->properties;
		  obj.tag = s->tag;
                  obj.version = si->lastImportTime;
		  if (data)
		    obj.scalar.append(data, s->dataLength-1);
		  if (qdata)
		    obj.qdata.append(qdata, s->qtestLength);
		  streamers = streamers_.key(si->streamerInfoIdx);
	        }
	      }
	    }

	    if (isROOTType(obj.flags))
	    {
	      VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_DATA, si->files));
	      VisDQMFile::ReadHead rdhead(file.get(), keyidx);
	      if (! rdhead.isdone())
	      {
		rdhead.get(&key, &begin, &end);
		if (key == keyidx && begin != end)
		  obj.rawdata.insert(obj.rawdata.end(),
				     (const unsigned char *)begin,
				     (const unsigned char *)end);
	      }
	    }
	  }

	  // Not found, return null data.
	  break;
	}
	catch (Error &e)
	{
	  if (fileReadFailure(ntries, e.explain().c_str()))
	    break;
	}
	catch (std::exception &e)
	{
	  if (fileReadFailure(ntries, e.what()))
	    break;
	}
	catch (...)
	{
	  if (fileReadFailure(ntries, "(unknown error)"))
	    break;
	}
      }
    }

  virtual void
  getattr(const VisDQMSample &sample,
	  const std::string &path,
	  VisDQMIndex::Summary &attrs,
	  std::string & /* xstreamers */,
	  DQMNet::Object & /* xobj */)
    {
      clearattr(attrs);

      // Keep retrying until we can successfully read the data.
      for (int ntries = 1; true; ++ntries)
      {
	try
	{
	  // Reload index if necessary.
	  maybeReloadIndex();

	  // Locate the requested sample.
	  RDLock rdgate(&lock_);
	  SampleList::const_iterator si = findSample(sample);
	  uint32_t objindex = StringAtom(&objnames_, path, StringAtom::TestOnly).index();

	  // Check if we found what we are looking for.  If yes, try
	  // opening the data file and return the streamed data for
	  // the requested object; if this fails we'll end up in a
	  // catch statement which will loop around.
	  if (si != samples_.end() && objindex != 0)
	  {
	    // FIXME: handle keys other than run summary?
	    IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, objindex);
	    IndexKey key;
	    void *begin;
	    void *end;

	    VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
	    VisDQMFile::ReadHead rdhead(file.get(), keyidx);
	    if (! rdhead.isdone())
	    {
	      rdhead.get(&key, &begin, &end);
	      if (key == keyidx)
		attrs = *(VisDQMIndex::Summary *)begin;
	    }
	  }

	  // No error, return.
	  break;
	}
	catch (Error &e)
	{
	  if (fileReadFailure(ntries, e.explain().c_str()))
	    break;
	}
	catch (std::exception &e)
	{
	  if (fileReadFailure(ntries, e.what()))
	    break;
	}
	catch (...)
	{
	  if (fileReadFailure(ntries, "(unknown error)"))
	    break;
	}
      }
    }

  py::str
  getJson(const int runnr,
          const std::string &dataset,
          const std::string &path,
          py::dict opts)
  {
    VisDQMSample sample(SAMPLE_ANY, runnr, dataset);
    std::map<std::string,std::string> options;
    bool jsonok = false;
    std::string jsonData;
    std::string imagetype;
    std::string streamers;
    DQMNet::Object obj;
    copyopts(opts, options);

    {
      PyReleaseInterpreterLock nogil;
      getdata(sample, path, streamers, obj);
      jsonok = link_->prepareJson(jsonData, imagetype, path, options,
                                  &streamers, &obj, 1, STDIMGOPTS);
    }
    if(jsonok) {
      py::str _str(jsonData);
      return _str;
    }
    return "\"error\":\"JSON preparing process was interrupted.\"";
  }

  py::tuple
  plot(int runnr,
       const std::string &dataset,
       const std::string &path,
       py::dict opts)
    {
      VisDQMSample sample(SAMPLE_ANY, runnr, dataset);
      std::map<std::string,std::string> options;
      bool imageok = false;
      std::string imagedata;
      std::string imagetype;
      std::string streamers;
      DQMNet::Object obj;
      copyopts(opts, options);

      {
        PyReleaseInterpreterLock nogil;
        getdata(sample, path, streamers, obj);
	imageok = link_->render(imagedata, imagetype, path, options,
				&streamers, &obj, 1, STDIMGOPTS);
      }

      if (imageok)
	return py::make_tuple(imagetype, imagedata);
      else
	return py::make_tuple(py::object(), py::object());
    }

  // Refresh data from the database for a run.  Checks if the master
  // database exists, and if so, if it has changed relative to the
  // last copy and the time we last read in the data.  If so, discards
  // any existing cached data and copies the file again.  Then checks
  // to see if the data for the requested run is already cached, and
  // if not, reads it in.  Also discards cached data for runs that
  // have not been accessed recently, where "recent" is MAX_CACHE_LIFE
  // seconds.
  void
  update(VisDQMItems &items,
	 const VisDQMSample &sample,
	 VisDQMEventNum &current,
	 VisDQMRegexp *rxmatch,
	 Regexp *rxsearch,
	 bool *alarm)
    {
      // No point in even trying unless this is archived data.
      if (sample.type < SAMPLE_ONLINE_DATA || sample.type >= SAMPLE_ANY)
	return;

      // Keep retrying until we can successfully read the data.
      for (int ntries = 1; true; ++ntries)
      {
	try
	{
	  // Reload index if necessary.
	  maybeReloadIndex();

	  // Locate the requested sample.
	  RDLock rdgate(&lock_);
	  SampleList::const_iterator si = findSample(sample);

	  // Return with empty "items" if we didn't find the sample.
	  if (si == samples_.end())
	    return;

	  // Found the sample, try opening the data file.  If this
	  // fails we will end up in the catch statement which will
	  // loop around to retry a certain number of times.  The
	  // stack unwinding automatically handles lock release.
	  VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_INFO, si->files));

	  // Read the summary for the requested sample.
	  VisDQMEventNumList eventnums;
	  IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, 0);
	  for (VisDQMFile::ReadHead rdhead(file.get(), keyidx);
	       ! rdhead.isdone(); rdhead.next())
	  {
	    IndexKey key;
	    void *begin;
	    void *end;
	    rdhead.get(&key, &begin, &end);
	    // FIXME: handle keys other than run summary?
	    uint64_t keyparts[4] = { key.sampleidx(),
				     key.type(),
				     key.lumiend(),
				     key.objnameidx() };
	    if (keyparts[0] != keyidx.sampleidx() || keyparts[1] != 0)
	      break;

	    VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
	    uint32_t report = s->properties & VisDQMIndex::SUMMARY_PROP_REPORT_MASK;
	    const char *data = (s->dataLength ? (const char *) (s+1) : 0);
	    const std::string &name = objnames_.key(keyparts[3]);
	    StringAtom sname(&stree, name);

	    // Update run number information.
	    if (isIntegerType(s->properties) && name.find("/EventInfo/i") != std::string::npos)
	      getEventInfoNum(name, data, eventnums);
	    else if (isRealType(s->properties) && name.find("/EventInfo/ru") != std::string::npos)
	      getEventInfoNum(name, data, eventnums);

	    // If it doesn't match search criteria, skip it.
	    if (! fastmatch(rxmatch, sname)
		|| (rxsearch && rxsearch->search(name) < 0)
		|| (alarm && (report != 0) != (*alarm == true)))
	      continue;

	    // We want to keep it, construct an item for use.
	    shared_ptr<VisDQMItem> i(new VisDQMItem);
	    i->flags = s->properties;
	    i->version = si->lastImportTime;
	    i->name = sname;
	    i->plotter = this;
	    buildParentNames(i->parents, i->name);
	    if (s->dataLength)
	      i->data = data;

	    items[i->name] = i;
	  }

	  // Pick greatest run/lumi/event number combo seen.
	  setEventInfoNums(eventnums, current);

	  // Successfully read the data, return.
	  return;
	}
	catch (Error &e)
	{
	  if (fileReadFailure(ntries, e.explain().c_str()))
	    return;
	}
	catch (std::exception &e)
	{
	  if (fileReadFailure(ntries, e.what()))
	    return;
	}
	catch (...)
	{
	  if (fileReadFailure(ntries, "(unknown error)"))
	    return;
	}
      }
    }

  // Initialise a new session.  Select the most recent sample.
  virtual void
  prepareSession(py::dict session)
    {
      if (! session.has_key("dqm.sample.type"))
      {
	bool found = false;
	long runnr = -1;
	uint32_t importversion = 0;
	std::string dataset;
	VisDQMSampleType type = SAMPLE_ANY;

	{
	  PyReleaseInterpreterLock nogil;

	  // Make sure we are up to date.
	  VisDQMItems items;
	  VisDQMSample sample(SAMPLE_ONLINE_DATA, 0);
	  VisDQMEventNum current = { "", -1, -1, -1, -1 };
	  update(items, sample, current, 0, 0, 0);

	  // Scan samples for the most recent one.
	  RDLock rdgate(&lock_);
	  SampleList::const_iterator newest;
	  SampleList::const_iterator si;
	  SampleList::const_iterator se;
	  for (newest = si = samples_.begin(), se = samples_.end(); si != se; ++si)
	    if (si->runNumber > newest->runNumber)
	      newest = si;
	    else if (si->runNumber == newest->runNumber
		     && si->lastImportTime > newest->lastImportTime)
	      newest = si;

	  if (newest != se)
	  {
	    found = true;
	    runnr = newest->runNumber;
	    dataset = dsnames_.key(newest->datasetNameIdx);
	    importversion = newest->importVersion;
	    type = (newest->cmsswVersion > 0 ? SAMPLE_OFFLINE_RELVAL
		    : newest->runNumber == 1 ? SAMPLE_OFFLINE_MC
		    : rxonline_.search(dataset) < 0
		    ? SAMPLE_OFFLINE_DATA : SAMPLE_ONLINE_DATA);
	  }
	}

	if (found)
	{
	  session["dqm.sample.runnr"] = runnr;
	  session["dqm.sample.dataset"] = dataset;
	  session["dqm.sample.importversion"] = importversion;
	  session["dqm.sample.type"] = long(type);
	}
      }
    }

  // Extract data from this source.  Work is done by update().
  virtual void
  scan(VisDQMItems &result,
       const VisDQMSample &sample,
       VisDQMEventNum &current,
       VisDQMRegexp *rxmatch,
       Regexp *rxsearch,
       bool *alarm,
       std::string *,
       VisDQMRegexp *)
    {
      update(result, sample, current, rxmatch, rxsearch, alarm);
    }

  virtual void
  json(const VisDQMSample &sample,
       const std::string &rootpath,
       bool fulldata,
       bool lumisect,
       std::string &result,
       double &stamp,
       const std::string &mename,
       std::set<std::string> &dirs)
    {
      // No point in even trying unless this is archived data.
      if (sample.type < SAMPLE_ONLINE_DATA)
	return;

      // Keep retrying until we can successfully read the data.
      for (int ntries = 1; true; ++ntries)
      {
	try
	{
	  // Reload index if necessary.
	  maybeReloadIndex();

	  // Locate the requested sample.
	  RDLock rdgate(&lock_);
	  SampleList::const_iterator si = findSample(sample);

	  // Return with empty "items" if we didn't find the sample.
	  if (si == samples_.end())
	    return;

	  // Stamp as latest import time.
	  stamp = std::max(stamp, si->processedTime * 1e-9);

	  // Stuff streamers first, in case it is missing.
	  if (result.find("streamerinfo") == std::string::npos)
	    result += StringFormat("%1{ \"streamerinfo\":\"%2\" }\n")
		      .arg(result.empty() ? "" : ", ")
		      .arg(fulldata ? hexlify(streamers_.key(si->streamerInfoIdx)) : std::string());

	  // Found the sample, try opening the data file.  If this
	  // fails we will end up in the catch statement which will
	  // loop around to retry a certain number of times.  The
	  // stack unwinding automatically handles lock release.
	  VisDQMFilePtr info(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
	  VisDQMFilePtr data(open(VisDQMIndex::MASTER_FILE_DATA, si->files));
	  std::set<std::string>::iterator di, de;
	  DQMNet::DataBlob rawdata;
	  DQMNet::QReports qreports;
	  std::string qstr;
	  std::string name;
	  std::string dir;

	  // Read the summary for the requested sample.
	  IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, 0);
	  VisDQMFile::ReadHead rdinfo(info.get(), keyidx);
	  for ( ; ! rdinfo.isdone(); rdinfo.next())
	  {
	    IndexKey ikey;
	    IndexKey dkey;
	    void *begin;
	    void *end;
	    rdinfo.get(&ikey, &begin, &end);
	    uint64_t keyparts[4] = { ikey.sampleidx(),
				     ikey.type(),
				     ikey.lumiend(),
				     ikey.objnameidx() };
	    if (keyparts[0] != keyidx.sampleidx() || (!lumisect && (keyparts[1] != 0)))
	      break;

	    VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
	    const std::string &path = objnames_.key(keyparts[3]);
	    name.clear();
	    dir.clear();
	    splitPath(dir, name, path);

	    if (rootpath == dir)
	      /* Keep object directly in rootpath directory */;
	    else if (isSubdirectory(rootpath, dir))
	    {
	      // Object in subdirectory, remember subdirectory part
	      size_t begin = (rootpath.empty() ? 0 : rootpath.size()+1);
	      size_t slash = dir.find('/', begin);
	      dirs.insert(std::string(dir, begin, slash - begin));
	      continue;
	    }
	    else
	      // Object outside directory of interest to us, skip
	      continue;

	    // If we filter by ME name, get rid of this ME in case we
	    // have no match
	    if (!mename.empty() && mename != name)
	      continue;

	    // Get data for this object.
	    if (fulldata)
	    {
	      rawdata.clear();
	      VisDQMFile::ReadHead rddata(data.get(), ikey);
	      if (! rddata.isdone())
	      {
		rddata.get(&dkey, &begin, &end);
		if (dkey == ikey)
		  rawdata.insert(rawdata.end(),
				 (unsigned char *) begin,
				 (unsigned char *) end);
	      }
	    }

	    const char *data = (s->dataLength ? (const char *) (s+1) : "");
	    const char *qdata
	      = (s->qtestLength ? ((const char *) (s+1) + s->dataLength) : "");

	    objectToJSON(name,
			 data,
			 qdata,
			 rawdata,
			 keyparts[1] == 0 ? 0 : keyparts[2],
			 s->properties,
			 s->tag,
			 s->nentries,
			 s->nbins,
			 s->mean,
			 s->rms,
			 s->bounds,
			 qreports,
			 qstr,
			 result);
	  }

	  // Format sub-directories, only in case we were not
	  // explicitely asked for a single MonitorElement: in this
	  // case omit directory listing.
	  if (mename.empty())
	    for (di = dirs.begin(), de = dirs.end(); di != de; ++di)
	      result += StringFormat(", { \"subdir\": %1 }\n").arg(stringToJSON(*di));

	  // Successfully read the data, return.
	  return;
	}
	catch (Error &e)
	{
	  if (fileReadFailure(ntries, e.explain().c_str()))
	    return;
	}
	catch (std::exception &e)
	{
	  if (fileReadFailure(ntries, e.what()))
	    return;
	}
	catch (...)
	{
	  if (fileReadFailure(ntries, "(unknown error)"))
	    return;
	}
      }
    }

  virtual void
  getcert(VisDQMSample &sample,
	  const std::string &rootpath,
          const std::string &variableName,
	  std::vector<VisDQMIndex::Summary> &attrs,
          std::vector<double> &axisvals,
	  std::string &binlabels)
    {
      attrs.reserve(4000);
      axisvals.reserve(4000);
      char tmpbuf[64];
      // No point in even trying unless this is archived data.
      if (sample.type < SAMPLE_ONLINE_DATA)
	return;

      // Keep retrying until we can successfully read the data.
      for (int ntries = 1; true; ++ntries)
      {
	try
	{
	  // Reload index if necessary.
	  maybeReloadIndex();

	  // Locate the requested sample.
	  RDLock rdgate(&lock_);
	  SampleList::const_iterator si = findSample(sample);

	  // Return with empty "items" if we didn't find the sample.
	  if (si == samples_.end())
	    return;

	  // Found the sample, try opening the info file.  If this
	  // fails we will end up in the catch statement which will
	  // loop around to retry a certain number of times.  The
	  // stack unwinding automatically handles lock release.
	  VisDQMFilePtr info(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
	  std::string name;
	  std::string dir;
	  std::string path;

	  // Read the summary for the requested sample.
	  IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, 0);
	  VisDQMFile::ReadHead rdinfo(info.get(), keyidx);
	  for ( ; ! rdinfo.isdone(); rdinfo.next())
	  {
	    IndexKey ikey;
	    void *begin;
	    void *end;
	    rdinfo.get(&ikey, &begin, &end);
	    uint64_t keyparts[4] = { ikey.sampleidx(),
				     ikey.type(),
				     ikey.lumiend(),
				     ikey.objnameidx() };
	    if (keyparts[0] != keyidx.sampleidx())
	      break;
	    if (keyparts[1] == 0)
	      continue;

	    VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
	    const std::string &path = objnames_.key(keyparts[3]);
	    name.clear();
	    dir.clear();
	    splitPath(dir, name, path);

	    // Skip object outside directory of interest to us.
	    if (rootpath != dir)
	      continue;

	    // Select only the proper variable, in case one was
	    // supplied.
	    if (!variableName.empty() && variableName != name)
	      continue;

	    uint32_t type = s->properties & DQMNet::DQM_PROP_TYPE_MASK;
	    // Expose only scalar data: there is no point in making a
	    // trend plot for TH* objects.
	    if (type == DQMNet::DQM_PROP_TYPE_INT || type == DQMNet::DQM_PROP_TYPE_REAL)
	    {
	      // Get data for this object.
	      const char *data = (s->dataLength ? (const char *) (s+1) : "");
	      double t = atof(data);
	      // Check if the last lumisection used is equal to the
	      // current one decremented by 1 unit. If this is not the
	      // case, fill the gap with values -2 (to differentiate
	      // between default -1), until we reach the current
	      // lumisection.
	      if (attrs.size() > 0)
		while (attrs.back().mean[0] < (double)(keyparts[2]-1))
		{
		  VisDQMIndex::Summary tmp = attrs.back();
		  tmp.mean[0] += 1.;
		  tmp.mean[1] = -2.;
		  attrs.push_back(tmp);
		  axisvals.push_back(tmp.mean[0]); /* Lumisection number */
		  makeBinLabel(tmpbuf, (uint64_t)tmp.mean[0]);
		  std::string binlabel(tmpbuf);
		  binlabels.append(binlabel.c_str(), binlabel.size()+1);
		}
	      axisvals.push_back((double)keyparts[2]); /* Lumisection number */
	      attrs.push_back(*s);
	      attrs.back().mean[0] = (double)keyparts[2];
	      attrs.back().mean[1] = t;
	      makeBinLabel(tmpbuf, keyparts[2]);
	      std::string binlabel(tmpbuf);
	      binlabels.append(binlabel.c_str(), binlabel.size()+1);
	    }
	  }

	  // Successfully read the data, return.
	  return;
	}
	catch (Error &e)
	{
	  if (fileReadFailure(ntries, e.explain().c_str()))
	    return;
	}
	catch (std::exception &e)
	{
	  if (fileReadFailure(ntries, e.what()))
	    return;
	}
	catch (...)
	{
	  if (fileReadFailure(ntries, "(unknown error)"))
	    return;
	}
      }
    }

  virtual void
  samples(VisDQMSamples &samples)
    {
      try
      {
	// Reload index if necessary.
	maybeReloadIndex();

	// Locate the requested sample.
	RDLock rdgate(&lock_);
	SampleList::const_iterator si;
	SampleList::const_iterator se;
	samples.reserve(samples.size() + samples_.size());
	for (si = samples_.begin(), se = samples_.end(); si != se; ++si)
	{
	  if (! si->numObjects)
	    continue;

	  samples.push_back(VisDQMSample());
	  VisDQMSample &s = samples.back();
	  s.dataset = dsnames_.key(si->datasetNameIdx);
	  s.version = vnames_.key(si->cmsswVersion);
	  s.importversion = si->importVersion;
	  s.runnr = si->runNumber;
	  s.type = (si->cmsswVersion > 0 ? SAMPLE_OFFLINE_RELVAL
		    : si->runNumber == 1 ? SAMPLE_OFFLINE_MC
		    : rxonline_.search(s.dataset) < 0 ? SAMPLE_OFFLINE_DATA
		    : SAMPLE_ONLINE_DATA);
	  s.origin = this;
	  s.time = si->processedTime;
	}
      }
      catch (Error &e)
      {
	fileReadFailure(-1, e.explain().c_str());
      }
      catch (std::exception &e)
      {
	fileReadFailure(-1, e.what());
      }
      catch (...)
      {
	fileReadFailure(-1, "(unknown error)");
      }
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMWorkspace
{
protected:
  py::object	gui_;
  double	guiTimeStamp_;
  std::string	guiServiceName_;
  std::string	guiServiceListJSON_;
  std::string	guiWorkspaceListJSON_;
  std::string	workspaceName_;
  void		(*snapdump_)(const char *);

public:
  VisDQMWorkspace(py::object gui, const std::string &name)
    : gui_(gui),
      guiTimeStamp_(py::extract<double>(gui.attr("stamp"))),
      guiServiceName_(py::extract<std::string>(gui.attr("serviceName"))),
      workspaceName_(name),
      snapdump_(0)
    {
      if (void *sym = dlsym(0, "igprof_dump_now"))
        snapdump_ = (void(*)(const char *)) sym;
    }

  virtual ~VisDQMWorkspace(void)
    {}

  virtual std::string
  state(py::dict /* session */)
    {
      return "{}";
    }

  virtual void
  profilesnap(void)
    {
      if (snapdump_)
      {
        char tofile[256];
        struct timeval tv;
        gettimeofday(&tv, 0);
        if (snprintf(tofile, sizeof(tofile), "|gzip -c>igprof.dqmgui.%ld.%f.gz",
	             (long) getpid(), tv.tv_sec + 1e-6*tv.tv_usec) < int(sizeof(tofile)))
	  snapdump_(tofile);
      }
    }

  // Helper function to compute next available run in current dataset
  int changeRun(py::dict session, bool forward)
    {
      std::vector<VisDQMSource *> srclist;
      std::set<long>		  runlist;
      sources(srclist);

      VisDQMSample sample(sessionSample(session));

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {

	PyReleaseInterpreterLock nogil;
	VisDQMSamples		samples;
	std::set<long>::iterator rli, rle;

	// Request samples from backends.
	for (size_t i = 0, e = srclist.size(); i != e; ++i)
	  srclist[i]->samples(samples);

	// Filter out samples which do not pass search criteria.  In
	// this particular case the fixed criteria are current sample
	// and current dataset.
	for (size_t i = 0, e = samples.size(); i != e; ++i)
	{
	  VisDQMSample &s = samples[i];

	  // Stay on current sample type
	  if (s.type != sample.type)
	    continue;
	  // Stay on current dataset
	  if (s.dataset != sample.dataset)
	    continue;
	  runlist.insert(s.runnr);
	}
      }
	// Now find current run and move according to the prescription
	// received.
	std::set<long>::iterator curr = runlist.find(sample.runnr);
	assert (curr != runlist.end());
	if (forward)
	  return ++curr != runlist.end() ? *curr : *(--curr);
	else
	  return curr == runlist.begin() ? *curr : *(--curr);
    }

protected:
  template <class T> T
  workspaceParam(const py::dict &session, const char *key)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(this->workspaceName_));
    }

  template <class T> T
  workspaceParam(const py::dict &session, const char *key, const T &def)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(this->workspaceName_, def));
    }

  template <class T> T
  workspaceParamOther(const py::dict &session, const char *key, const std::string &wspace)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(wspace));
    }

  template <class T> T
  workspaceParamOther(const py::dict &session, const char *key, const T &def, const std::string &wspace)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(wspace, def));
    }

  // Produce panel configuration format.
  static std::string
  sessionPanelConfig(const py::dict &session, const char *panel)
    {
      char attrname[64];
      bool show;
      int  x;
      int  y;

      snprintf(attrname, sizeof(attrname), "dqm.panel.%.20s.show", panel);
      show = py::extract<bool>(session.get(attrname, false));

      snprintf(attrname, sizeof(attrname), "dqm.panel.%.20s.x", panel);
      x = py::extract<int>(session.get(attrname, -1));

      snprintf(attrname, sizeof(attrname), "dqm.panel.%.20s.y", panel);
      y = py::extract<int>(session.get(attrname, -1));

      return StringFormat("'%1':{'show':%2,'x':%3,'y':%4}")
	.arg(panel).arg(show).arg(x).arg(y);
    }

    // Produce zoom configuration format.
  static std::string
  sessionZoomConfig(const py::dict &session)
    {
      bool show, jsonmode;
      int  x, jx;
      int  y, jy;
      int  w, jw;
      int  h, jh;

      show = py::extract<bool>(session.get("dqm.zoom.show", false));
      x = py::extract<int>(session.get("dqm.zoom.x", -1));
      y = py::extract<int>(session.get("dqm.zoom.y", -1));
      w = py::extract<int>(session.get("dqm.zoom.w", -1));
      h = py::extract<int>(session.get("dqm.zoom.h", -1));
      jsonmode = py::extract<bool>(session.get("dqm.zoom.jsonmode", false));
      jx = py::extract<int>(session.get("dqm.zoom.jx", -1));
      jy = py::extract<int>(session.get("dqm.zoom.jy", -1));
      jw = py::extract<int>(session.get("dqm.zoom.jw", -1));
      jh = py::extract<int>(session.get("dqm.zoom.jh", -1));

      return StringFormat("'zoom':{'show':%1,'x':%2,'y':%3,'w':%4,'h':%5,\
                           'jsonmode':%6,'jx':%7,'jy':%8,'jw':%9,'jh':%10}")
          .arg(show).arg(x).arg(y).arg(w).arg(h)
          .arg(jsonmode).arg(jx).arg(jy).arg(jw).arg(jh);
    }

  // Produce Certification zoom configuration format.
  static std::string
  sessionCertZoomConfig(const py::dict &session)
    {
      bool show;
      int  x;
      int  y;
      int  w;
      int  h;

      show = py::extract<bool>(session.get("dqm.certzoom.show", false));
      x = py::extract<int>(session.get("dqm.certzoom.x", -1));
      y = py::extract<int>(session.get("dqm.certzoom.y", -1));
      w = py::extract<int>(session.get("dqm.certzoom.w", -1));
      h = py::extract<int>(session.get("dqm.certzoom.h", -1));

      return StringFormat("'certzoom':{'show':%1,'x':%2,'y':%3,'w':%4,'h':%5}")
	.arg(show).arg(x).arg(y).arg(w).arg(h);
    }

  static std::string
  sessionReferenceOne(const py::dict &ref)
    {
      return StringFormat("{'type':'%1','run':%2,'dataset':%3, 'ktest':%4}")
	.arg(py::extract<std::string>(ref.get("type"))) // refobj, other, none
	.arg(stringToJSON(py::extract<std::string>(ref.get("run"))))
	.arg(stringToJSON(py::extract<std::string>(ref.get("dataset"))))
	.arg(stringToJSON(py::extract<std::string>(ref.get("ktest"))));
    }

  // Produce a reference description.
  static std::string
  sessionReference(const py::dict &session)
    {
      py::dict refdict = py::extract<py::dict>(session.get("dqm.reference"));
      py::list refspec = py::extract<py::list>(refdict.get("param"));
      std::string ref1(sessionReferenceOne(py::extract<py::dict>(refspec[0])));
      std::string ref2(sessionReferenceOne(py::extract<py::dict>(refspec[1])));
      std::string ref3(sessionReferenceOne(py::extract<py::dict>(refspec[2])));
      std::string ref4(sessionReferenceOne(py::extract<py::dict>(refspec[3])));
      return StringFormat("{'position':'%1', 'show':'%2', \
                            'param':[%3,%4,%5,%6]}")
        .arg(py::extract<std::string>(refdict.get("position")))    // overlay, on-side
	.arg(py::extract<std::string>(refdict.get("show")))        // all, none, custom
	.arg(ref1).arg(ref2).arg(ref3).arg(ref4);
    }

  static std::string
  sessionStripChart(const py::dict &session)
    {
      std::string type = py::extract<std::string>(session.get("dqm.strip.type"));
      std::string omit = py::extract<std::string>(session.get("dqm.strip.omit"));
      std::string axis = py::extract<std::string>(session.get("dqm.strip.axis"));
      std::string n = py::extract<std::string>(session.get("dqm.strip.n"));
      return StringFormat("{'type':'%1', 'omit':'%2', 'axis':'%3','n':%4}")
	.arg(type).arg(omit).arg(axis).arg(stringToJSON(n));
    }


  // Produce a standard response.
  std::string
  makeResponse(const std::string &state,
	       int interval,
	       VisDQMEventNum &current,
	       const std::string &services,
	       const std::string &workspace,
	       const std::string &workspaces,
	       const std::string &submenu,
	       const VisDQMSample &sample,
	       const std::string &filter,
               const std::string &showstats,
               const std::string &showerrbars,
	       const std::string &reference,
	       const std::string &strip,
	       const std::string &rxstr,
	       const std::string &rxerr,
	       size_t rxmatches,
	       const std::string &toolspanel,
	       Time startTime)
    {
      StringFormat result
	= StringFormat("([{'kind':'AutoUpdate', 'interval':%1, 'stamp':%2, 'serverTime':%21},"
		       "{'kind':'DQMHeaderRow', 'run':\"%3\", 'lumi':\"%4\", 'event':\"%5\","
		       " 'runstart':\"%6\", 'service':'%7', 'services':[%8], 'workspace':'%9',"
		       " 'workspaces':[%10], 'view':{'show':'%11','sample': %12, 'filter':'%13',"
		       " 'showstats': %14, 'showerrbars': %15, 'reference':%16, 'strip':%17,"
                       " 'search':%18, %19}},%20])")
	.arg(interval)
	.arg(guiTimeStamp_, 0, 'f')
	.arg(current.runnr < 0 ? std::string("(None)")
	     : current.runnr <= 1 ? std::string("(Simulated)")
	     : thousands(StringFormat("%1").arg(current.runnr)))
	.arg(current.luminr < 0 ? std::string("(None)")
	     : thousands(StringFormat("%1").arg(current.luminr)))
	.arg(current.eventnr < 0 ? std::string("(None)")
	     : thousands(StringFormat("%1").arg(current.eventnr)))
	.arg(formatStartTime(current.runstart))
	.arg(guiServiceName_)
	.arg(services)
	.arg(workspace)
	.arg(workspaces)
	.arg(submenu)
	.arg(sampleToJSON(sample))
	.arg(filter)
        .arg(showstats)
        .arg(showerrbars)
	.arg(reference)
	.arg(strip)
	.arg(StringFormat("{'pattern':%1, 'valid':%2, 'nmatches':%3, 'error':%4}")
	     .arg(stringToJSON(rxstr))
	     .arg(rxerr.empty() ? 1 : 0)
	     .arg(rxmatches)
	     .arg(stringToJSON(rxerr)))
	.arg(toolspanel)
	.arg(state);

      return result.arg((Time::current() - startTime).ns() * 1e-6, 0, 'f');
    }

  // Get the list of native VisDQMSources configured in the GUI.
  void
  sources(std::vector<VisDQMSource *> &into)
    {
      py::list sources = py::extract<py::list>(gui_.attr("sources"));
      py::stl_input_iterator<py::object> i(sources), e;
      for ( ; i != e; ++i)
      {
	py::extract<VisDQMSource *> x(*i);
	if (x.check())
	  into.push_back(x());
      }
    }

  // Build and return a cached JSON representation of the DQM GUI
  // service list.  This must be called when it's safe to access
  // python.  Will build the list just once.
  const std::string &
  serviceListJSON(void)
    {
      if (guiServiceListJSON_.empty())
      {
	StringList parts;
	parts.reserve(100);

	py::list services = py::extract<py::list>(gui_.attr("services"));
	for (py::stl_input_iterator<py::tuple> i(services), e; i != e; ++i)
	  parts.push_back(StringFormat("{'title':'%1', 'href':'%2'}")
			  .arg(py::extract<std::string>((*i)[0]))
			  .arg(py::extract<std::string>((*i)[1])));
	guiServiceListJSON_ = StringOps::join(parts, ", ");
      }

      return guiServiceListJSON_;
    }

  // Build and return a cached JSON representation of the DQM GUI
  // workspace object list.  This must be called when it's safe to
  // access python.  Will build the list just once.
  const std::string &
  workspaceListJSON(void)
    {
      if (guiWorkspaceListJSON_.empty())
      {
	StringList parts;
	parts.reserve(100);
	py::list workspaces = py::extract<py::list>(gui_.attr("workspaces"));
	for (py::stl_input_iterator<py::object> i(workspaces), e; i != e; ++i)
	{
	  int         rank(py::extract<int>(i->attr("rank")));
	  std::string name(py::extract<std::string>(i->attr("name")));
	  std::string category(py::extract<std::string>(i->attr("category")));
	  std::string lower(name);
	  std::transform(lower.begin(), lower.end(), lower.begin(), tolower);
	  parts.push_back(StringFormat("{'title':'%1', 'label':'%2', 'category':'%3', 'rank':%4}")
			  .arg(name).arg(lower).arg(category).arg(rank));
	}
	guiWorkspaceListJSON_ = StringOps::join(parts, ", ");
      }

      return guiWorkspaceListJSON_;
    }

  // Get the contents currently shown in this session.
  void
  buildContents(const std::vector<VisDQMSource *> &sources,
		VisDQMItems &contents,

		StringAtomSet &myobjs,
		const VisDQMSample &sample,
		VisDQMEventNum &current,

		VisDQMRegexp *rxmatch,
		Regexp *rxsearch,
		bool *alarm,

		std::string *layoutroot,
		VisDQMRegexp *rxlayout)
    {
      // Now get filtered contents from all the sources.
      for (size_t i = 0, e = sources.size(); i != e; ++i)
      {
	sources[i]->scan(contents, sample, current,
			 rxmatch, rxsearch, alarm,
			 layoutroot, rxlayout);
      }


      // Add in the quick collection, filtered against rx if there is
      // one, but not the general workspace filter.
      StringAtomSet::iterator mi, me;
      for (mi = myobjs.begin(), me = myobjs.end(); mi != me; ++mi)
      {
	std::string trailer(mi->string(), StringOps::rfind(mi->string(), '/')+1);
	StringAtom label(&stree, "Quick collection/" + trailer);
	if (! rxsearch || rxsearch->search(label.string()) >= 0)
	{
	  shared_ptr<VisDQMItem> i(new VisDQMItem);
	  i->flags = 0;
	  i->version = 0;
	  i->name = label;
	  i->plotter = 0;
	  buildParentNames(i->parents, i->name);

	  VisDQMItems::iterator pos = contents.find(*mi);
	  if (pos != contents.end() && pos->second->layout)
	    i->layout = pos->second->layout;
	  else
	  {
	    shared_ptr<VisDQMLayoutRows> layout(new VisDQMLayoutRows);
	    shared_ptr<VisDQMLayoutRow> row(new VisDQMLayoutRow);
	    shared_ptr<VisDQMLayoutItem> column(new VisDQMLayoutItem);
	    column->path = *mi;
	    layout->push_back(row);
	    row->columns.push_back(column);
	    i->layout = layout;
	  }

	  contents[label] = i;
	}
      }
    }

  // Utility method to build a look-up table of shown objects.
  void
  buildShown(const VisDQMItems &contents,
	     StringAtomSet &shown,
	     const std::string &root)
    {
      std::string myroot;
      if (! root.empty())
      {
	myroot.reserve(root.size() + 2);
	myroot += root;
	myroot += '/';
      }

      VisDQMItems::const_iterator ci, ce;
      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	if (ci->first.string().size() > myroot.size()
	    && ci->first.string().compare(0, myroot.size(), myroot) == 0
	    && ci->first.string().find('/', myroot.size()) == std::string::npos)
	  shown.insert(ci->first);
    }

  // Update the object metrics for the items in @a contents.
  // "Shown" means the object is included in canvas listing.
  void
  updateStatus(const VisDQMItems &contents,
	       VisDQMStatusMap &status)
    {
      VisDQMStatus empty;
      empty.nleaves = 0;
      empty.nalarm = 0;
      empty.nlive = 0;

      status[StringAtom(&stree, "")] = empty;
      status[StringAtom(&stree, "Quick collection")] = empty;

      status.resize(10*contents.size());
      VisDQMItems::const_iterator ci, ce, lpos;
      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
      {
	// Initialise object data.
	VisDQMItem &o = *ci->second;
	VisDQMStatus &s = status[ci->first];
	s.nleaves = 1;
	s.nalarm = ((o.flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) ? 1 : 0);
	s.nlive = 1;

	// If this is a layout, resolve it.
	if (o.layout)
	{
	  VisDQMLayoutRows &rows = *o.layout;
	  for (size_t nrow = 0; nrow < rows.size(); ++nrow)
	  {
	    VisDQMLayoutRow &row = *rows[nrow];
	    for (size_t ncol = 0; ncol < row.columns.size(); ++ncol)
	    {
	      VisDQMLayoutItem &col = *row.columns[ncol];
	      if ((lpos = contents.find(col.path)) != contents.end())
		s.nalarm += ((lpos->second->flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) ? 1 : 0);
	      else if (! col.path.string().empty())
		s.nlive = 0;
	    }
	  }
	}

	// Update data to "virtual parents."
	for (size_t i = 0, e = o.parents.size(); i != e; ++i)
	{
	  VisDQMStatusMap::iterator pos = status.find(o.parents[i]);
	  if (pos == status.end())
	    pos = status.insert(VisDQMStatusMap::value_type(o.parents[i], empty)).first;
	  pos->second.nleaves += s.nleaves;
	  pos->second.nalarm += s.nalarm;
	  pos->second.nlive += s.nlive;
	}
      }
    }

private:
  VisDQMWorkspace(VisDQMWorkspace &);
  VisDQMWorkspace &operator=(VisDQMWorkspace &);
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMSummaryWorkspace : public VisDQMWorkspace
{
  typedef std::map<std::string, std::string> KeyValueMap;
  typedef std::map<std::string, KeyValueMap> SummaryMap;

  Regexp rxevi_;
public:
  VisDQMSummaryWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name),
      rxevi_("(.*?)/EventInfo/(.*)")
    {
      rxevi_.study();
    }

  virtual ~VisDQMSummaryWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample sample(sessionSample(session));
      std::string toolspanel(sessionPanelConfig(session, "tools"));
      std::string qplot(py::extract<std::string>(session.get("dqm.qplot", "")));
      std::string filter(py::extract<std::string>(session.get("dqm.filter")));
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string reference(sessionReference(session));
      std::string strip(sessionStripChart(session));
      std::string submenu(py::extract<std::string>(session.get("dqm.submenu")));
      std::string rxstr(py::extract<std::string>(session.get("dqm.search")));
      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		myobjs; // deliberately empty, not needed
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	RegexpMatch		m;
	std::string		plotter;
	SummaryMap		summary;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), 0, 0, 0);
	updateStatus(contents, status);

	for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  VisDQMItem &o = *ci->second;
	  if (! isScalarType(o.flags))
	    continue;

	  m.reset();
	  if (! rxevi_.match(o.name.string(), 0, 0, &m))
	    continue;

	  std::string system(m.matchString(o.name.string(), 1));
	  std::string param(m.matchString(o.name.string(), 2));

	  if (rxsearch && rxsearch->search(system) < 0)
	    continue;

	  // Alarm filter. Somewhat convoluted as we need to dig out the
	  // report summary value, and use the value known by JavaScript
	  // to check for items passing an alarm filter here.
	  if (qalarm)
	  {
	    StringAtom reportSummary(&stree,
				     system + "/EventInfo/reportSummary",
				     StringAtom::TestOnly);

	    VisDQMItems::iterator pos = contents.find(reportSummary);
	    if (pos != ce && isRealType(pos->second->flags))
	    {
	      double value = atof(pos->second->data.c_str());
	      if ((value >= 0 && value < 0.95) != alarms)
		continue;
	    }
	  }

	  KeyValueMap &info = summary[system];
	  if (info.empty())
	  {
	    plotter = StringFormat("%1/%2%3")
		      .arg(o.plotter ? o.plotter->plotter() : "unknown")
		      .arg(sample.runnr)
		      .arg(sample.dataset);
	    info["MEs"] = StringFormat("%1").arg(status[StringAtom(&stree, system)].nleaves);
	    info["processName"] = system;
	  }

	  info[param] = o.data;
	}

        return makeResponse(StringFormat("{'kind':'DQMSummary', 'items':[%1],"
                            		 " 'qrender':'%2', 'qplot':'%3'}")
			    .arg(summaryToJSON(summary))
			    .arg(plotter)
			    .arg(qplot),
                            sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, summary.size(),
                            toolspanel, startTime);
      }
    }

private:
  static std::string
  summaryToJSON(const SummaryMap &summary)
    {
      size_t len = 4 * summary.size();
      SummaryMap::const_iterator si, se;
      KeyValueMap::const_iterator ki, ke;
      for (si = summary.begin(), se = summary.end(); si != se; ++si)
      {
	len += 8 * si->second.size();
	for (ki = si->second.begin(), ke = si->second.end(); ki != ke; ++ki)
	  len += ki->first.size() + 2 * ki->second.size();
      }

      std::string result;
      result.reserve(len + 1);
      for (si = summary.begin(), se = summary.end(); si != se; ++si)
      {
	if (! result.empty())
	  result += ", ";
	result += "{";

	bool first = true;
	for (ki = si->second.begin(), ke = si->second.end(); ki != ke; ++ki, first = false)
	{
	  if (! first)
	    result += ", ";
	  result += stringToJSON(ki->first);
	  result += ":";
	  result += stringToJSON(ki->second);
	}

	result += "}";
      }

      return result;
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMCertificationWorkspace : public VisDQMWorkspace
{
  typedef std::map<std::string, std::vector<std::string> > KeyVectorMap;

  Regexp rxevi_;
public:
  VisDQMCertificationWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name),
      rxevi_("(.*?)/EventInfo/(CertificationContents|DAQContents|DCSContents|reportSummaryContents)/(.*)")
    {
      rxevi_.study();
    }

  virtual ~VisDQMCertificationWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample sample(sessionSample(session));
      std::string toolspanel(sessionPanelConfig(session, "tools"));
      std::string zoom(sessionCertZoomConfig(session));
      std::string qplot(py::extract<std::string>(session.get("dqm.qplot", "")));
      std::string filter(py::extract<std::string>(session.get("dqm.filter")));
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string reference(sessionReference(session));
      std::string strip(sessionStripChart(session));
      std::string submenu(py::extract<std::string>(session.get("dqm.submenu")));
      std::string rxstr(py::extract<std::string>(session.get("dqm.search")));
      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		myobjs; // deliberately empty, not needed
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	RegexpMatch		m;
	std::string		plotter;
	KeyVectorMap		cert;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), 0, 0, 0);
	updateStatus(contents, status);

	for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  VisDQMItem &o = *ci->second;
	  if (! isScalarType(o.flags))
	    continue;

	  m.reset();
	  if (! rxevi_.match(o.name.string(), 0, 0, &m))
	    continue;

	  std::string system(m.matchString(o.name.string(), 1));
	  std::string kind(m.matchString(o.name.string(), 2));
	  std::string variable(m.matchString(o.name.string(), 3));

	  if (rxsearch && rxsearch->search(system) < 0)
	    continue;

	  std::vector<std::string> &info = cert[system];
	  if (info.empty())
	  {
	    plotter = StringFormat("%1/%2%3")
		      .arg(o.plotter ? o.plotter->plotter() : "unknown")
		      .arg(sample.runnr)
		      .arg(sample.dataset);
	  }
	  info.push_back(kind+"/"+variable);
	}

        return makeResponse(StringFormat("{\"kind\":\"DQMCertification\", \"items\":[%1],"
					 " \"current\":\"%2\", %3}")
			    .arg(certificationToJSON(cert))
			    .arg(plotter)
			    .arg(zoom),
			    sample.type == SAMPLE_LIVE ? 30 : 300, current,
			    services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, cert.size(),
                            toolspanel, startTime);
      }
    }

private:
  static std::string
  certificationToJSON(const KeyVectorMap &cert)
    {
      size_t len = 4 * cert.size();
      KeyVectorMap::const_iterator si, se;
      std::vector<std::string>::const_iterator vi, ve;
      for (si = cert.begin(), se = cert.end(); si != se; ++si)
      {
	len += 8 * si->second.size();
	for (vi = si->second.begin(), ve = si->second.end(); vi != ve; ++vi)
	  len += 2 * vi->size();
      }

      std::string result;
      result.reserve(len + 1);
      bool first = true;
      for (si = cert.begin(), se = cert.end(); si != se; ++si, first = false)
      {
	if (! first)
	    result += ", ";
	result += StringFormat("{ \"text\": \"%1\", \"children\": [")
		  .arg(si->first);
	std::string prevFolder = "";
	for (vi = si->second.begin(), ve = si->second.end(); vi != ve; ++vi)
	{
	  std::string validationFolder = vi->substr(0,vi->rfind('/'));
	  std::string variable = vi->substr(vi->rfind('/')+1, vi->size());
	  if (prevFolder.empty())
	  {
	    prevFolder = validationFolder;
	    result += StringFormat("{ \"text\": \"%1\", \"children\": [{\"text\": \"%2\", \"leaf\": true}")
		      .arg(validationFolder)
		      .arg(variable);
	  }
	  if (prevFolder != validationFolder)
	  {
	    prevFolder = validationFolder;
	    result += StringFormat("]}, { \"text\": \"%1\", \"children\": [{\"text\": \"%2\", \"leaf\": true}")
		      .arg(validationFolder)
		      .arg(variable);
	  }
	  else
	    result += StringFormat(",{\"text\": \"%1\", \"leaf\": true}")
		      .arg(variable);
	}
	result += "]}]}";
      }
      return result ;
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMQualityWorkspace : public VisDQMWorkspace
{
  struct QMapItem
  {
    std::string location;
    std::string name;
    std::string	reportSummary;
    std::string eventTimeStamp;
    uint64_t version;
  };

  typedef std::map<std::string, QMapItem> QMap;
public:
  VisDQMQualityWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMQualityWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample sample(sessionSample(session));
      std::string toolspanel(sessionPanelConfig(session, "tools"));
      std::string filter(py::extract<std::string>(session.get("dqm.filter")));
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string reference(sessionReference(session));
      std::string strip(sessionStripChart(session));
      std::string submenu(py::extract<std::string>(session.get("dqm.submenu")));
      std::string rxstr(py::extract<std::string>(session.get("dqm.search")));
      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		myobjs; // deliberately empty, not needed
	VisDQMItems::iterator	ci, ce;
	std::string		plotter;
	std::string		label;
	QMap			qmap;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), 0, 0, 0);

	for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  VisDQMItem &o = *ci->second;
	  if (o.name.string().size() < 27)
	    continue;

	  size_t pos = o.name.string().size() - 27;
	  if (o.name.string().compare(pos, 27, "/EventInfo/reportSummaryMap") != 0)
	    continue;

	  if (! isScalarType(o.flags))
	  {
	    size_t start = o.name.string().rfind('/', pos ? pos-1 : 0);
	    if (start == std::string::npos) start = 0;
	    label = o.name.string().substr(start, pos-start);
	    if (rxsearch && rxsearch->search(label) < 0)
	      continue;

	    StringAtom reportSummary(&stree,
				     label + "/EventInfo/reportSummary",
				     StringAtom::TestOnly);

	    StringAtom eventTimeStamp(&stree,
				      label + "/EventInfo/eventTimeStamp",
				      StringAtom::TestOnly);

	    // Alarm filter. Somewhat convoluted as we need to dig out the
	    // report summary value, and use the value known by JavaScript
	    // to check for items passing an alarm filter here.
	    if (qalarm)
	    {
	      VisDQMItems::iterator pos = contents.find(reportSummary);
	      if (pos != ce && isRealType(pos->second->flags))
	      {
		double value = atof(pos->second->data.c_str());
		if ((value >= 0 && value < 0.95) != alarms)
		  continue;
	      }
	    }

	    QMapItem &i = qmap[label];
	    i.name = o.name.string();
	    i.version = o.version;
	    i.reportSummary.clear();
	    i.eventTimeStamp.clear();
	    i.location = StringFormat("%1/%2%3")
			 .arg(o.plotter ? o.plotter->plotter() : "unknown")
			 .arg(sample.runnr)
			 .arg(sample.dataset);

	    VisDQMItems::iterator other;
	    if ((other = contents.find(reportSummary)) != ce
		&& isRealType(other->second->flags))
	      i.reportSummary = other->second->data;

	    if ((other = contents.find(eventTimeStamp)) != ce
		&& isRealType(other->second->flags))
	      i.eventTimeStamp = other->second->data;
	  }
	}

        return makeResponse(StringFormat("{'kind':'DQMQuality', 'items':[%1]}")
			    .arg(qmapToJSON(qmap)),
			    sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, qmap.size(),
                            toolspanel, startTime);
      }
    }

private:
  static std::string
  qmapToJSON(const QMap &qmap)
    {
      size_t len = 30 * qmap.size();
      QMap::const_iterator i, e;
      for (i = qmap.begin(), e = qmap.end(); i != e; ++i)
	len += 100 + 2 * (i->first.size()
			  + i->second.location.size()
			  + i->second.name.size()
			  + i->second.reportSummary.size()
			  + i->second.eventTimeStamp.size());

      std::string result;
      result.reserve(len);
      for (i = qmap.begin(), e = qmap.end(); i != e; ++i)
      {
	char buf[32];
	__extension__
	  sprintf(buf, "%ju", (uintmax_t) i->second.version);
	if (! result.empty())
	  result += ", ";
	result += "{'label':";
	result += stringToJSON(i->first);
	result += ",'version':";
	result += buf;
	result += ",'name':";
	result += stringToJSON(i->second.name);
	result += ",'location':";
	result += stringToJSON(i->second.location);
	result += ",'reportSummary':";
	result += stringToJSON(i->second.reportSummary);
	result += ",'eventTimeStamp':";
	result += stringToJSON(i->second.eventTimeStamp);
	result += "}";
      }

      return result;
    }
};


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMSampleWorkspace : public VisDQMWorkspace
{
public:
  VisDQMSampleWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMSampleWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      VisDQMSample sample(sessionSample(session));
      std::string vary(py::extract<std::string>(session.get("dqm.sample.vary")));
      std::string order(py::extract<std::string>(session.get("dqm.sample.order")));
      std::string dynsearch(py::extract<std::string>(session.get("dqm.sample.dynsearch")));
      std::string pat(py::extract<std::string>(session.get("dqm.sample.pattern")));

      // If the current sample is live, force varying off, otherwise
      // it's too hard for users to find anything else than the live.
      if (sample.type == SAMPLE_LIVE)
	vary = "any";

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	typedef std::set<std::string> MatchSet;
	typedef std::list< std::pair<std::string, shared_ptr<Regexp> > > RXList;

	PyReleaseInterpreterLock nogil;
	VisDQMSamples		samples;
	VisDQMSamples		final;
	StringList		pats;
	StringList		runlist;
	RXList			rxlist;
	std::string		rxerr;
	MatchSet		uqpats;
        bool			varyAny = (vary == "any");
	bool			varyRun = (vary == "run");
	bool			varyDataset = (vary == "dataset");
	std::string		samplebase;

	if (sample.type == SAMPLE_OFFLINE_RELVAL)
	  samplebase = StringOps::remove(sample.dataset, RX_CMSSW_VERSION);

	pats = StringOps::split(pat, Regexp::rxwhite, StringOps::TrimEmpty);
	runlist.reserve(pats.size());
	uqpats.insert(pats.begin(), pats.end());

	// Process space-separated patterns.  Split pure numbers to
	// run number criteria, and others to regexp criteria.  The
	// run number requires a prefix match on the number, not an
	// exact match, and we do the prefix comparison as strings.
	for (size_t i = 0, e = pats.size(); i != e; ++i)
	{
	  errno = 0;
	  char *p = 0;
	  strtol(pats[i].c_str(), &p, 10);
	  if (p && ! *p && ! errno)
	    runlist.push_back(pats[i]);

	  shared_ptr<Regexp> rx;
	  makerx(pats[i], rx, rxerr, Regexp::IgnoreCase);
	  if (rx)
	    rxlist.push_back(RXList::value_type(pats[i], rx));
	}

	// Request samples from backends.
	for (size_t i = 0, e = srclist.size(); i != e; ++i)
	  srclist[i]->samples(samples);

	// Filter out samples which do not pass search criteria.
	// Require all pattern components to match at least one
	// criteria (run number or something in the dataset name).
        final.reserve(samples.size());
	for (size_t i = 0, e = samples.size(); i != e; ++i)
	{
	  MatchSet matched;
	  StringList::iterator vi, ve;
	  StringList::iterator rli, rle;
	  RXList::iterator rxi, rxe;
	  VisDQMSample &s = samples[i];

	  if (varyAny)
	    /* keep all */;
          else if (s.type == SAMPLE_LIVE)
	    /* always provide link to live sample */;
	  else if (varyDataset)
	  {
	    if (sample.type == SAMPLE_OFFLINE_RELVAL)
	    {
	      // current sample has no version set, search dataset name.
	      if (s.type != sample.type
		  || sample.dataset.find(s.version) == std::string::npos)
	        continue;
	    }
	    else if (s.runnr != sample.runnr || s.version != sample.version)
	      continue;
          }
          else if (varyRun)
          {
	    if (sample.type == SAMPLE_OFFLINE_RELVAL)
	    {
	      if (s.type != sample.type
		  || samplebase != StringOps::remove(s.dataset, RX_CMSSW_VERSION))
		continue;
	    }
	    else if (s.dataset != sample.dataset)
	      continue;
	  }
	  else
	    continue;

	  for (rli = runlist.begin(), rle = runlist.end(); rli != rle; ++rli)
	  {
	    char buf[32];
	    sprintf(buf, "%ld", s.runnr);
	    if (! strncmp(buf, rli->c_str(), rli->size()))
	      matched.insert(*rli);
	  }

	  for (rxi = rxlist.begin(), rxe = rxlist.end(); rxi != rxe; ++rxi)
	    if (rxi->second->search(s.dataset) >= 0
		|| rxi->second->search(sampleTypeLabel[s.type]) >= 0)
	      matched.insert(rxi->first);

	  if (matched.size() == uqpats.size())
	    final.push_back(samples[i]);
	}

        std::sort(final.begin(), final.end(),
		  (order == "run" ? orderSamplesByRun
		   : orderSamplesByDataset));

	StringFormat result
	  = StringFormat("([{'kind':'AutoUpdate', 'interval':%1, 'stamp':%2, 'serverTime':%9},"
			 "{'kind':'DQMSample', 'vary':%3, 'order':%4, 'dynsearch':%5, 'search':%6, 'current':%7,"
			 " 'items':[%8]}])")
	  .arg(sample.type == SAMPLE_LIVE ? 30 : 300)
	  .arg(guiTimeStamp_, 0, 'f')
	  .arg(stringToJSON(vary))
	  .arg(stringToJSON(order))
	  .arg(stringToJSON(dynsearch))
	  .arg(stringToJSON(pat))
	  .arg(sampleToJSON(sample))
	  .arg(samplesToJSON(final));
	return result.arg((Time::current() - startTime).ns() * 1e-6, 0, 'f');
      }
    }

private:
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMLayoutTools
{
protected:
  static std::string
  axisToJSON(const VisDQMAxisOptions &axis)
    {
      return StringFormat("{'min':%1, 'max':%2, 'type':%3}")
	.arg(stringToJSON(axis.min, true))
	.arg(stringToJSON(axis.max, true))
	.arg(stringToJSON(axis.type));
    }

  static std::string
  layoutToJSON(const VisDQMShownItem &x, const VisDQMSample &sample)
    {
      char vbuf[64]; __extension__ sprintf(vbuf, "%ju", (uintmax_t) x.version);
      return StringFormat("{'name':%1, 'desc':%2, 'location':\"%3\","
			  " 'version':\"%4\", 'alarm':%5, 'live':%6,"
			  " 'xaxis':%7, 'yaxis':%8, 'zaxis':%9,"
			  " 'drawopts':%10, 'withref':%11, 'overlays':%12}")
          .arg(stringToJSON(x.name.string(), true))
          .arg(stringToJSON(x.desc, true))
          .arg(StringFormat("%1/%2%3")
               .arg(x.plotter ? x.plotter->plotter() : "unknown")
               .arg(sample.runnr)
               .arg(sample.dataset))
          .arg(vbuf)
          .arg(x.nalarm)
          .arg(x.nlive)
          .arg(axisToJSON(x.drawopts.xaxis))
          .arg(axisToJSON(x.drawopts.yaxis))
          .arg(axisToJSON(x.drawopts.zaxis))
          .arg(stringToJSON(x.drawopts.drawopts))
          .arg(stringToJSON(x.drawopts.withref))
          .arg(stringsToJSON(x.overlays));
    }

  static std::string
  layoutToJSON(const VisDQMShownRow &layout, const VisDQMSample &sample)
    {
      VisDQMShownItems::const_iterator i, e;
      std::string result;
      result.reserve(2048);
      result += '[';
      for (size_t i = 0, e = layout.columns.size(); i != e; ++i)
      {
	if (i > 0)
	  result += ", ";
	result += layoutToJSON(*layout.columns[i], sample);
      }
      result += ']';
      return result;
    }

  static std::string
  layoutToJSON(const VisDQMShownRows &layout, const VisDQMSample &sample)
    {
      VisDQMShownRows::const_iterator i, e;
      std::string result;
      result.reserve(2048);
      result += '[';
      for (size_t i = 0, e = layout.size(); i != e; ++i)
      {
	if (i > 0)
	  result += ", ";
	result += layoutToJSON(*layout[i], sample);
      }
      result += ']';
      return result;
    }

  static std::string
  shownToJSON(const VisDQMItems &contents,
	      const VisDQMStatusMap &status,
	      const VisDQMDrawOptionMap &drawopts,
	      const StringAtom &path,
	      const VisDQMSample &sample,
	      const StringAtomSet &shown)
    {
      std::string prefix;
      if (! path.string().empty())
      {
	prefix.reserve(path.size() + 2);
	prefix += path.string();
	prefix += '/';
      }

      StringAtomList names;
      StringAtomSet subdirs;
      VisDQMItems::const_iterator ci, ce;
      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
      {
	size_t slash;
	const std::string &name = ci->first.string();
	if (name.size() > prefix.size()
	    && name.compare(0, prefix.size(), prefix) == 0
	    && (slash = name.find('/', prefix.size())) != std::string::npos)
	  subdirs.insert(StringAtom(&stree, std::string(name, 0, slash)));
      }

      std::string result;
      result.reserve(102400);
      result += '[';

      names.insert(names.end(), subdirs.begin(), subdirs.end());
      std::sort(names.begin(), names.end(), natcmpstratom);
      for (size_t i = 0, e = names.size(); i < e; ++i)
      {
	if (result.size() > 1)
	  result += ", ";

	result += shownDirToJSON(contents, status, names[i]);
      }

      names.clear();
      names.insert(names.end(), shown.begin(), shown.end());
      std::sort(names.begin(), names.end(), natcmpstratom);
      for (size_t i = 0, e = names.size(); i < e; ++i)
      {
	if (result.size() > 1)
	  result += ", ";
	result += shownObjToJSON(contents, status, drawopts, sample, names[i]);
      }

      result += ']';
      return result;
    }

  static std::string
  shownDirToJSON(const VisDQMItems & /* contents */,
		 const VisDQMStatusMap &status,
		 const StringAtom &x)
    {
      assert(status.count(x));
      const VisDQMStatus &info = status.find(x)->second;
      return StringFormat("{'name':%1, 'dir':{'nalarm':%2, 'nlive':%3, 'nleaves':%4}}")
	.arg(stringToJSON(x.string(), true))
	.arg(info.nalarm)
	.arg(info.nlive)
	.arg(info.nleaves);
    }

  static std::string
  shownObjToJSON(const VisDQMItems &contents,
		 const VisDQMStatusMap &status,
		 const VisDQMDrawOptionMap &drawopts,
		 const VisDQMSample &sample,
		 const StringAtom &x)
    {
      VisDQMShownRows layout;
      layoutOne(layout, contents, status, drawopts, x);
      return StringFormat("{'name':%1, 'items':%2}")
	.arg(stringToJSON(x.string()))
	.arg(layoutToJSON(layout, sample));
    }

  static void
  copyAxisOptions(VisDQMAxisOptions &to, const VisDQMAxisOptions &from)
    {
      if (! from.min.empty())
	to.min = from.min;

      if (! from.max.empty())
	to.max = from.max;

      if (! from.type.empty())
	to.type = from.type;
    }

  static void
  copyDrawOptions(VisDQMDrawOptions *to, const VisDQMDrawOptions *from)
    {
      if (! from)
	return;

      if (! from->withref.empty())
	to->withref = from->withref;

      if (! from->drawopts.empty())
	to->drawopts = from->drawopts;

      copyAxisOptions(to->xaxis, from->xaxis);
      copyAxisOptions(to->yaxis, from->yaxis);
      copyAxisOptions(to->zaxis, from->zaxis);
    }

  static void
  setDrawOptions(VisDQMDrawOptions *to,
		 const VisDQMDrawOptions *opt1 = 0,
		 const VisDQMDrawOptions *opt2 = 0)
    {
      to->withref = "def";
      to->drawopts = "";
      to->xaxis.type = "def";
      to->yaxis.type = "def";
      to->zaxis.type = "def";
      copyDrawOptions(to, opt1);
      copyDrawOptions(to, opt2);
    }

  // Build a layout on this node, 1x1 if it's not a layout otherwise.
  static void
  layoutOne(VisDQMShownRows &layout,
	    const VisDQMItems &contents,
	    const VisDQMStatusMap &status,
	    const VisDQMDrawOptionMap &drawopts,
	    const StringAtom &path)
    {
      VisDQMItems::const_iterator          objpos = contents.find(path);
      VisDQMStatusMap::const_iterator      statuspos = status.find(path);

      assert(objpos != contents.end());
      assert(objpos->second->name == path);
      assert(statuspos != status.end());

      const VisDQMItem &obj = *objpos->second;
      const VisDQMStatus &info = statuspos->second;

      if (! obj.layout)
      {
	VisDQMDrawOptionMap::const_iterator drawpos = drawopts.find(path);
	const VisDQMDrawOptions *xopts = 0;
	if (drawpos != drawopts.end())
	  xopts = &drawpos->second;

	shared_ptr<VisDQMShownItem> item(new VisDQMShownItem);
	item->version = obj.version;
	item->nalarm = info.nalarm;
	item->nlive = info.nlive;
	item->name = obj.name;
	item->plotter = obj.plotter;
	setDrawOptions(&item->drawopts, xopts);
	shared_ptr<VisDQMShownRow> row(new VisDQMShownRow);
	row->columns.push_back(item);
	layout.push_back(row);
      }
      else
      {
	const VisDQMLayoutRows &rows = *obj.layout;
	layout.reserve(rows.size());

	for (size_t nrow = 0; nrow < rows.size(); ++nrow)
	{
	  const VisDQMLayoutRow &row = *rows[nrow];
	  shared_ptr<VisDQMShownRow> srow(new VisDQMShownRow);
	  srow->columns.reserve(row.columns.size());
	  layout.push_back(srow);

	  for (size_t ncol = 0; ncol < row.columns.size(); ++ncol)
	  {
	    const VisDQMDrawOptions *xopts = 0;
	    const VisDQMDrawOptions *lopts = 0;
	    const VisDQMLayoutItem &col = *row.columns[ncol];
	    shared_ptr<VisDQMShownItem> scol(new VisDQMShownItem);

	    if ((objpos = contents.find(col.path)) != contents.end())
	    {
	      statuspos = status.find(col.path);
	      assert(statuspos != status.end());

	      const VisDQMItem &lobj = *objpos->second;
	      const VisDQMStatus &linfo = statuspos->second;
	      assert(lobj.name == col.path);

	      scol->version = lobj.version;
	      scol->nalarm = linfo.nalarm;
	      scol->nlive = linfo.nlive;
	      scol->name = col.path;
	      scol->desc = col.desc;
	      scol->plotter = lobj.plotter;
              scol->overlays = col.overlays;
	      lopts = &col.drawopts;

	      VisDQMDrawOptionMap::const_iterator drawpos = drawopts.find(col.path);
	      if (drawpos != drawopts.end())
		xopts = &drawpos->second;
	    }
	    else
	    {
	      scol->version = 0;
	      scol->nalarm = 0;
	      scol->nlive = 1;
	      scol->name = col.path;
	      scol->desc = col.desc;
	      scol->plotter = 0;
	    }

	    setDrawOptions(&scol->drawopts, lopts, xopts);
	    srow->columns.push_back(scol);
	  }
	}
      }
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMContentWorkspace : public VisDQMWorkspace, public VisDQMLayoutTools
{
  VisDQMRegexp  rxmatch_;
  VisDQMRegexp	rxlayout_;
public:
  VisDQMContentWorkspace(py::object gui,
			 const std::string &name,
			 const std::string &match,
			 const std::string &layouts)
    : VisDQMWorkspace(gui, name)
    {
      if (! match.empty() && match != "^")
      {
        fastrx(rxmatch_, match);
        if (layouts.empty())
          fastrx(rxlayout_, "(" + match + "Layouts)/.*");
      }
      if (! layouts.empty() && layouts !="^")
        fastrx(rxlayout_, "(" + layouts + ")/.*");

    }

  virtual ~VisDQMContentWorkspace(void)
    {}

  // Helper function to present session state.  Returns a JSON object
  // representing current menu and canvas state.
  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample  sample       (sessionSample(session));
      std::string   toolspanel   (sessionPanelConfig(session, "tools"));
      std::string   helppanel    (sessionPanelConfig(session, "help"));
      std::string   custompanel  (sessionPanelConfig(session, "custom"));
      std::string   zoom         (sessionZoomConfig(session));
      std::string   root         (workspaceParam<std::string>(session, "dqm.root", ""));
      std::string   focus        (workspaceParam<std::string>(session, "dqm.focus", ""));  // None
      std::string   filter       (py::extract<std::string>(session.get("dqm.filter")));
      std::string   showstats    (py::extract<std::string>(session.get("dqm.showstats")));
      std::string   showerrbars  (py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string   reference    (sessionReference(session));
      std::string   strip	 (sessionStripChart(session));
      std::string   submenu      (py::extract<std::string>(session.get("dqm.submenu")));
      std::string   size	 (py::extract<std::string>(session.get("dqm.size")));
      std::string   rxstr        (py::extract<std::string>(session.get("dqm.search")));
      py::dict      pydrawopts   (workspaceParam<py::dict>(session, "dqm.drawopts", py::dict()));
      py::tuple     myobjst      (workspaceParam<py::tuple>(session, "dqm.myobjs", py::tuple()));
      StringAtomSet myobjs;

      for (py::stl_input_iterator<std::string> e, i(myobjst); i != e; ++i)
        myobjs.insert(StringAtom(&stree, *i));

      VisDQMDrawOptionMap drawopts;
      for (py::stl_input_iterator<std::string> i(pydrawopts), e; i != e; ++i)
	translateDrawOptions(py::extract<py::dict>(pydrawopts.get(*i)),
			     drawopts[StringAtom(&stree, *i)]);

      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		shown;
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	std::string		data;
	std::string		layoutroot;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample, current,
		      &rxmatch_, rxsearch.get(), qalarm,
		      &layoutroot, &rxlayout_);
	buildShown(contents, shown, root);
	updateStatus(contents, status);

	if (! status.count(StringAtom(&stree, root)))
	  root = ""; // FIXME: #36093

        return makeResponse(StringFormat("{'kind':'DQMCanvas', 'items':%1,"
                                         " 'root':%2, 'focus':%3, 'size':'%4', %5,"
                                         " %6, 'showstats': %7, 'showerrbars': %8, %9,"
                                         " 'reference':%10, 'strip':%11,"
                                         " 'layoutroot':%12}")
                            .arg(shownToJSON(contents, status, drawopts,
                                             StringAtom(&stree, root),
                                             sample, shown))
                            .arg(stringToJSON(root))
                            .arg(stringToJSON(focus, true))
                            .arg(size)
                            .arg(helppanel)
                            .arg(custompanel)
                            .arg(showstats)
                            .arg(showerrbars)
                            .arg(zoom)
                            .arg(reference)
                            .arg(strip)
                            .arg(stringToJSON(layoutroot)),
                            sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, contents.size(),
                            toolspanel, startTime);
      }
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMPlayWorkspace : public VisDQMWorkspace, public VisDQMLayoutTools
{
public:
  VisDQMPlayWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMPlayWorkspace(void)
    {}

  // Helper function to present session state.  Returns a JSON object
  // representing current menu and canvas state.
  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample  sample       (sessionSample(session));
      std::string   toolspanel   (sessionPanelConfig(session, "tools"));
      std::string   workspace    (py::extract<std::string>(session.get("dqm.play.prevws")));
      std::string   root         (workspaceParamOther<std::string>(session, "dqm.root", "", workspace));
      std::string   filter       (py::extract<std::string>(session.get("dqm.filter")));
      std::string   showstats    (py::extract<std::string>(session.get("dqm.showstats")));
      std::string   showerrbars  (py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string   reference    (sessionReference(session));
      std::string   strip	 (sessionStripChart(session));
      std::string   submenu      (py::extract<std::string>(session.get("dqm.submenu")));
      std::string   rxstr        (py::extract<std::string>(session.get("dqm.search")));
      int           playpos      (py::extract<int>(session.get("dqm.play.pos")));
      int           playinterval (py::extract<int>(session.get("dqm.play.interval")));
      py::dict      pydrawopts   (workspaceParamOther<py::dict>(session, "dqm.drawopts", py::dict(), workspace));
      py::tuple     myobjst      (workspaceParamOther<py::tuple>(session, "dqm.myobjs", py::tuple(), workspace));
      StringAtomSet myobjs;

      for (py::stl_input_iterator<std::string> e, i (myobjst); i != e; ++i)
        myobjs.insert(StringAtom(&stree, *i));

      VisDQMDrawOptionMap drawopts;
      for (py::stl_input_iterator<std::string> i(pydrawopts), e; i != e; ++i)
	translateDrawOptions(py::extract<py::dict>(pydrawopts.get(*i)),
			     drawopts[StringAtom(&stree, *i)]);

      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		shown;
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	std::string		data;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), qalarm, 0, 0);
	buildShown(contents, shown, root);
	updateStatus(contents, status);

	if (! status.count(StringAtom(&stree, root)))
	  root = ""; // FIXME: #36093

	if (shown.size())
	  playpos = std::min(playpos, int(shown.size())-1);
	else
	  playpos = 0;

        return makeResponse(StringFormat("{'kind':'DQMPlay', 'items':%1,"
					 " 'interval':%2, 'pos':%3, 'max':%4,"
					 " 'reference':%5, 'strip':%6, 'showstats': %7,"
                                         " 'showerrbars': %8}")
			    .arg(shownToJSON(contents, status, drawopts,
					     StringAtom(&stree, root),
					     sample, shown))
			    .arg(playinterval)
			    .arg(playpos)
			    .arg(shown.size())
			    .arg(reference)
			    .arg(strip)
                            .arg(showstats)
                            .arg(showerrbars),
			    300, current, services, workspace, workspaces,
			    submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, contents.size(),
                            toolspanel, startTime);
      }
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
VisDQMRenderLink *VisDQMRenderLink::s_instance;

#define EXC_TRANSLATE(n,t,what)					\
  struct translate ## n { static void translate (const t &e)	\
    { PyErr_SetString(PyExc_RuntimeError, e. what); } };	\
  py::register_exception_translator< t >			\
    (&translate ## n :: translate)

BOOST_PYTHON_MODULE(Accelerator)
{
  DebugAids::failHook(&onAssertFail);
  Signal::handleFatal(0, IOFD_INVALID, 0, 0,
		      (Signal::FATAL_DEFAULT
		       & ~(Signal::FATAL_ON_INT
			   | Signal::FATAL_ON_QUIT
			   | Signal::FATAL_DUMP_CORE)));
  Signal::ignore(SIGPIPE);

  // Initialise locks.
  pthread_rwlock_init(&parentslock, 0);

  // Register types and conversions.
  py::enum_<VisDQMSampleType>("sampletype")
    .value(sampleTypeLabel[SAMPLE_LIVE],           SAMPLE_LIVE)
    .value(sampleTypeLabel[SAMPLE_ONLINE_DATA],    SAMPLE_ONLINE_DATA)
    .value(sampleTypeLabel[SAMPLE_OFFLINE_DATA],   SAMPLE_OFFLINE_DATA)
    .value(sampleTypeLabel[SAMPLE_OFFLINE_RELVAL], SAMPLE_OFFLINE_RELVAL)
    .value(sampleTypeLabel[SAMPLE_OFFLINE_MC],     SAMPLE_OFFLINE_MC);

  // Register methods.
  py::class_<VisDQMRenderLink, shared_ptr<VisDQMRenderLink>, boost::noncopyable>
    ("DQMRenderLink", py::init<std::string, std::string, std::string, int, bool>())
    .def("_start", &VisDQMRenderLink::start)
    .def("_stop", &VisDQMRenderLink::stop);

  py::class_<VisDQMToJSON, shared_ptr<VisDQMToJSON>, boost::noncopyable>
    ("DQMToJSON", py::init<>())
    .def("_samples", &VisDQMToJSON::samples)
    .def("_list", &VisDQMToJSON::list);

  py::class_<VisDQMSource, shared_ptr<VisDQMSource>, boost::noncopyable>
    ("DQMSource", py::no_init)
    .def("prepareSession", &VisDQMSource::prepareSession);

  py::class_<VisDQMUnknownSource, shared_ptr<VisDQMUnknownSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMUnknownSource", py::init<>())
    .add_property("plothook", &VisDQMUnknownSource::plotter)
    .add_property("jsonhook", &VisDQMUnknownSource::jsoner)
    .def("_plot", &VisDQMUnknownSource::plot);

  py::class_<VisDQMOverlaySource, shared_ptr<VisDQMOverlaySource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMOverlaySource", py::init<>())
    .add_property("plothook", &VisDQMOverlaySource::plotter)
    .add_property("jsonhook", &VisDQMOverlaySource::jsoner)
    .def("_plot", &VisDQMOverlaySource::plot);

  py::class_<VisDQMStripChartSource, shared_ptr<VisDQMStripChartSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMStripChartSource", py::init<>())
    .add_property("plothook", &VisDQMStripChartSource::plotter)
    .add_property("jsonhook", &VisDQMStripChartSource::jsoner)
    .def("_plot", &VisDQMStripChartSource::plot);

  py::class_<VisDQMCertificationSource, shared_ptr<VisDQMCertificationSource>,
             py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMCertificationSource", py::init<>())
    .add_property("plothook", &VisDQMCertificationSource::plotter)
    .add_property("jsonhook", &VisDQMCertificationSource::jsoner)
    .def("_plot", &VisDQMCertificationSource::plot);

  py::class_<VisDQMLiveSource, shared_ptr<VisDQMLiveSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMLiveSource", py::init<py::object, py::dict>())
    .add_property("plothook", &VisDQMLiveSource::plotter)
    .add_property("jsonhook", &VisDQMLiveSource::jsoner)
    .def("_plot", &VisDQMLiveSource::plot)
    .def("_exit", &VisDQMLiveSource::exit);

  py::class_<VisDQMArchiveSource, shared_ptr<VisDQMArchiveSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMArchiveSource", py::init<py::object, py::dict>())
    .add_property("plothook", &VisDQMArchiveSource::plotter)
    .add_property("jsonhook", &VisDQMArchiveSource::jsoner)
    .def("_plot", &VisDQMArchiveSource::plot)
    .def("_getJson", &VisDQMArchiveSource::getJson)
    .def("_exit", &VisDQMArchiveSource::exit);

  py::class_<VisDQMLayoutSource, shared_ptr<VisDQMLayoutSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMLayoutSource")
    .def("_pushLayouts", &VisDQMLayoutSource::pushLayouts);

  py::class_<VisDQMWorkspace, shared_ptr<VisDQMWorkspace>, boost::noncopyable>
    ("DQMWorkspace", py::no_init)
    .def("_changeRun", &VisDQMContentWorkspace::changeRun)
    .def("_profilesnap", &VisDQMWorkspace::profilesnap);

  py::class_<VisDQMSampleWorkspace, shared_ptr<VisDQMSampleWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMSampleWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMSampleWorkspace::state);

  py::class_<VisDQMSummaryWorkspace, shared_ptr<VisDQMSummaryWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMSummaryWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMSummaryWorkspace::state);

  py::class_<VisDQMCertificationWorkspace, shared_ptr<VisDQMCertificationWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMCertificationWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMCertificationWorkspace::state);

  py::class_<VisDQMQualityWorkspace, shared_ptr<VisDQMQualityWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMQualityWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMQualityWorkspace::state);

  py::class_<VisDQMContentWorkspace, shared_ptr<VisDQMContentWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMContentWorkspace", py::init<py::object, std::string, std::string, std::string>())
    .def("_state", &VisDQMContentWorkspace::state);

  py::class_<VisDQMPlayWorkspace, shared_ptr<VisDQMPlayWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMPlayWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMPlayWorkspace::state);

  EXC_TRANSLATE(StdError, std::exception, what());
  EXC_TRANSLATE(LATError, lat::Error, explain().c_str());
}
