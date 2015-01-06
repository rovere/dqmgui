#ifndef DQM_VISDQMARCHIVESOURCE_H
#define DQM_VISDQMARCHIVESOURCE_H

#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMArchiveWatch.h"
#include "DQM/VisDQMCache.h"
#include "DQM/VisDQMRenderLink.h"

#include "classlib/utils/Regexp.h"
#include "classlib/iobase/IOStatus.h"

#include <pthread.h>
#include <inttypes.h>

#include <vector>
#include <map>

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
  void loadIndex(void);

  // Report a file read failure.  Returns true if reading should
  // be abandoned: more than 10 tries have failed to open a file.
  bool fileReadFailure(int ntries, const char *message);
  
  // Prepare to read from the index. Reloads the index if there have
  // been past file read errors, or if the index has changed.
  void maybeReloadIndex(void);

  // Open a possibly cached data file.  This must be called with a
  // read lock on "this" held; it will upgrade the lock to a write
  // lock if that is necessary to open the file.
  VisDQMFilePtr open(uint16_t kind, const uint32_t *file);

  SampleList::iterator findSample(const VisDQMSample &sample);

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

  virtual void getdata(const VisDQMSample &sample,
                       const std::string &path,
                       std::string &streamers,
                       DQMNet::Object &obj);

  virtual void getattr(const VisDQMSample &sample,
                       const std::string &path,
                       VisDQMIndex::Summary &attrs,
                       std::string & /* xstreamers */,
                       DQMNet::Object & /* xobj */);

  py::str getJson(const int runnr,
                  const std::string &dataset,
                  const std::string &path,
                  py::dict opts);

  py::tuple plot(int runnr,
                 const std::string &dataset,
                 const std::string &path,
                 py::dict opts);

  // Refresh data from the database for a run.  Checks if the master
  // database exists, and if so, if it has changed relative to the
  // last copy and the time we last read in the data.  If so, discards
  // any existing cached data and copies the file again.  Then checks
  // to see if the data for the requested run is already cached, and
  // if not, reads it in.  Also discards cached data for runs that
  // have not been accessed recently, where "recent" is MAX_CACHE_LIFE
  // seconds.
  void update(VisDQMItems &items,
              const VisDQMSample &sample,
              VisDQMEventNum &current,
              VisDQMRegexp *rxmatch,
              Regexp *rxsearch,
              bool *alarm);

  // Initialise a new session.  Select the most recent sample.
  virtual void prepareSession(py::dict session);

  // Extract data from this source.  Work is done by update().
  virtual void scan(VisDQMItems &result,
                    const VisDQMSample &sample,
                    VisDQMEventNum &current,
                    VisDQMRegexp *rxmatch,
                    Regexp *rxsearch,
                    bool *alarm,
                    std::string *,
                    VisDQMRegexp *);

  virtual void json(const VisDQMSample &sample,
                    const std::string &rootpath,
                    bool fulldata,
                    bool lumisect,
                    std::string &result,
                    double &stamp,
                    const std::string &mename,
                    std::set<std::string> &dirs);

  virtual void getcert(VisDQMSample &sample,
                       const std::string &rootpath,
                       const std::string &variableName,
                       std::vector<VisDQMIndex::Summary> &attrs,
                       std::vector<double> &axisvals,
                       std::string &binlabels);

  virtual void samples(VisDQMSamples &samples);
};

#endif  // DQM_VISDQMARCHIVESOURCE_H
