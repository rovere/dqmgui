#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMCache.h"
#include "DQM/VisDQMFile.h"
#include "DQM/VisDQMError.h"
#include "classlib/iobase/File.h"
#include "classlib/iobase/FileError.h"
#include "classlib/iobase/IOStatus.h"
#include "classlib/utils/SystemError.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/Error.h"
#include <memory>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>

using namespace lat;

/** Initialise an index object, using @a path as the directory name
    and @a cache to optimise reads. */
VisDQMIndex::VisDQMIndex(const Filename &path, VisDQMCache *cache /* = 0 */)
  : path_(path),
    cache_(cache),
    mtime_(0),
    update_(false),
    generation_(0),
    master_(new VisDQMFile),
    newmaster_(new VisDQMFile)
{
  pthread_rwlock_init(&lock_, 0);
}

/** Destroy the index. */
VisDQMIndex::~VisDQMIndex(void)
{
  delete master_;
  delete newmaster_;
}

/** Initiate an index read transaction, filling @a master with a
    pointer to the current master catalogue file.  The #VisDQMIndex
    owns the master catalogue file; the caller is free to read it,
    but not write to it, or close, re-open or destroy it. */
void
VisDQMIndex::beginRead(VisDQMFile *&master)
{
  char buf[32];
  struct flock fl;

  // Clear the cache, we are going to invalidate it.
  if (cache_)
    cache_->flush();

  // Open generation, lock for read, and read current generation id.
  File gen(Filename(path_, "generation"), IOFlags::OpenRead);
  fl.l_type = F_RDLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fl.l_pid = 0; // unused
  if (fcntl(gen.fd(), F_SETLKW, &fl) < 0)
    throw FileError("fcntl()", errno);

  gen.read(&generation_, sizeof(generation_));

  // Now while still holding read lock, open the master file.
  sprintf(buf, "master-%" PRIu32 ".dqm", (unsigned) generation_);
  master_->open(~0llu, Filename(path_, buf), VisDQMFile::OPEN_READ, cache_);

  // Close generation, releasing read lock.
  gen.close();

  master = master_;
}

/** Close an index read transaction.  This invalidates the master
    catalogue returned by #beginRead() and the caller may no longer
    use it. */
void
VisDQMIndex::finishRead(void)
{
  master_->close();
}

/** Begin an index update transaction, filling @a master with the
    pointer to the current master catalogue file, and @a newmaster
    with a pointer to the new catalogue the caller should fill in.
    Both pointers are owned by the index; the caller is free to read
    the current master catalogue and write to the new one, but may not
    write to the current one, or close, re-open or destroy either.

    Only a single writer may concurrently even attempt to write to the
    index.  A second writer will happily overwrite the files written
    by the first writer, and while the overlapping writes will likely
    be detected and reported as an error, the index contents are very
    likely damaged by the parallel writes. */
void
VisDQMIndex::beginUpdate(VisDQMFile *&master, VisDQMFile *&newmaster)
{
  char buf[32];
  struct flock fl;

  // Open generation, lock for read, and read current generation id.
  File gen(Filename(path_, "generation"), IOFlags::OpenRead);
  fl.l_type = F_RDLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fl.l_pid = 0; // unused
  if (fcntl(gen.fd(), F_SETLKW, &fl) < 0)
    throw FileError("fcntl()", errno);

  gen.read(&generation_, sizeof(generation_));

  // Now while still holding read lock, open the master files.
  sprintf(buf, "master-%" PRIu32 ".dqm", generation_);
  master_->open(~0llu, Filename(path_, buf), VisDQMFile::OPEN_READ, cache_);

  sprintf(buf, "master-%" PRIu32 ".dqm", generation_+1);
  newmaster_->open(~0llu, Filename(path_, buf), VisDQMFile::OPEN_WRITE, 0);

  // Close generation, releasing read lock.
  gen.close();

  master = master_;
  newmaster = newmaster_;
}

/** Cancel and roll back an index update transaction.  This aborts the
    changes to the new master catalogue.  The caller must delete any
    other new files they started to write.  This invalidates both the
    master catalogues returned by #beginUpdate() and the caller may no
    longer use them. */
void
VisDQMIndex::cancelUpdate(void)
{
  Filename::remove(newmaster_->path(), false, true);
  master_->close();
  newmaster_->close();
}

/** Commit an index update transaction.  First verifies the index is
    still at the same generation as when the update started (i.e. no
    other writers are interfering), and if so, atomically upgrades the
    index generation to the new master catalogue which the caller must
    have completed writing.  The caller must also have finished
    writing and closed all new data files referred to by the new
    master catalogue. */
void
VisDQMIndex::commitUpdate(void)
{
  uint32_t oldgen;
  struct flock fl;

  // Open generation, lock for read, and read current generation id.
  File gen(Filename(path_, "generation"),
	   IOFlags::OpenRead | IOFlags::OpenWrite);
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fl.l_pid = 0; // unused
  if (fcntl(gen.fd(), F_SETLKW, &fl) < 0)
    throw FileError("fcntl()", errno);

  gen.read(&oldgen, sizeof(oldgen), 0);

  // Verify the generation has not changed.
  if (oldgen != generation_)
    throw VisDQMError(0, path_.name(),
		      StringFormat("index generation changed while updating"
				   ", generation at the start of the update"
				   " was %1, but is %2 at the commit")
		      .arg(generation_).arg(oldgen));

  // Commit the new generation.
  newmaster_->close();

  ++generation_;
  gen.write(&generation_, sizeof(generation_), 0);

  // Close generation, releasing the lock.
  gen.close();

  // Now remove the old master.
  Filename::remove(master_->path(), false, true);
  master_->close();
}

/** Check the size of a data file in the index.

    @param kind     #MASTER_FILE_INFO or #MASTER_FILE_DATA.
    @param index    Running file index.
    @param version  Version of the file.

    @return File size in bytes, or zero if the file doesn't exist. */
IOOffset
VisDQMIndex::size(uint16_t kind, uint16_t index, uint16_t version)
{
  IOStatus stat;
  char buf[128];
  if (kind == MASTER_FILE_INFO)
    sprintf(buf, "info/%03d/%05d-%05d.dqm", index / 1000, index, version);
  else if (kind == MASTER_FILE_DATA)
    sprintf(buf, "data/%03d/%05d-%05d.dqm", index / 1000, index, version);
  else
    throw VisDQMError(0, path_.name(),
		      StringFormat("request to open unrecognised file"
				   " [kind:%1, index:%2, version:%3]")
		      .arg(kind).arg(index).arg(version));

  return stat.fill(Filename(path_, buf)) ? stat.m_size : 0;
}

/** Open a data file in the index.

    If the file is opened for reading and the file does not exist, the
    method returns a null pointer.  As you can pass a null pointer to
    #VisDQMFile::ReadHead, this is a convenience that allows the
    caller not to care whether a previous generation exists or not.

    If the file is opened for writing, the requested file is created
    and opened for writing; it may not be read from while written to.
    If the requested file already exists, it is truncated, which
    allows the callers to restart failed / died / aborted transactions
    which did not manage to fully clean up after themselves, however
    the caller must be absolutely sure only a single writer will ever
    attempt to update the index concurrently!

    @param kind     #MASTER_FILE_INFO or #MASTER_FILE_DATA.
    @param index    Running file index.
    @param version  Version of the file.
    @param mode     File open mode.

    @return Pointer to the requested file, or null if the file was
    opened for reading and the file does not exist.  The caller is
    responsible for closing and destroying the object. */
VisDQMFile *
VisDQMIndex::open(uint16_t kind,
		  uint16_t index,
		  uint16_t version,
		  VisDQMFile::OpenMode mode)
{
  char buf[128];
  if (kind == MASTER_FILE_INFO)
    sprintf(buf, "info/%03d/%05d-%05d.dqm", index / 1000, index, version);
  else if (kind == MASTER_FILE_DATA)
    sprintf(buf, "data/%03d/%05d-%05d.dqm", index / 1000, index, version);
  else
    throw VisDQMError(0, path_.name(),
		      StringFormat("request to open unrecognised file"
				   " [kind:%1, index:%2, version:%3]")
		      .arg(kind).arg(index).arg(version));

  Filename path(path_, buf);
  if (mode == VisDQMFile::OPEN_READ && ! path.exists())
    return 0;
  else
  {
    Filename::makedir(path.directory(), 0755, true, true);
    std::auto_ptr<VisDQMFile> f(new VisDQMFile);
    f->open(uint64_t(kind) << 32 | uint64_t(index) << 16 | version,
	    path, mode, (mode == VisDQMFile::OPEN_READ ? cache_ : 0));
    return f.release();
  }
}

/** Initialise a new, empty index.  The caller must have created the
    index directory itself, however the directory must be empty with
    no files in it at all. */
void
VisDQMIndex::initialise(void)
{
  struct flock fl;

  // Create empty master index file.
  VisDQMFile master;
  master.open(~0ul, Filename(path_, "master-0.dqm"), VisDQMFile::OPEN_WRITE, 0);
  master.close();

  // Create initial generation index file.
  File gen(Filename(path_, "generation"),
	   IOFlags::OpenRead | IOFlags::OpenWrite
	   | IOFlags::OpenCreate | IOFlags::OpenTruncate
	   | IOFlags::OpenExclusive);

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fl.l_pid = 0; // unused
  if (fcntl(gen.fd(), F_SETLKW, &fl) < 0)
    throw FileError("fcntl()", errno);

  uint32_t genid = 0;
  gen.write(&genid, sizeof(genid), 0);
  gen.close();
}
