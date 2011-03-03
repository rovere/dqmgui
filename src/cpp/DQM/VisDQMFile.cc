#include "DQM/VisDQMFile.h"
#include "DQM/VisDQMCache.h"
#include "DQM/VisDQMError.h"
#include "classlib/iobase/File.h"
#include "classlib/iobase/Filename.h"
#include "classlib/iobase/FileError.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/DebugAids.h"
#include "classlib/utils/Error.h"
#include "classlib/utils/Time.h"
#include "classlib/zip/CRC32.h"

using namespace lat;

/** Utility function to round @a value to a value divisible by @a unit. */
static inline uint32_t
roundup(uint32_t value, uint32_t unit)
{
  return (value + unit - 1) / unit * unit;
}

/** Create a new DQM file object.  Use #open() to make use of the file object. */
VisDQMFile::VisDQMFile(void)
  : mode_(OPEN_READ),
    cache_(0),
    cacheKey_(0),
    indexBegin_(0),
    indexEnd_(0),
    wpos_(0)
{
  pthread_rwlock_init(&lock_, 0);
}

/** Destroy the file object.  The caller must explicitly close the file. */
VisDQMFile::~VisDQMFile(void)
{
}

/** Return the file path. */
const Filename &
VisDQMFile::path(void) const
{
  return path_;
}

/** Close the file and reset the object state so another file can be
    opened later.

    If the file was open for writing, flushes the B-tree index and file
    header out to disk before closing the file.

    This method will block until all read and write heads open on this
    file have been closed.  Note that this will block even in single
    threaded applications if there is an open read or write head. */
void
VisDQMFile::close(void)
{
  // FIXME: purge pages. careful with locking as other thread might update
  // simultaneously, and might try to get rid of a page which requires a
  // write lock on this object!

  pthread_rwlock_wrlock(&lock_);

  // If we are writing, flush out the index and update the header.
  if (mode_ == OPEN_WRITE)
  {
    Header header = { MAGIC, VINFO,
		      { 0, indexData_.size(),
			OPT_COMPRESSION_ZLIB
			| OPT_INDEX_LEVEL_BTREE } };
    put(header.index, indexData_);
    file_.write(&header, sizeof(header), 0);
  }

  // Now close the underlying file.
  file_.close();

  // Zero state.
  path_ = Filename();
  data_.clear();
  indexData_.release();
  indexBegin_ = 0;
  indexEnd_ = 0;
  wpos_ = 0;
  cache_ = 0;
  cacheKey_ = 0;

  pthread_rwlock_unlock(&lock_);
}

/** Utility method to read file contents from position @a at, storing
    the data to buffer @a into.  If @a at is a compressed segment, the
    data is uncompressed in a temporary buffer.  Returns the number of
    uncompressed bytes read.  Throws a #VisDQMError exception in case
    of an error. */
IOSize
VisDQMFile::get(const Address &at, VisDQMBuf &into)
{
  IOSize n;

  // Fast bypass for null reads.
  if (at.length == 0)
  {
    into.resize(0);
    return 0;
  }

  // See if it's cached.
  if (cache_ && cache_->get(cacheKey_, at.offset, into))
    return into.size();

  // If it's compressed data, read into temporary buffer and
  // uncompress.  (FIXME: Reuse available memory from cache.)
  else if ((at.options & OPT_COMPRESSION_MASK) != OPT_COMPRESSION_NONE)
  {
    VisDQMBuf temp(at.length+1);
    CRC32 crc32;

    if (at.length < 2*sizeof(uint32_t))
      throw VisDQMError(0, path_.name(),
			StringFormat("[o=%1:n=%2:f=%3]: page length too short")
			.arg((long) at.offset).arg(at.length).arg(at.options));

    if ((n = file_.read(&temp[0], at.length, at.offset)) < at.length)
      throw VisDQMError(0, path_.name(),
			StringFormat("[o=%1:n=%2:f=%3]: only %4 bytes read")
			.arg((long) at.offset).arg(at.length).arg(at.options)
			.arg(n));

    uint32_t dlen = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
    uint32_t cksum = (temp[4] << 24) + (temp[5] << 16) + (temp[6] << 8) + temp[7];
    crc32.update(&temp[2*sizeof(uint32_t)], at.length-2*sizeof(uint32_t));
    if (cksum != crc32.value())
      throw VisDQMError(0, path_.name(),
			StringFormat("[o=%1:n=%2:f=%3]: checksum mismatch for"
				     " compressed data, expected %4, got %5")
			.arg((long) at.offset).arg(at.length).arg(at.options)
			.arg(cksum).arg(crc32.value()));

    // Uncompress if known compression method.
    // NB: Must use different inflate engine in concurrent threads.
    into.resize(dlen);
    if ((at.options & OPT_COMPRESSION_MASK) == OPT_COMPRESSION_ZLIB)
    {
      ZDecompressor inflate;
      inflate.input(&temp[2*sizeof(uint32_t)], at.length-2*sizeof(uint32_t)+1);
      if ((n = inflate.decompress(&into[0], dlen)) != dlen)
	throw VisDQMError(0, path_.name(),
			  StringFormat("[o=%1:n=%2:f=%3]: expected %4, got %5"
				       " bytes decompressed data")
			  .arg((long) at.offset).arg(at.length).arg(at.options)
			  .arg(dlen).arg(n));

      if (! inflate.finished())
	throw VisDQMError(0, path_.name(),
			  StringFormat("[o=%1:n=%2:f=%3]: decompression incomplete,"
				       " %4 bytes remain, %5more input needed,"
				       " %6 bytes consumed, %7 bytes returned")
			  .arg((long) at.offset).arg(at.length).arg(at.options)
			  .arg(inflate.rest()).arg(inflate.more() ? "" : "no ")
			  .arg((long) inflate.in()).arg((long) inflate.out()));

      return n;
    }
    else
      throw VisDQMError(0, path_.name(),
			StringFormat("[o=%1:n=%2:f=%3]: unexpected block"
				     " compression method %4")
			.arg((long) at.offset).arg(at.length).arg(at.options)
			.arg(at.options & OPT_COMPRESSION_MASK));
  }

  // Uncompressed output, read as such.
  else
  {
    into.resize(at.length);
    if ((n = file_.read(&into[0], at.length, at.offset)) < at.length)
      throw VisDQMError(0, path_.name(),
			StringFormat("[o=%1:n=%2:f=%3]: only %4 bytes read")
			.arg((long) at.offset).arg(at.length).arg(at.options)
			.arg(n));

    return n;
  }
}

/** Utility method to write data buffer @a from to the next available
    location in the file.  The @at parameter is updated to reflect the
    location where the data was written.

    Data is stored into the file in first-come, first server basis,
    consequtively without any holes.  There is no way for the caller
    to request where to write the data.  If the write is successful,
    the internal write pointer is moved by the amount of data written.

    Throws a #VisDQMError exception in case of an error. */
void
VisDQMFile::putraw(Address &at, const VisDQMBuf &from)
{
  IOSize n = file_.write(&from[0], at.length, at.offset = wpos_);
  if (n != at.length)
    throw VisDQMError(0, path_.name(),
		      StringFormat("[o=%1:n=%2:f=%3]: only %4 bytes written")
		      .arg((long) at.offset).arg(at.length).arg(at.options)
		      .arg(n));
  wpos_ += n;
}

/** Utility method to write data buffer @a from to the next available
    location in the file.  Like #putraw(), but does synchronous data
    compression if requested.  WriteHead offers asynchronous parallel
    writing and should be used where possible. */
void
VisDQMFile::put(Address &at, const VisDQMBuf &from)
{
  VisDQMBuf workbuf;
  const VisDQMBuf *data = &from;

  switch (at.options & OPT_COMPRESSION_MASK)
  {
  case OPT_COMPRESSION_NONE:
    break;

  case OPT_COMPRESSION_ZLIB:
    {
      IOSize n;
      workbuf.resize(at.length + 2*sizeof(uint32_t));
      ZCompressor deflate(ZConstants::BEST_COMPRESSION);
      deflate.input(&from[0], at.length);
      deflate.finish();
      n = deflate.compress(&workbuf[2*sizeof(uint32_t)], at.length);
      if (deflate.finished())
      {
	CRC32 crc32;
	crc32.update(&workbuf[2*sizeof(uint32_t)], n);
	unsigned checksum = crc32.value();

	workbuf[0] = (at.length >> 24) & 0xff;
	workbuf[1] = (at.length >> 16) & 0xff;
	workbuf[2] = (at.length >>  8) & 0xff;
	workbuf[3] = (at.length      ) & 0xff;

	workbuf[4] = (checksum  >> 24) & 0xff;
	workbuf[5] = (checksum  >> 16) & 0xff;
	workbuf[6] = (checksum  >>  8) & 0xff;
	workbuf[7] = (checksum       ) & 0xff;

	at.length = n + 2*sizeof(uint32_t);
	data = &workbuf;
      }
      else
	at.options &= ~OPT_COMPRESSION_MASK;
    }
    break;

  default:
    throw VisDQMError(0, path_.name(),
		      StringFormat("[o=%1:n=%2:f=%3]: request for"
				   " unrecognised compression method %4")
		      .arg((long) at.offset).arg(at.length).arg(at.options)
		      .arg(at.options & OPT_COMPRESSION_MASK));
  }

  putraw(at, *data);
}

/** Utility method to transfer a segment from another file to this one
    without uncompressing and recompressing it, i.e. as a direct byte
    copy.

    The data is copied from file @a f at position @a from, to the
    current end of this file.  The caller should set @a at argument to
    an exact copy of the @a from address; the @a at offset field is
    updated to reflect the new file position in this file.

    Throws a #VisDQMError in case of an error. */
void
VisDQMFile::xfer(Address &at, VisDQMFile *f, const Address &from)
{
  IOSize n;
  VisDQMBuf temp(from.length);
  if ((n = f->file_.read(&temp[0], from.length, from.offset)) != from.length)
    throw VisDQMError(0, f->path_.name(),
		      StringFormat("[o=%1:n=%2:f=%3]: only %4 bytes read")
		      .arg((long) from.offset).arg(from.length).arg(from.options)
		      .arg(n));

  if ((n = file_.write(&temp[0], from.length, at.offset = wpos_)) != from.length)
    throw VisDQMError(0, path_.name(),
		      StringFormat("[o=%1:n=%2:f=%3]: only %4 bytes written")
		      .arg((long) wpos_).arg(from.length).arg(from.options)
		      .arg(n));
  wpos_ += n;
}

/** Open the file at @a path for reading or writing according to @a mode.

    If the file is opened for reaeding, it must exist.  If the file is
    opened for writing, it is created if it doesn't exist, and
    truncated if it does exist as documented in #VisDQMIndex::open().

    A file opened for reading cannot be written to, and conversely, a
    file open for writing cannot be read from.

    If @a cache is non-null, will use it for optimising reads with
    file cache key @a cacheKey.

    Throws a #VisDQMError in case of an error. */
void
VisDQMFile::open(uint64_t cacheKey, const Filename &path,
		 OpenMode mode, VisDQMCache *cache)
{
  // Obtain write lock on this object.
  pthread_rwlock_wrlock(&lock_);
  ASSERT(file_.fd() == IOFD_INVALID);
  ASSERT(path_.empty());
  ASSERT(cache_ == 0);
  ASSERT(cacheKey_ == 0);
  ASSERT(indexData_.empty());
  ASSERT(indexBegin_ == 0);
  ASSERT(indexEnd_ == 0);
  ASSERT(data_.empty());

  // Open the file, release lock on any exception.
  try
  {
    // Open the file with appropriate options.  When opening a file
    // for write, simply overwrite already existing files, on the
    // assumption they are a result of a previous aborted write.
    file_.open(path, mode == OPEN_READ ? IOFlags::OpenRead
	       : (IOFlags::OpenRead | IOFlags::OpenWrite
		  | IOFlags::OpenCreate | IOFlags::OpenTruncate));

    // If we are opening for read, read the file header and index in.
    if (mode == OPEN_READ)
    {
      Header header;
      if (file_.read(&header, sizeof(header), 0) != sizeof(header)
	  || header.magic != MAGIC
	  || header.vinfo != VINFO)
	throw VisDQMError(0, path.name(), "invalid file");

      get(header.index, indexData_);
      if (indexData_.size() > 0)
      {
	size_t n = indexData_.size() / sizeof(IndexEntry);
	indexBegin_ = (IndexEntry *) &indexData_[0];
	indexEnd_ = indexBegin_ + n;
      }
      else
      {
	indexBegin_ = 0;
	indexEnd_ = 0;
      }
    }

    // If we are opening for write, prepare place holder file header.
    else
    {
      indexData_.reserve(300*sizeof(IndexEntry));
      Header h = { MAGIC, VINFO, { sizeof(Header), 0, OPT_INDEX_LEVEL_BTREE } };
      if (file_.write(&h, sizeof(h), 0) != sizeof(h))
	throw VisDQMError(0, path.name(), "failed to initialise file header");

      wpos_ = sizeof(h);
    }

    path_ = path;
    mode_ = mode;
  }
  catch (...)
  {
    pthread_rwlock_unlock(&lock_);
    throw;
  }

  // If we are caching, enable cache now.
  cache_ = cache;
  cacheKey_ = cacheKey;

  // Finally release the lock.
  pthread_rwlock_unlock(&lock_);
}

// ----------------------------------------------------------------------
/** If the read head is marked for lazy re-positioning, make sure the
    requested position is paged in, then move to the right key.  If
    #MOVE_BOUNDARY (= mostly), just move the first key on the page; if
    #MOVE_SEARCH (= constructor with non-zero key), search for the
    right key as well.  Then reset lazy re-positioning. */
void
VisDQMFile::ReadHead::move(void)
{
  pthread_rwlock_rdlock(&file_->lock_);
  if (index_ < file_->indexEnd_)
  {
    page_ = pagein(index_->address);
    if (move_ == MOVE_SEARCH)
      while (obj_ < page_->nobjs && target_ > page_->objkeys[-obj_])
	++obj_;
    ASSERT(obj_ < page_->nobjs);
  }
  else
    pthread_rwlock_unlock(&file_->lock_);

  move_ = MOVE_NONE;
}

/** Create a file read head for @a file at @a key.

    If @a file is a null pointer, the next #isdone() call will return
    true.  Otherwise the read head is positioned at the first object
    whose key is greater or equal to @a key; if @a key is zero, at the
    first object in the file.  Note the next #get() call may return an
    object with a key larger than @a key if no such key was stored.

    The next #isdone() call will indicate whether there is data to be
    read at this position, and #get() will return that data.  Normal
    iteration pattern is:

    @code
       VisDQMFile::ReadHead rdhead(file, 0);
       while (! rdhead.isdone())
       {
	 void *begin, *end;
         uint64_t key;
         rdhead.get(&key, &begin, &end);
	 // Use key, begin, end
	 rdhead.next();
       }
    @endcode */
VisDQMFile::ReadHead::ReadHead(VisDQMFile *file, uint64_t key)
  : file_(file),
    page_(0),
    index_(0),
    target_(key),
    obj_(0),
    move_(MOVE_NONE)
{
  if (file)
  {
    uint64_t boundary = 0;
    pthread_rwlock_rdlock(&file->lock_);

    index_ = file->indexBegin_;
    while (index_ != file->indexEnd_ && target_ > index_->key)
    {
      boundary = index_->key+1;
      ++index_;
    }

    if (index_ != file->indexEnd_)
      move_ = (target_ == boundary ? MOVE_BOUNDARY : MOVE_SEARCH);

    pthread_rwlock_unlock(&file->lock_);
  }
}

/** Destroy a read head.  If the head hasn't been released yet,
    calls #finish() to release it. */
VisDQMFile::ReadHead::~ReadHead(void)
{
  finish();
}

/** Indicate end of iteration and release all resources held by the
    read head.  Note that the file the read head is reading cannot be
    closed until all reads are done with it. */
void
VisDQMFile::ReadHead::finish(void)
{
  if (page_)
  {
    // Release page lock and its bytes.
    pthread_rwlock_unlock(&page_->lock);
    if (! pthread_rwlock_trywrlock(&page_->lock))
    {
      if (page_->bytes.size())
      {
	if (file_->cache_)
	  file_->cache_->put(file_->cacheKey_, page_->offset, page_->bytes);
	else
	  page_->bytes.release();
      }

      pthread_rwlock_unlock(&page_->lock);
    }

    page_ = 0;
  }
}

/** Get the object at the current read head position.

    The key of the current object is stored into @a key, the pointer
    to the beginning of the object data area into @a start, and the
    pointer to one byte past the end of the object is stored into @a
    end.

    It is safe to call this method any number of times for a given
    read head position.  It will always return the same values.

    This method can only be called if #isdone() returns false. */
void
VisDQMFile::ReadHead::get(uint64_t *key, void **start, void **end)
{
  if (move_ != MOVE_NONE)
    move();

  *key = page_->objkeys[-obj_];
  *start = &page_->bytes[0] + page_->objlocs[-obj_];
  *end = &page_->bytes[0] + page_->objlocs[-obj_-1];
}

/** Check whether the read head has reached the end of the file.
    Returns true if there is no more data, false otherwise.  The
    #get() and #next() methods should be used only if this method
    returns true. */
bool
VisDQMFile::ReadHead::isdone(void)
{
  if (move_ != MOVE_NONE)
    move();

  if (! page_)
    return true;
  else if (obj_ < page_->nobjs)
    return false;
  else
    return (index_ + 1 == file_->indexEnd_);
}

/** Move to the next object position in the file.  This method can
    only be called if #isdone() returns true.  If there is no more
    data after this object, then the next call to #isdone() will
    return true and #get() or #next() should not be called any more. */
void
VisDQMFile::ReadHead::next(void)
{
  // Advance object location.  If we proceed past the end of the page,
  // see if we should hop to another page or have reached the end.
  if (++obj_ == page_->nobjs)
  {
    // Try upgrading page lock to write lock, and if successful
    // release the memory for the page data.
    pthread_rwlock_unlock(&page_->lock);
    if (! pthread_rwlock_trywrlock(&page_->lock))
    {
      if (page_->bytes.size())
      {
	if (file_->cache_)
	  file_->cache_->put(file_->cacheKey_, page_->offset, page_->bytes);
	else
	  page_->bytes.release();
      }

      pthread_rwlock_unlock(&page_->lock);
    }

    // Now mark the iterator to reload on next access.  If there is no
    // more index data to scan, we leave iterator marked as done.
    pthread_rwlock_rdlock(&file_->lock_);
    if (++index_ < file_->indexEnd_)
      move_ = MOVE_BOUNDARY;
    page_ = 0;
    obj_ = 0;
    pthread_rwlock_unlock(&file_->lock_);
  }
}

/** Helper method to page in file data from position @a address.

    Returns a pointer to the requested page, coordinating concurrent
    access between multiple read heads with #VisDQMFile, and caching
    with #VisDQMCache.

    When the method returns, the page data bytes are guaranteed to be
    in and locked for reading so nobody else can release them while this
    read head is looking at the data.

    This method must be entered with a read lock held on #file_, and
    it returns with the #file_ lock released and page lock held as a
    read lock.  The page read lock must be released when the page is
    no longer of use; at that point the page bytes should be reclaimed
    if nobody else is holding a concurrent read lock on the page. */
VisDQMFile::DataPage *
VisDQMFile::ReadHead::pagein(const Address &addr)
{
  // See if the file already has this page in a map.
  bool reload = false;
  bool upgrade = true;
  DataPage *pg = 0;
  DataMap::iterator dpos = file_->data_.find(addr.offset);

  // If the page wasn't found, upgrade the file lock to a write lock,
  // and look again.  If still not found, add the page to the file's
  // page map, mark the page for known reload, and grab a page write
  // lock (nobody can interefere here because we are still holding a
  // write lock on the file).
  if (dpos == file_->data_.end())
  {
    pthread_rwlock_unlock(&file_->lock_);
    pthread_rwlock_wrlock(&file_->lock_);
    dpos = file_->data_.find(addr.offset);
    if (dpos == file_->data_.end())
    {
      pg = &file_->data_[addr.offset];
      pthread_rwlock_init(&pg->lock, 0);
      pthread_rwlock_wrlock(&pg->lock);
      pg->offset = addr.offset;
      pg->first = 0;
      pg->objkeys = 0;
      pg->objlocs = 0;
      pg->ndata = 0;
      pg->nobjs = 0;
      reload = true;
      upgrade = false;
    }
  }

  // If we don't have a page pointer yet, it must mean it was found in
  // the page map, so grab a pointer to it, grab a read lock on the
  // page, and determine if we should reload the page data.  We do not
  // however know for sure whether we need to reload the page, so mark
  // the reload to be conditional; some other thread might be going
  // through these same motions.
  if (! pg)
  {
    pg = &dpos->second;
    pthread_rwlock_rdlock(&pg->lock);
    reload = pg->bytes.empty();
  }

  // Now try to bring the page bytes into memory and grab a read lock
  // on them so nobody can purge while we are looking at this page.
  //
  // We either just created the page, already hold a page write lock
  // and know for sure we have to reload the data, or we have a read
  // lock which we have to upgrade to a write lock, and we then have
  // to check if the data still needs to be loaded.
  //
  // Once we have a write lock on the page and have read in the bytes
  // we need to downgrade the page lock to a read lock again.  This
  // means we lose the lock -- and someone else could release the
  // bytes while we release and re-grab our lock.  So we loop.
  while (reload)
  {
    // Upgrade read lock to write lock if necessary.  Verify is
    // turned on if we don't have a write lock on the page, at the
    // beginning if we came in on an existing page, or around the
    // loop if we lost the page while downgrading the write lock.
    if (upgrade)
    {
      pthread_rwlock_unlock(&pg->lock);
      pthread_rwlock_wrlock(&pg->lock);
      reload = pg->bytes.empty();
      upgrade = false;
    }

    // Read page data.
    if (reload)
      try
      {
        file_->get(addr, pg->bytes);
      }
      catch (...)
      {
        pg->bytes.resize(0);
        pthread_rwlock_unlock(&pg->lock);
        pthread_rwlock_unlock(&file_->lock_);
        throw;
      }

    // Set up pointers.
    ASSERT(pg->bytes.size() > 3 * sizeof(uint32_t) + sizeof(uint64_t));
    uint32_t *end = (uint32_t *)(&pg->bytes[0] + pg->bytes.size());
    pg->nobjs = end[-1];
    pg->objlocs = &end[-2];
    pg->objkeys = ((uint64_t *)&pg->objlocs[-pg->nobjs]) - 1;
    pg->ndata = pg->objlocs[-pg->nobjs];
    pg->first = &pg->bytes[0] + pg->objlocs[0];

    // Now downgrade to a read lock.  This means we could lose the
    // page bytes (but not the page, because we are still holding the
    // file lock).  So check if we were unlucky and have to reload the
    // data again.
    pthread_rwlock_unlock(&pg->lock);
    pthread_rwlock_rdlock(&pg->lock);
    reload = pg->bytes.empty();
    upgrade = true;
  }

  // OK, we got the page data.  Now release the file lock and exit.
  // We are still holding a read lock on the page.
  pthread_rwlock_unlock(&file_->lock_);
  return pg;
}

// ----------------------------------------------------------------------
/** Thread to compress and write data to the file on the background. */
void *
VisDQMFile::WriteHead::writeback(void *arg)
{
  ZCompressor *deflate = 0;
  WriteBuf *buf = (WriteBuf *)arg;
  WriteHead *self = buf->owner;
  VisDQMFile *file = self->file_;
  size_t idx = buf - self->pending_;

  while (true)
  {
    // Wait for action request.
    pthread_mutex_lock(&self->buflock_);
    while (buf->action == WRITE_NOREQ)
      pthread_cond_wait(&self->bufwakeup_, &self->buflock_);

    // Exit if so requested.
    if (buf->action == WRITE_QUIT)
    {
      buf->action = WRITE_NOREQ;
      buf->status = WRITE_IDLE;
      pthread_mutex_unlock(&self->buflock_);
      delete deflate;
      return 0;
    }

    // Caller marked me busy, now let go of the lock.
    ASSERT(buf->status == WRITE_BUSY);
    pthread_mutex_unlock(&self->buflock_);

    // Work request.  Compress into temporary buffer if requested,
    // however store uncompressed if it requires less space that way.
    // The ixentry.address is used to determine whether the data
    // should be compressed or not.
    //
    // On disk compressed blocks are stored with a 32-bit uncompressed
    // size and CRC32 check sum, followed by the compressed data.  Other
    // data is stored as is, without the length and checksum prefixes.
    //
    // FIXME: Store length + checksum also for uncompressed data?
    VisDQMBuf *data = &buf->data;
    Address &at = buf->ixentry.address;
    try
    {
      switch (at.options & OPT_COMPRESSION_MASK)
      {
      case OPT_COMPRESSION_ZLIB:
	{
	  IOSize n;
	  buf->workbuf.resize(at.length + 2*sizeof(uint32_t));
	  if (! deflate)
	    deflate = new ZCompressor(ZConstants::BEST_COMPRESSION);
	  deflate->input(&buf->data[0], at.length);
	  deflate->finish();
	  n = deflate->compress(&buf->workbuf[2*sizeof(uint32_t)], at.length);
	  if (deflate->finished())
	  {
	    CRC32 crc32;
	    crc32.update(&buf->workbuf[2*sizeof(uint32_t)], n);
	    unsigned checksum = crc32.value();

	    buf->workbuf[0] = (at.length >> 24) & 0xff;
	    buf->workbuf[1] = (at.length >> 16) & 0xff;
	    buf->workbuf[2] = (at.length >>  8) & 0xff;
	    buf->workbuf[3] = (at.length      ) & 0xff;

	    buf->workbuf[4] = (checksum  >> 24) & 0xff;
	    buf->workbuf[5] = (checksum  >> 16) & 0xff;
	    buf->workbuf[6] = (checksum  >>  8) & 0xff;
	    buf->workbuf[7] = (checksum       ) & 0xff;

	    at.length = n + 2*sizeof(uint32_t);
	    data = &buf->workbuf;
	  }
	  else
	    at.options &= ~OPT_COMPRESSION_MASK;
	  deflate->reset();
	}
	break;

      default:
	buf->error = new VisDQMError
		     (0, file->path_.name(),
		      StringFormat
		      ("[o=%1:n=%2:f=%3]: request for unrecognised"
		       " compression method %4")
		      .arg((long) at.offset).arg(at.length).arg(at.options)
		      .arg(at.options & OPT_COMPRESSION_MASK));
	break;
      }
    }
    catch (Error &e)
    {
      buf->error = new VisDQMError(0, file->path_.name(), e.explain());
    }
    catch (std::exception &e)
    {
      buf->error = new VisDQMError(0, file->path_.name(), e.what());
    }

    // Write the data out.  Wait for my turn to write, write the data,
    // update the index table, then pass on the writing token.  Then
    // mark this buffer done and wait for more work.  Skip steps if
    // errors have happened above or happen during the write.
    //
    // The slow I/O must occur with lock held to update file position.
    pthread_mutex_lock(&self->buflock_);
    try
    {
      if (! buf->error)
      {
	while (self->nextWriter_ != idx)
	  pthread_cond_wait(&self->bufwakeup_, &self->buflock_);

	file->putraw(at, *data);
	file->indexData_.append(&buf->ixentry, sizeof(buf->ixentry));
      }
    }
    catch (Error &e)
    {
      buf->error = new VisDQMError(0, file->path_.name(), e.explain());
    }
    catch (std::exception &e)
    {
      buf->error = new VisDQMError(0, file->path_.name(), e.what());
    }

    buf->data.resize(0);
    buf->workbuf.resize(0);
    buf->action = WRITE_NOREQ;
    buf->status = WRITE_IDLE;
    self->nextWriter_ = (self->nextWriter_ + 1) % MAX_PENDING;
    pthread_cond_broadcast(&self->bufwakeup_);
    pthread_mutex_unlock(&self->buflock_);
  }
}

/** Wait for a write buffer to become available.  This must be called
    with the buflock_ held.  If the background writer had an error,
    releases buflock_ and throws an exception. */
void
VisDQMFile::WriteHead::reapbuf(WriteBuf &buf)
{
  // Wait while the buffer is busy.
  while (buf.status == WRITE_BUSY)
    pthread_cond_wait(&bufwakeup_, &buflock_);

  ASSERT(buf.status == WRITE_IDLE);
  ASSERT(buf.action == WRITE_NOREQ);

  // If there was an error, throw it now.
  if (buf.error)
  {
    VisDQMError err(*buf.error);
    delete buf.error;
    pthread_mutex_unlock(&buflock_);
    throw err;
  }

  // Release working memory if we are not reusing it immediately.
  if (&buf != &pending_[nextPending_])
  {
    buf.data.release();
    buf.workbuf.release();
  }
}

/** Return the next write buffer in the circular pending_ list.  Waits
    to return only once the buffer is free for re-use. */
VisDQMFile::WriteHead::WriteBuf *
VisDQMFile::WriteHead::initbuf(void)
{
  // Wait until the next buffer becomes free.
  size_t next = nextPending_;
  reapbuf(pending_[next]);

  // Mark this buffer in use.
  ASSERT(! pending_[next].error);
  ASSERT(pending_[next].action == WRITE_NOREQ);
  ASSERT(pending_[next].status == WRITE_IDLE);
  ASSERT(pending_[next].data.empty());
  ASSERT(pending_[next].workbuf.empty());
  ASSERT(next == nextPending_);
  nextPending_ = (next + 1) % MAX_PENDING;

  // Tell caller which buffer to use.
  return &pending_[next];
}

/** Wait for all outstanding writes to complete. */
void
VisDQMFile::WriteHead::flushbufs(void)
{
  pthread_mutex_lock(&buflock_);
  for (size_t i = 0; i < MAX_PENDING; ++i)
    reapbuf(pending_[i]);
  pthread_mutex_unlock(&buflock_);
}

/** Helper method to rewind a write head after a page has been
    flushed.  Re-initialises the head to start filling in the internal
    buffer from start again. */
void
VisDQMFile::WriteHead::rewind(void)
{
  if (page_.bytes.empty())
    page_.bytes.resize(pageSize_);

  next_ = page_.first = &page_.bytes[0]; // FIXME: bias alignment
  page_.objlocs = ((uint32_t *)(&page_.bytes[0] + page_.bytes.size())) - 2;
  page_.objkeys = 0;
  page_.ndata = 0; // FIXME: bias alignment
  page_.nobjs = 0;
  keys_.clear();
}

/** Flush a page to disk, marking it with @a flags as either
    #OPT_PAGE_FILLED_FULL or #OPT_PAGE_FILLED_PARTIAL.

    Before the page data is written to disk it's compacted to
    the minimum possible size, storing the leaf-level B-tree
    index - keys and page-relative object locations - at the
    end of the data.  The caller must ensure there is enough
    space on the page after the data section for the tables.

    Adds a new index entry for the page to the file-level indirect
    B-tree.  Returns after invoking #rewind() so the page is again
    ready for adding new objects. */
void
VisDQMFile::WriteHead::flush(uint32_t flags)
{
  // Skip if the page is empty.
  if (page_.nobjs == 0)
    return;

  // Compact the page.  Write keys and locations immediately after the
  // data on the page, shrinking the page to minimum required size.
  uint32_t offset = roundup(page_.ndata, sizeof(uint64_t));
  uint64_t *keys = (uint64_t *) (&page_.bytes[0] + offset);
  uint32_t *locs = (uint32_t *) (keys + page_.nobjs);
  *locs++ = page_.ndata;
  for (int32_t i = page_.nobjs-1; i >= 0; --i)
  {
    *keys++ = keys_[i];
    *locs++ = page_.objlocs[-i];
  }
  *locs++ = page_.nobjs;

  // Hand compression and write work to a background worker thread.
  pthread_mutex_lock(&buflock_);
  WriteBuf *buf = initbuf();
  buf->ixentry.address.offset = 0;
  buf->ixentry.address.length = (unsigned char *) locs - &page_.bytes[0];
  buf->ixentry.address.options = OPT_INDEX_LEVEL_LEAF
				 | OPT_COMPRESSION_ZLIB
				 | flags;
  buf->ixentry.key = keys_[page_.nobjs-1];
  buf->data.swap(page_.bytes);
  buf->action = WRITE_WORK;
  buf->status = WRITE_BUSY;
  if (buf->thread == (pthread_t)-1
      && pthread_create(&buf->thread, 0, &writeback, buf))
  {
    pthread_mutex_unlock(&buflock_);
    throw VisDQMError(0, file_->path_.name(),
		      "failed to start background thread");
  }
  pthread_cond_broadcast(&bufwakeup_);
  pthread_mutex_unlock(&buflock_);

  // Now rewind page pointers.
  rewind();
}

/** Create a file write head for @a file, which must be a non-null
    pointer to a file which has been opened for writing.  No other
    read or write head must be active on the file at the same time and
    no other thread may be using the file object in any way while this
    write head is active on the file.

    By default data is written to 2MB pages compressed with maximum
    ZLIB compression.  The page size can be modified giving a non-zero
    @a pagesize argument.  The page should be kept fairly large, in
    megabyte range, for best performance.

    While data is written to the pages, the keys for the objects are
    kept on a side in a separate buffer.  The size of this buffer is
    by default 65536 keys, but it will expand as necessary.  If the
    caller specifies a very large page and/or stores many very small
    objects per page, non-zero @a numkeys argument can be used to
    specify a larger initial allocation for key space.

    Normal write iteration pattern for writing objects Foo is:

    @code
       VisDQMFile::WriteHead wrhead(file);
       for (uint64_t key = 0; key < n; ++key)
       {
         void *begin, *end;
         wrhead.alloacte(key, sizeof(Foo), &begin, &end);
	 Foo *obj = (Foo *)begin;
	 obj->bar = xyz;
       }
       wrhead.finish();
    @endcode */
VisDQMFile::WriteHead::WriteHead(VisDQMFile *file,
				 size_t pagesize /* = 0 */,
				 size_t numkeys /* = 0 */)
  : file_(file),
    next_(0),
    nextPending_(0),
    nextWriter_(0),
    pageSize_(pagesize)
{
  ASSERT(file);
  keys_.reserve(numkeys ? numkeys : 32*1024);
  if (! pageSize_)
    pageSize_ = 2*1024*1024;

  for (size_t i = 0; i < MAX_PENDING; ++i)
  {
    memset(&pending_[i].ixentry, 0, sizeof(IndexEntry));
    pending_[i].owner = this;
    pending_[i].action = WRITE_NOREQ;
    pending_[i].status = WRITE_IDLE;
    pending_[i].thread = (pthread_t)-1;
    pending_[i].error = 0;
  }

  page_.offset = 0;
  pthread_mutex_init(&buflock_, 0);
  pthread_cond_init(&bufwakeup_, 0);
  rewind();
}

/** Destroy a write head.  If the head hasn't been released yet, calls
    #finish() to release it and to flush data to the file. */
VisDQMFile::WriteHead::~WriteHead(void)
{
  if (file_)
    finish();
}

/** Allocate a new object with @a key as identification and of size @a
    amount.  The @a start and @a end will point to the object storage
    and one byte past the end, respectively.  */
void
VisDQMFile::WriteHead::allocate(uint64_t key,
				uint32_t amount,
				void **start,
				void **end)
{
  // Allocate space for the object.  If there isn't enough space left
  // on the page, flush the page (updating the B-tree for it), and
  // start a new page.  If even then there isn't enough space, make
  // the page bigger to fit the requested object (rounded to 128kB).
  size_t needed = roundup(page_.ndata + amount, sizeof(uint64_t))
		  + (page_.nobjs+1) * sizeof(uint64_t)
		  + (page_.nobjs+3) * sizeof(uint32_t);

  if (page_.bytes.size() < needed)
    flush(OPT_PAGE_FILLED_FULL);

  needed = roundup(page_.ndata + amount, sizeof(uint64_t))
	   + (page_.nobjs+1) * sizeof(uint64_t)
	   + (page_.nobjs+3) * sizeof(uint32_t);

  if (page_.bytes.size() < needed)
  {
    page_.bytes.resize(roundup(needed, 128*1024));
    rewind();
  }

  // Remember the object key, the location of this object on this
  // page, and bump the amount of data space used and objects stored
  // on this page.
  keys_.push_back(key);
  page_.objlocs[-(int32_t)page_.nobjs] = next_ - &page_.bytes[0];
  page_.ndata += amount;
  page_.nobjs++;

  // Tell the caller where their object is and bump our pointer where
  // to allocate the next object.
  *start = next_;
  next_ += amount;
  *end = next_;
}

/** Transfer data from another file to this one.  Copies from the
    read head @a from upto but not including key @a until.  The
    read head position after the copy has finished is returned in
    @a key, @a start and @a end, as in #ReadHead::get().

    The transfer copies full compressed pages as-is as long as
    possible.  The fast copy occurs as long as the read head is known
    to be at the exact page boundary, and the requested transfer range
    spans all the keys on that page, and the page was full when it was
    flushed out.  Once any of those conditions stops being true, the
    transfer switch to slow object-by-object copy mode.  Note that if
    the copy starts or degrades into the slow mode, it will not switch
    back to the fast mode.

    In practice this means that appending data by copying a leading
    part of the file occurs at I/O speed.  Removing or extending data
    in the middle is slow, as all the remaining data will be expanded
    and re-compressed. */
void
VisDQMFile::WriteHead::xfer(ReadHead &from,
			    uint64_t until,
			    uint64_t *key,
			    void **start,
			    void **end)
{
  if (! from.file_)
    return;

  // Do alternate full and partial page copies.  There should be at
  // most one full and partial copy cycle each: either a leading a
  // full copy ending in an optional partial copy, or an optional
  // ppartial copy ending in a full copy to transfer rest of the file.
  //
  // First copy data pages forward without re-compressing them, as
  // long as we have a page known to be outside interesting index
  // range.  This avoids having to read and re-compress data.
  //
  // Then copy partial page data to reach the requested point.  Then
  // if we transition back to a full page, flush what was written out
  // so far and switch back to a full page copy mode.  Right now we
  // mark the page written out so far as full, even though that may
  // not be true; this avoids falling back to slow mode on subsequent
  // file read.  Consider some sort of data consolidation later.
  while (true)
  {
    // Flush all background writing before transferring data.
    flushbufs();

    // First try fast copying full pages.
    pthread_rwlock_rdlock(&from.file_->lock_);
    while (from.move_ == MOVE_BOUNDARY
	   && until > from.index_->key
	   && ((from.index_->address.options & OPT_PAGE_FILLED_MASK)
	       == OPT_PAGE_FILLED_FULL))
    {
      IndexEntry ixentry = *from.index_;
      file_->xfer(ixentry.address, from.file_, from.index_->address);
      file_->indexData_.append(&ixentry, sizeof(ixentry));

      if (++from.index_ == from.file_->indexEnd_)
	from.move_ = MOVE_NONE;
    }
    pthread_rwlock_unlock(&from.file_->lock_);

    // Slow copy until we reach the desired key or end of input.
    while (true)
    {
      if (from.isdone())
	return;

      from.get(key, start, end);
      if (*key >= until)
	return;

      void *wrstart, *wrend;
      size_t len = (char *) *end - (char *) *start;
      allocate(*key, len, &wrstart, &wrend);
      memcpy(wrstart, *start, len);
      from.next();

      // If we fell off page, switch back to fast copy.
      if (from.move_ == MOVE_BOUNDARY)
      {
	flush(OPT_PAGE_FILLED_FULL);
	break;
      }
    }
  }
}

/** Indicate end of write cycle on the file.  This flushes any
    remaining buffered data to the file.  It is possible to continue
    writing to the file afterwards with another write head. */
void
VisDQMFile::WriteHead::finish(void)
{
  // Flush current page (if any).
  flush(OPT_PAGE_FILLED_PARTIAL);

  // Flush all background writing.
  flushbufs();

  // Tell worker threads to quit.
  pthread_mutex_lock(&buflock_);
  for (size_t i = 0; i < MAX_PENDING; ++i)
  {
    pending_[i].action = WRITE_QUIT;
    pending_[i].status = WRITE_BUSY;
  }
  pthread_cond_broadcast(&bufwakeup_);
  pthread_mutex_unlock(&buflock_);

  // Wait for worker threads to quit.
  for (size_t i = 0; i < MAX_PENDING; ++i)
    if (pending_[i].thread != (pthread_t)-1)
      pthread_join(pending_[i].thread, 0);

  // Disengage from file.
  file_ = 0;
}
