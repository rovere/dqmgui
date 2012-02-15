#ifndef DQM_VISDQMINDEX_H
# define DQM_VISDQMINDEX_H

# include "DQM/VisDQMFile.h"
# include "classlib/iobase/Filename.h"
# include "classlib/utils/Time.h"
# include <pthread.h>
# include <map>

class VisDQMCache;

/** A multiple-readers, single-writer index of DQM data.  The DQM GUI
    only uses DQM data in this special format; it never uses the data
    from the actual ROOT files for efficiency and flexibility reasons.

    This object manages the overall on-disk index scheme of data in
    the DQM format for both reads and index updates.  The read and
    write heads of #VisDQMFile provide access to actual data.  The
    data layout inside the files is documented below.

    The on-disk index can be simultaneously read by any number of
    processes and written by one (and only one) process.  Technically
    the index is a directory containing one well-known generation ID
    identifying current master catalogue which in turn references the
    data files with the actual data content.  The master catalogue
    identifies data samples by a (dataset; runnr or cmssw version)
    tuple and also contains a global enumeration of known monitor
    element names.  The index contains two data files per dataset: one
    for summary and histogram statistics information, another for
    serialised ROOT monitor element objects.  All the files are
    compressed and a simple file naming convention is used to place
    files on disk.

    Each index generation is read and updated atomically such that the
    reader processes always see a fully consistent state of the index
    for that generation only.  This is achieved first by writing all
    new data - including updates of previous data - to new uniquely
    named files, second by the ordering of index update steps, and
    third by an atomic update of the current index generation ID.
    This is in overall similar to how any transactional system works,
    simplified for at most a single concurrent writer assumption.

    Specifically the index is updated as follows, "new file" always
    referring to creation of a fresh uniquely named file: 1) write new
    data to new files, if updating leave old files untouched and copy
    the data to new file updating it as required; 2) write a new
    updated master catalogue; 3) write out the index generation id
    atomically, by acquiring an exclusive lock on the id file before
    updating it; 4) remove now unused old data files, including the
    old master catalogue.

    Readers should periodically check the index generation by opening
    the id file, attempt to get a read lock on it, if successful read
    the generation and open the corresponding master catalogue, and
    release the lock.  This process can be optimised to first check
    the time stamp on the generation id; if the file has not changed,
    it does not need to be opened at all.  Readers must keep open all
    the files whose data they use; the data contents remain valid and
    consistent as long as those files remain open.  If a reader fails
    to open a data file referenced by the master catalogue (meaning an
    updater deleted the file, so there must be a new index generation,
    or the index was removed entirely), the reader should retry by
    reading the generation again to acquire an update.

    If a reader already has a master catalogue open, has not
    experienced file open errors, and fails to acquire the read lock
    on the generation id, it can proceed on the assumption that its
    already open data files, including the master catalogue, is still
    valid.  If the reader has no files open or has experienced a file
    open error, it can either wait until it can acquire the read lock
    on the generation id, or pretend the data did not exist, which
    ever is more convenient.

    Whenever reader upgrades the index generation id, it must close
    all its previously open data files and discard all data it had
    cached in memory, and re-read everything starting from the new
    master catalogue.

    The technical structure of individual cache files is explained in
    #VisDQMFile.  #VisDQMCache can be used to improve performance by
    caching recently accessed file contents.  The rest of this
    documentation describes the logical structure of the index data.

    The master catalogue contains the following information.  The data
    is partitioned so that each part has a different value in the
    upper 32 bits of the 64-bit object key.  Objects of type UTF8 are
    UTF8 strings not terminated by a null byte.

    - Key #MASTER_SAMPLE_RECORD, type #Sample: All the data samples so
      far imported into this index.  A "sample" is a unit of CMS data
      for which there is a collection of DQM objects.  For detector
      data, a sample is a dataset and run.  For release validation a
      sample is a dataset and CMSSW version.  For other monte carlo
      data, a sample is a dataset.

    - Key #MASTER_SOURCE_PATHNAME, type UTF8: The names of the source
      files from which the DQM data was most recently imported.

    - Key #MASTER_DATASET_NAME, type UTF8: The names of the datasets,
      referenced by #Sample.

    - Key #MASTER_CMSSW_VERSION, type UTF8: The names of the CMSSW
      releases, referenced by release validation samples.

    - Key #MASTER_OBJECT_NAME, type UTF8: The full names of all the
      monitor element objects stored anywhere in the index.

    - Key #MASTER_TSTREAMERINFO, binary data: Serialised ROOT
      TStreamerInfo objects for TH1F, TH1S, TH2F, TH2S, TH3F, TH3S,
      TProfile and TProfile2D types.  The sample's streamer info
      should be used whenever reading in ROOT object data for the
      sample.  All data originating from a particular ROOT version
      will share the streamer information.

   In addition to the master catalogue, the index contains two other
   types of files: summary and statistics, and object data.  Each type
   is in separate files, built up to about 500MB per file but such
   that all the data for one sample is in one file.

   Each sample identifies its two files with 32-bit numbers: the high
   16 bits specify file index and the low 16 bits the file version.
   The files are named <type>/<nnn>/<nnn><mmm>-<vvvvv>.dqm where
   <type> is "info" or "data", <nnn> is the thousands part of the file
   index, <mmm> is the mod 1000 part and <vvvvv> is the version.

   Inside the data files objects are ordered by 64-bit keys. The top
   20 bits are the sample index, followed by 4-bit type identifier,
   20-bit lumi section index and 20-bit object name index.  The sample
   index is the index of this sample in the master catalogue.  The
   type identifier is 0 for run summary in which case the lumi section
   index will be zero.  The type is 1 for lumi range summary, and the
   lumi section index will be the end of the luminository range for
   which the data is valid; the beginning lumi index is not recorded,
   it is implicit from the previous entry.  The object name index is
   the monitor element name in the master catalogue table of names.

   The summary data is made of #Summary records, with space between
   the records for scalar value data and quality reports (if any).
   The scalar value is stored as a UTF8Z string: a UTF8 C string
   terminated in a null byte.  The quality reports are stored as a
   tuple of UTF8Z strings, converted from integer or real as
   necessary, with an extra null at the end of the tuple like unix
   argv and environ.  The first tuple member is the quality test
   status code (cf. #dqm::qstatus in DQMDefinitions.h), the second the
   quality test result (a real), third the quality report name, and
   the fourth and last member the quality report message.  Note that
   older versions of DQM did not store the quality test result in the
   file separately, so the value will be zero.

   The object data file contains two serialised ROOT objects per key:
   the object itself and the reference.  A null pointer is stored for
   a non-existent object.  To extract the objects first read in and
   activate the streamer info for the sample in the master catalogue,
   then simply extract objects from the byte buffer as long as it has
   data left in it.

   Only the monitor element objects present in the original ROOT files
   are stored, so readers should use the B-tree index exclusively to
   scan, locate and identify objects.  No ROOT object is stored for
   the scalar monitor elements.  Note that monitor element names are
   not stored in alphabetical order in the master catalogue, but in
   the order they happen to be indexed.  As this ordering is dependent
   on which files happen to be indexed in which order, the object key
   ordering should not be relied upon under any circumstances.  The
   only guarantee is that the string table in the master catalogue is
   consistent with the data files.  It is however very easy and
   efficient to build and use a #StringAtomTree for the names. */
class VisDQMIndex
{
public:
  // ------------------------------------------------------------
  static const uint64_t MASTER_SAMPLE_RECORD   	   = ((uint64_t) 0) << 32;
  static const uint64_t MASTER_SOURCE_PATHNAME 	   = ((uint64_t) 1) << 32;
  static const uint64_t MASTER_DATASET_NAME    	   = ((uint64_t) 2) << 32;
  static const uint64_t MASTER_CMSSW_VERSION   	   = ((uint64_t) 3) << 32;
  static const uint64_t MASTER_OBJECT_NAME     	   = ((uint64_t) 4) << 32;
  static const uint64_t MASTER_TSTREAMERINFO   	   = ((uint64_t) 5) << 32;

  static const uint16_t MASTER_FILE_INFO           = 0;
  static const uint16_t MASTER_FILE_DATA           = 1;

  static const uint32_t SUMMARY_PROP_TYPE_MASK 	   = 0x000000ff;
  static const uint32_t SUMMARY_PROP_TYPE_SCALAR   = 0x0000000f;
  static const uint32_t SUMMARY_PROP_TYPE_INVALID  = 0x00000000;
  static const uint32_t SUMMARY_PROP_TYPE_INT      = 0x00000001;
  static const uint32_t SUMMARY_PROP_TYPE_REAL     = 0x00000002;
  static const uint32_t SUMMARY_PROP_TYPE_STRING   = 0x00000003;
  static const uint32_t SUMMARY_PROP_TYPE_TH1F 	   = 0x00000010;
  static const uint32_t SUMMARY_PROP_TYPE_TH1S 	   = 0x00000011;
  static const uint32_t SUMMARY_PROP_TYPE_TH1D 	   = 0x00000012;
  static const uint32_t SUMMARY_PROP_TYPE_TH2F 	   = 0x00000020;
  static const uint32_t SUMMARY_PROP_TYPE_TH2S 	   = 0x00000021;
  static const uint32_t SUMMARY_PROP_TYPE_TH2D 	   = 0x00000022;
  static const uint32_t SUMMARY_PROP_TYPE_TH3F 	   = 0x00000030;
  static const uint32_t SUMMARY_PROP_TYPE_TH3S 	   = 0x00000031;
  static const uint32_t SUMMARY_PROP_TYPE_TH3D 	   = 0x00000032;
  static const uint32_t SUMMARY_PROP_TYPE_TPROF	   = 0x00000040;
  static const uint32_t SUMMARY_PROP_TYPE_TPROF2D  = 0x00000041;
  static const uint32_t SUMMARY_PROP_TYPE_DATABLOB = 0x00000050;

  static const uint32_t SUMMARY_PROP_REPORT_MASK   = 0x00000f00;
  static const uint32_t SUMMARY_PROP_REPORT_CLEAR  = 0x00000000;
  static const uint32_t SUMMARY_PROP_REPORT_ERROR  = 0x00000100;
  static const uint32_t SUMMARY_PROP_REPORT_WARN   = 0x00000200;
  static const uint32_t SUMMARY_PROP_REPORT_OTHER  = 0x00000400;
  static const uint32_t SUMMARY_PROP_REPORT_ALARM  = (SUMMARY_PROP_REPORT_ERROR
						      | SUMMARY_PROP_REPORT_WARN
						      | SUMMARY_PROP_REPORT_OTHER);

  static const uint32_t SUMMARY_PROP_HAS_REFERENCE = 0x00001000;
  static const uint32_t SUMMARY_PROP_TAGGED        = 0x00002000;
  static const uint32_t SUMMARY_PROP_ACCUMULATE    = 0x00004000;
  static const uint32_t SUMMARY_PROP_RESET         = 0x00008000;

  static const uint32_t SUMMARY_PROP_NEW           = 0x00010000;
  static const uint32_t SUMMARY_PROP_RECEIVED      = 0x00020000;
  static const uint32_t SUMMARY_PROP_DEAD          = 0x00080000;
  static const uint32_t SUMMARY_PROP_EFFICIENCY_PLOT = 0x00200000;

  static const unsigned STATS_X                    = 0;
  static const unsigned STATS_Y                    = 1;
  static const unsigned STATS_Z                    = 2;
  static const unsigned STATS_MIN                  = 0;
  static const unsigned STATS_MAX                  = 1;

  // ------------------------------------------------------------
  /** Information about one sample (dataset/run). */
  struct Sample
  {
    /** The unix time stamp of the first import to the DQM index. */
    uint64_t		firstImportTime;

    /** The unix time stamp of the most recent import to the DQM index. */
    uint64_t		lastImportTime;

    /** The number of times the index has been refreshed for this sample. */
    uint32_t		importVersion;

    /** The file index of the summary and stats (0), and object data
	(1) files.  The upper 16 bits identify the file index and the
	lower 16 the version of that file. */
    uint32_t		files[2];

    /** B-tree string index delta from #MASTER_SOURCE_PATHNAME for the
	name of the original source from which the last version of
	this DQM data was imported from. */
    uint32_t		sourceFileIdx;

    /** B-tree string index delta from #MASTER_DATASET_NAME for the
	dataset name.  */
    uint32_t		datasetNameIdx;

    /** B-tree string index delta from #MASTER_TSTREAMERINFO for the
	serialised ROOT TStreamerInfo. */
    uint32_t		streamerInfoIdx;

    /** B-tree string index delta from #MASTER_CMSSW_VERSION for the
        name of the CMSSW release for release validation samples. */
    uint32_t		cmsswVersion;

    /** The run number; generally one for monte carlo data. */
    int32_t		runNumber;

    /** Number of monitor element objects in this sample. */
    uint32_t		numObjects;

    /** The number of events over which DQM data was collected. */
    uint64_t		numEvents;

    /** The number of lumi sections over which DQM data was collected. */
    uint64_t		numLumiSections;

    /** The time stamp when the run originally started. */
    uint64_t		runStartTime;

    /** The time stamp of when the DQM data was produced. */
    uint64_t		processedTime;
  };

  // ------------------------------------------------------------
  /** Information about one monitor element in a sample. */
  struct Summary
  {
    /** Type and other properties of the monitor element, including
        quality test summary and alarm condition. */
    uint32_t		properties;

    /** Amount of data that follows for scalar representation, if
	non-zero a null-terminated string of this length follows. */
    uint32_t		dataLength;

    /** Amount of data that follows for quality test result
        representation, if non-zero one or more null-terminated
        strings of this total amount follow (after scalar data if
        any). */
    uint32_t		qtestLength;

    /** Uncompressed size of the serialised monitor element ROOT object. */
    uint32_t		objectLength;

    /** DetId tag. */
    uint32_t		tag;

    /** Number of bins along X, Y and Z axes. */
    uint32_t		nbins[3];

    /** Number of histogram entries. */
    double		nentries;

    /** Mean along X, Y and Z axes. */
    double		mean[3];

    /** Standard deviation along X, Y and Z axes. */
    double		rms[3];

    /** Minimum and maximum bounds for X, Y and Z axes. */
    double		bounds[3][2];
  };

  // ------------------------------------------------------------
  VisDQMIndex(const lat::Filename &path, VisDQMCache *cache = 0);
  ~VisDQMIndex(void);

  void			initialise(void);

  void			beginRead(VisDQMFile *&master);
  void			finishRead(void);

  void			beginUpdate(VisDQMFile *&master, VisDQMFile *&newmaster);
  void			cancelUpdate(void);
  void			commitUpdate(void);

  lat::IOOffset		size(uint16_t kind, uint16_t index, uint16_t version);
  VisDQMFile *		open(uint16_t kind,
			     uint16_t index,
			     uint16_t version,
			     VisDQMFile::OpenMode mode);

private:
  /** Read-write lock. */
  pthread_rwlock_t	lock_;

  /** Index directory path. */
  lat::Filename		path_;

  /** Read cache if any. */
  VisDQMCache		*cache_;

  /** Last modification time of the generation file, last we looked. */
  lat::Time		mtime_;

  /** Flag indicating whether we are updating or just reading. */
  bool			update_;

  /** The current index generation, last we looked. */
  uint32_t		generation_;

  /** The master catalogue file.  Always open. */
  VisDQMFile		*master_;

  /** The new master catalogue file if we are updating, or null otherwise. */
  VisDQMFile		*newmaster_;
};

#endif // DQM_VISDQMINDEX_H
