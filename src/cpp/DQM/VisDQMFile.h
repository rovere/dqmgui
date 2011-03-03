#ifndef DQM_VISDQMFILE_H
# define DQM_VISDQMFILE_H

# include "DQM/VisDQMBuf.h"
# include "classlib/zip/ZCompressor.h"
# include "classlib/zip/ZDecompressor.h"
# include "classlib/iobase/Filename.h"
# include "classlib/iobase/File.h"
# include <pthread.h>
# include <stdint.h>
# include <vector>
# include <map>

class VisDQMCache;
class VisDQMError;

/** A DQM data file, storing a single data stream of objects indexed
    with a B-tree like index.  The data is laid out efficiently for
    both sequential and random access, both from application and I/O
    performance point of views.

    The actual payload data is stored in compressed data pages.  The
    uncompressed size of pages is currently 2MB, or larger if an
    individual object requires more.  Objects are indexed to a B-tree
    such that the last (leaf) level of the B-tree is stored on the
    same page as the data itself.  For simplicity the files currently
    contain only one more B-tree indirection level which is stored on
    disk compressed.

    Objects are identified by 64-bit arbitrary keys.  The application
    must arrange to write the objects into the file in strictly
    increasing key order.  Typically each object is an implicit array;
    the arrays are guaranteed to be contiguous in memory.  The objects
    are currently stored and returned exactly as written, without
    conversion of any kind.

    The DQM data files should in general be opened through
    #VisDQMIndex which may be configured with a #VisDQMCache for
    high-performance caching of file contents in memory.

    The data in the file should be read via #ReadHead and written with
    #WriteHead iterator-like interface.  #ReadHead provides a simple
    interface optimised for sequential scan and random access, however
    the latter requires an active #VisDQMCache to be efficient.
    #WriteHead provides an interface to write data out in strictly
    sequential order; the caller must ensure the objects are written
    out in strictly increasing key order.  #WriteHead does however
    support an efficient bulk copy from a #ReadHead to transfer data
    from one file to another without having to uncompress and
    re-compress the contents. */
class VisDQMFile
{
public:
  // ------------------------------------------------------------
  static const uint64_t MAGIC = 0x54494e595354524dULL; //'TINYSTRM'
  static const uint64_t VINFO = 0x1234567800000000ULL; // byte order, v0

  static const uint32_t OPT_COMPRESSION_MASK     = 0x0000000f;
  static const uint32_t OPT_COMPRESSION_NONE     = 0x00000000;
  static const uint32_t OPT_COMPRESSION_ZLIB     = 0x00000001;

  static const uint32_t OPT_INDEX_LEVEL_MASK     = 0x00000010;
  static const uint32_t OPT_INDEX_LEVEL_LEAF     = 0x00000000;
  static const uint32_t OPT_INDEX_LEVEL_BTREE    = 0x00000010;

  static const uint32_t OPT_PAGE_FILLED_MASK     = 0x00000020;
  static const uint32_t OPT_PAGE_FILLED_FULL     = 0x00000000;
  static const uint32_t OPT_PAGE_FILLED_PARTIAL  = 0x00000020;

  /** Maximum number of ongoing background writes. */
  static const size_t MAX_PENDING = 7;

  /** Number of index entries per B-tree indirect index page.  This
      number is chosen so that separate index pages are just under
      4kB, for good CPU L1 data cache use - a single index entry is 24
      bytes (3*64 bits).  Note that as the 2MB data pages contain the
      B-tree leaf level of the objects stored on that page, only a
      handful of separate 4kB index pages are required to address
      hundreds of megabytes of data.  */
  static const int INDEX_PAGE_ENTRIES = 170;

  /** A file address, consisting of a 64-bit offset, 32-bit disk size
      and 32-bit options, for a total size of 128 bits.  The disk size
      is the compressed size plus any headers added such as the
      uncompressed size and checksum.  The options describe other
      information: compression method and whether this is a direct
      pointer to a data page or an indirect one to another B-tree
      level.  Currently the B-tree index is just one level so there
      are no intermediate tree levels. */
  struct Address
  {
    uint64_t		offset;		//< Absolute file offset.
    uint32_t		length;		//< On-disk size compressed.
    uint32_t		options;	//< Page handling options.
  };

  // ------------------------------------------------------------
  /** Container holding actual data page bytes. */
  typedef std::vector<unsigned char> DataBytes;

  /** Temporary container used to hold keys when building pages. */
  typedef std::vector<uint64_t> KeyVector;

  /** An individual data page.  This both holds the actual page data,
      extracted meta information, a page lock and a last use stamp.

      The meta data includes: pointer to the first objcet on the page,
      pointer to object key and offset tables, the number of objects
      and the total number of bytes of data on the page.  The page is
      laid out as follows in memory, from lowest to highest address:
      gap - object data as a linear array - gap - key table - offset
      table - nobjs.  (While writing, the key table is kept separate.)
      The top-most word indicates how many objects there are on the
      page, which is the number of keys and one less than the number
      of offsets; the extra offset indicates where the data area ends.
      The offset and key tables form the leaf level of the B-tree
      index, for the objects stored in the bottom part of that same
      page.  The offset and key tables grow top-down and data grows
      bottom-up.  The area between the data and the tables is a gap
      remaining to be filled in.  The initial gap - a bias to avoid
      cache collisions - is defined by the offset of the first object.

      Clients must acquire and hold a read lock on the page as long as
      they refer to the page data to avoid data from being reclaimed.
      The page reclaim acquires a write lock first on the entire cache
      and then on individual pages, so it's important the readers hold
      read locks on data pages as long as they need the page (readers
      should release the full cache read lock but keep page locks). */
  struct DataPage
  {
    pthread_rwlock_t	lock;		//< Page read-write lock.
    uint64_t		offset;		//< File offset, for caching.
    VisDQMBuf		bytes;		//< Actual page data.
    unsigned char	*first;		//< Pointer to first object.
    uint64_t		*objkeys;	//< Pointer to object key table.
    uint32_t		*objlocs;	//< Pointer to object offset table.
    uint32_t		ndata;		//< Amount of data filled in.
    int32_t		nobjs;		//< Number of obj* table entries.
  };

  /** One B-tree index entry.  The "B-tree" we use is basically just
      an alternating series of 128-bit addresses and 64-bit keys.

      The address gives the file location of a data page on disk, and
      that data page contains all objects with keys up to and
      including the key given in the index.  The last key in the index
      basically gives the "dimension" of the entire data stream.

      The index assumes keys are stored in strictly increasing
      order: monotonically increasing with no repeated keys. */
  struct IndexEntry
  {
    Address		address;
    uint64_t		key;
  };

  /** Data file header.  Currently simplified because we know the file
      layout well enough in advance - one data and one index stream.
      Hence we simply store all the compressed data pages first, and
      the full B-tree index as one big blob at the end.  */
  struct Header
  {
    uint64_t		magic;		//< File magic.
    uint64_t		vinfo;		//< Version and byte order.
    Address		index;		//< The index area.
  };

  // ------------------------------------------------------------
  /** Flag indicating whether the file is opened for reading or
      writing.  A file open for writing can not be read from! */
  enum OpenMode
  {
    OPEN_READ,
    OPEN_WRITE
  };

  /** Flag indicating how to move a read head on next access. */
  enum MoveMode
  {
    MOVE_NONE,     //< No moving needed.
    MOVE_BOUNDARY, //< Move to next page, known to be on boundary.
    MOVE_SEARCH    //< Move to next page, not known where.
  };

  // ------------------------------------------------------------
  class ReadHead;
  class WriteHead;

  /** A read head for reading data from the file.

      A read head can be used for sequential scans of file data, and
      random access to anywhere in the file using the file's B-tree
      index.  Files accessed frequently truly randomly should be be
      backed by a #VisDQMCache for the efficiency.

      Any number of multiple concurrent read heads can be active on
      the same data in the same file at same time, including read
      heads in different threads.  The threads coordinate access so
      that they share the file data, and the data remains accessible
      as long as at least one read head is referring to it.

      Read heads also coordinate with #VisDQMCache so that recently
      accessed data remains uncompressed in memory for some time even
      if no read head is currently looking at that particular data.
      This avoids excessive re-uncompression on random access.

      Note that the file being read cannot be closed until all read
      heads accessing it have been closed.  Clients can use #finish()
      to explicitly release access to the data. */
  class ReadHead
  {
  public:
    ReadHead(VisDQMFile *file, uint64_t key);
    ~ReadHead(void);

    void		get(uint64_t *key, void **start, void **end);
    bool		isdone(void);
    void		next(void);
    void		finish(void);

  private:
    void		move(void);
    DataPage *		pagein(const Address &addr);

    VisDQMFile		*file_;
    DataPage		*page_;
    IndexEntry		*index_;
    uint64_t		target_;
    int32_t		obj_;
    MoveMode		move_;

    friend class WriteHead;
    ReadHead(const ReadHead &other);
    ReadHead&operator=(const ReadHead &other);
  };

  /** A write head to write data to a file.

      A write head is used to fill in a file.  The file must have been
      opened in write mode, there may not be any read heads active on
      the file at the same time, only a single write head can be
      active on a file at a time, and the write head can only move in
      sequential order - random access is not possible.  The caller
      filling in the data must guarantee to store data in strictly
      increasing key order: monotonically growing key values with no
      repeated keys.  Note the keys only need to be increasing, they
      do not have to be consequtive.

      Note that the file being written cannot be closed until a write
      head accessing it has been closed.  Clients can use #finish()
      to explicitly release access to the file. */
  class WriteHead
  {
  public:
    WriteHead(VisDQMFile *file, size_t pagesize = 0, size_t numkeys = 0);
    ~WriteHead(void);

    void		allocate(uint64_t key, uint32_t amount, void **start, void **end);
    void		xfer(ReadHead &from, uint64_t until,
			     uint64_t *key, void **start, void **end);
    void		finish(void);

  private:
    enum WriteAction
    {
      WRITE_NOREQ,
      WRITE_WORK,
      WRITE_QUIT
    };

    enum WriteStatus
    {
      WRITE_IDLE,
      WRITE_BUSY
    };

    struct WriteBuf
    {
      WriteHead		*owner;
      WriteAction	action;
      WriteStatus	status;
      VisDQMBuf		data;
      VisDQMBuf		workbuf;
      IndexEntry	ixentry;
      pthread_t		thread;
      VisDQMError	*error;
    };

    void		rewind(void);
    void		flush(uint32_t flags);
    WriteBuf *		initbuf(void);
    void		reapbuf(WriteBuf &buf);
    void		flushbufs(void);
    static void *	writeback(void *arg);

    /** File we are writing to. */
    VisDQMFile		*file_;

    /** The data we are currently filling. */
    DataPage		page_;

    /** Temporary buffer to collect key data while building a page. */
    KeyVector		keys_;

    /** Next free location. */
    unsigned char	*next_;

    /** Circular pending write buffers for background writing. */
    WriteBuf		pending_[MAX_PENDING];

    /** The write buffer in pending_ to use on next #flush(). */
    size_t		nextPending_;

    /** The write buffer in pending_ to next write out. */
    size_t		nextWriter_;

    /** Lock to gate access to pending_. */
    pthread_mutex_t	buflock_;

    /** Condition variable for background communication. */
    pthread_cond_t	bufwakeup_;

    /** Requested write page size. */
    size_t		pageSize_;

    WriteHead(const WriteHead &other);
    WriteHead&operator=(const WriteHead &other);
  };

  // ------------------------------------------------------------
  VisDQMFile(void);
  ~VisDQMFile(void);

  const lat::Filename &	path(void) const;
  void			open(uint64_t cacheKey, const lat::Filename &path,
			     OpenMode mode, VisDQMCache *cache);
  void			close(void);

  // ------------------------------------------------------------
private:
  friend class ReadHead;
  friend class WriteHead;

  lat::IOSize		get(const Address &at, VisDQMBuf &into);
  void			putraw(Address &at, const VisDQMBuf &from);
  void			put(Address &at, const VisDQMBuf &from);
  void			xfer(Address &at, VisDQMFile *f, const Address &from);

  /** A table of currently cached data pages, keyed by the file offset
      of the page's Address.  Data pages currently held in memory are
      guaranteed to be in this map.  */
  typedef std::map<uint64_t, DataPage> DataMap;

  /** Lock for the entire file, allowing concurrent readers.  Readers
      locating a data page must first acquire the file level lock,
      then lock the page they are interested in, then release the file
      level lock.  The reader must keep the data page lock as long as
      they refer to the page.  The file level write lock must be
      acquired when updating the page map, such as to add or remove a
      data page, and other file-level operations. */
  pthread_rwlock_t	lock_;

  /** The name of this data file. */
  lat::Filename		path_;

  /** The mode in which this file is open. */
  OpenMode		mode_;

  /** The operating system file object for this data file. */
  lat::File		file_;

  /** Read cache if one was specified in #open(). */
  VisDQMCache		*cache_;

  /** Cache key for this file. */
  uint64_t		cacheKey_;

  /** The entire indirect index read into memory. */
  VisDQMBuf		indexData_;
  IndexEntry		*indexBegin_; //< Typed pointer to #indexData_.
  IndexEntry		*indexEnd_;   //< Typed pointer to #indexData_.

  /** The map of data pages present in memory.  See #DataMap. */
  DataMap		data_;

  /** Current write head position. */
  lat::IOOffset		wpos_;
};

#endif // DQM_VISDQMFILE_H
