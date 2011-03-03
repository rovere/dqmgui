#ifndef DQM_VISDQMCACHE_H
# define DQM_VISDQMCACHE_H

# include "DQM/VisDQMBuf.h"
# include "classlib/iobase/Filename.h"
# include "classlib/utils/Time.h"
# include <pthread.h>
# include <stdint.h>
# include <vector>
# include <map>

/** In-memory read cache for I/O efficient access to DQM data files
    (#VisDQMFile) in a DQM index (#VisDQMIndex).  The cache is a
    repository of uncompressed file segments currently not in use.
    The #VisDQMFile will put currently unreferenced pages into the
    cache, and will look data in the cache before going on disk.

    The cache is configured with a desired maximum memory use for
    caching.  The total cache size should be sufficiently large to
    satisfy normal workloads from the cache with no trashing, and
    small enough to avoid swapping with total system workload.

    The cache automatically discards least recently used pages, and
    should be flushed when the index generation is upgraded. */
class VisDQMCache
{
  typedef std::pair<uint64_t, uint64_t> FileKey;
  typedef std::pair<uint64_t, VisDQMBuf> LRUData;
  typedef std::map<FileKey, LRUData> CacheMap;

  pthread_mutex_t	lock_;
  size_t		maxsize_;
  size_t		cursize_;
  CacheMap		cache_;

public:
  VisDQMCache(size_t maxsize = 80*1024*1024);
  ~VisDQMCache(void);

  bool			get(uint64_t file, uint64_t offset, VisDQMBuf &into);
  void			put(uint64_t file, uint64_t offset, VisDQMBuf &from);
  void			flush(void);

private:
  void			shrink(void);

  VisDQMCache(VisDQMCache &);
  void operator=(VisDQMCache &);
};

#endif // DQM_VISDQMCACHE_H
