#include "DQM/VisDQMCache.h"

using namespace lat;

VisDQMCache::VisDQMCache(size_t maxsize /* = 80*1024*1024 */)
  : maxsize_(maxsize),
    cursize_(0)
{
  pthread_mutex_init(&lock_, 0);
}

VisDQMCache::~VisDQMCache(void)
{
}

bool
VisDQMCache::get(uint64_t file, uint64_t offset, VisDQMBuf &into)
{
  pthread_mutex_lock(&lock_);

  bool found = false;
  FileKey key(file, offset);
  CacheMap::iterator pos = cache_.find(key);
  if (pos != cache_.end())
  {
    ASSERT(pos->second.second.size());
    ASSERT(cursize_ >= pos->second.second.size());
    cursize_ -= pos->second.second.size();
    into.swap(pos->second.second);
    cache_.erase(pos);
    found = true;
  }

  pthread_mutex_unlock(&lock_);

  return found;
}

void
VisDQMCache::put(uint64_t file, uint64_t offset, VisDQMBuf &from)
{
  ASSERT(from.size());
  pthread_mutex_lock(&lock_);

  FileKey key(file, offset);
  LRUData &item = cache_[key];
  ASSERT(cursize_ >= item.second.size());
  ASSERT(! item.second.size());
  cursize_ -= item.second.size();
  cursize_ += from.size();
  item.second.release();
  item.second.swap(from);
  item.first = Time::current().ns();

  shrink();
  pthread_mutex_unlock(&lock_);
}

void
VisDQMCache::flush(void)
{
  pthread_mutex_lock(&lock_);
  cache_.clear();
  cursize_ = 0;
  pthread_mutex_unlock(&lock_);
}

void
VisDQMCache::shrink(void)
{
  while (cursize_ > maxsize_)
  {
    CacheMap::iterator end = cache_.end();
    CacheMap::iterator pos = cache_.begin();
    CacheMap::iterator oldest;
    for (oldest = pos; pos != end; ++pos)
      if (pos->second.first < oldest->second.first)
	oldest = pos;

    ASSERT(cursize_ >= oldest->second.second.size());
    cursize_ -= oldest->second.second.size();
    cache_.erase(oldest);
  }
}
