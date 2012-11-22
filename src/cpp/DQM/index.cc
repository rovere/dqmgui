#define __STDC_FORMAT_MACROS 1

/// Flag indicating whether and how much to debug.
int debug = 0;

#define DEBUG(x, msg) if (debug >= x) std::cout << "DEBUG: " << msg << std::flush

#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMCache.h"
#include "DQM/VisDQMFile.h"
#include "DQM/VisDQMError.h"
#include "DQM/VisDQMTools.h"
#include "DQM/StringAtom.h"
#include "DQM/Standalone.h"
#include "DQM/DQMStore.h"
#include "DQM/MonitorElement.h"
#include "classlib/utils/Regexp.h"
#include "classlib/utils/RegexpMatch.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/StringOps.h"
#include "classlib/utils/DebugAids.h"
#include "classlib/utils/Error.h"
#include "classlib/iobase/Filename.h"
#include "classlib/iobase/FileError.h"
#include "classlib/iobase/File.h"
#include "classlib/zip/MD5Digest.h"
#include "TH1.h"
#include "TAxis.h"
#include "TROOT.h"
#include <cerrno>
#include <cstdlib>
#include <cfloat>
#include <iostream>
#include <fstream>
#include <list>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <float.h>

using namespace lat;

// ----------------------------------------------------------------------
/** Index task to perform. */
enum TaskType
{
  TASK_CREATE,		 //< Create and initialise a new index.
  TASK_ADD,		 //< Add data to an index.
  TASK_REMOVE,		 //< Remove data from the index.
  TASK_MERGE,            //< Merge an index to another.
  TASK_DUMP,		 //< Dump the index contents.
  TASK_STREAM		 //< Stream a sample from the index into an intermediate .dat file.
};

/** Things user can choose to dump out. */
enum DumpType
{
  DUMP_CATALOGUE,	 //< Dump the master catalogue.
  DUMP_INFO,		 //< Dump monitor element summary information.
  DUMP_DATA,		 //< Dump monitor element serialised data.
  DUMP_ALL		 //< Dump everything.
};

/** Classification of what to do with a monitor element. */
enum MEClass
{
  ME_CLASS_BAD,		 //< Did not match expected naming conventions.
  ME_CLASS_SKIP,	 //< Not interesting or already handled some other way.
  ME_CLASS_KEEP		 //< Index this monitor element.
};

/** Classification of a sample to a broad class of DQM data types. */
enum DataType
{
  TYPE_OTHER,		 //< Type not recognised or not determined.
  TYPE_DATA,		 //< DQM data for real detector data.
  TYPE_RELVAL,		 //< DQM data for release validation simulated data.
  TYPE_MC		 //< DQM data for other simulated data.
};

/** Classification of DQM data into a unique DQM GUI "sample". */
struct SampleInfo
{
  uint32_t	index;	 //< Index of this sample among new files.
  int32_t	runnr;	 //< Run number, 0 for mc, > 0 for data.
  DataType	type;	 //< Broad type class of underlying data.
  std::string	dataset; //< Full 3-part dataset name.
  std::string	version; //< CMSSW version string for relval data.
};

/** Classification of input files to DQM samples. */
struct FileInfo
{
  Filename	path;	   //< Path name of the input file (or File name of the root file inside a zip archive).
  Filename	fullpath;  //< File name of the zip archive and ROOT filename.
  Filename	container; //< File name of the zip archive, if any, or of the root file.
  SampleInfo	*sample;   //< Sample classification of this file.
};

/** Information extracted from a monitor element and stored in the index. */
struct MonitorElementInfo
{
  /** Properties as bit flags (#VisDQMIndex::Summary::properties). */
  uint32_t	flags;

  /** Detector tag (#VisDQMIndex::Summary::tag). */
  uint32_t	tag;

  /** Pointer to the ROOT object and a reference if any. */
  TObject       *object[2];

  /** Index in the StringAtomTree holding the ROOT information to be
      stored in the index */
  size_t    	streamidx;

  /** DQM sample sub-catagory: 0 for run, 1 for lumi section range summary.
      Used to create a part of the 64-bit key in the index. */
  uint32_t	category;

  /** First lumi section for a lumi section range summary.  Zero if
      this is not a lumi section range summary object.  Ignored. */
  uint32_t	lumibegin;

  /** Last lumi section for a lumi section range summary.  Zero if
      this is not a lumi section range summary object.  Used to
      create part of the 64-bit key in the index. */
  uint32_t	lumiend;

  /** Full 64-bit key for this monitor element when stored into the
      index, constructed as per documentation in #VisDQMIndex.  The
      value is filled in only when we are about to insert data into a
      file in the index. */
  uint64_t	nameidx;

  /** Full path name of the monitor element, without the decorations
      added by #DQMStore when saving the objects in a ROOT file.  This
      is used to create a part of the 64-bit key in the index. */
  std::string	name;

  /** For scalar monitor elements the string representation of the
      value; stores only the value, without any of the decorations
      added by #DQMStore or #MonitorElement.  Empty for non-scalar
      monitor elements.  Stored in the summary immediately after
      the #VisDQMIndex::Summary object. */
  std::string	data;

  /** The quality report results attached to this monitor element.
      Each result is a tuple of four strings terminated in a null
      character, with an extra null to terminate the tuple (note: this
      means the string has several embedded nulls!).  The first tuple
      member is the quality report status (an integer formatted as a
      string), the second the quality report result (a real formatted
      as a string), the third the quality test name, and the fourth
      the quality test message.  Stored in the summary after the
      scalar data, after the #VisDQMIndex::Summary object. */
  std::string   qreports;

  /** ROOT histogram statistics: number of entries. */
  double	nentries;

  /** ROOT histogram statistics: number of bins in x, y and z. */
  uint32_t	nbins[3];

  /** ROOT histogram statistics: mean in x, y and z. */
  double	mean[3];

  /** ROOT histogram statistics: standard deviation in x, y and z. */
  double	rms[3];

  /** ROOT histogram statistics: min and max bounds in x, y and z. */
  double	bounds[3][2];

  /** ROOT histogram statistics: number of bytes of serialised data. */
  uint32_t	ndata;
};


/** Comparison operator for arranging monitor elements by their output
    order in the file, mainly by the name index in the table. */
class OrderByNameIndex
{
  std::vector<MonitorElementInfo> &minfo_;
public:
  OrderByNameIndex(std::vector<MonitorElementInfo> &minfo)
    : minfo_(minfo)
    {}

  bool operator()(uint32_t a, uint32_t b)
    { return minfo_[a].nameidx < minfo_[b].nameidx; }
};

/// Name of this program for diagnostic messages.
Filename app;

/// Regular expression to recognise a stream file.
Regexp rxstreamfile(".*\\.dat$$");

/// Regular expression to recognise valid dataset names.
Regexp rxdataset("^(/[-A-Za-z0-9_]+){3}$");

/// Regular expression to recognise valid online "real data" DQM files.
/// The first capture is the run number string.
Regexp rxonline("^(?:.*/)?DQM_V\\d+(?:_[A-Za-z0-9]+)?_R(\\d+)\\.(dat|root)$");

/// Regular expression to recognise valid offline DQM data files.  The
/// first capture is the run number, the second mangled dataset name.
Regexp rxoffline("^(?:.*/)?DQM_V\\d+_R(\\d+)((?:__[-A-Za-z0-9_]+){3})\\.(dat|root)$");

/// Regular expression to recognise release validation dataset names.
/// The first capture is the CMSSW release string.
Regexp rxrelval("^/RelVal[^/]+/(CMSSW(?:_[0-9])+(?:_pre[0-9]+)?)[-_].*$");

static const std::string MEINFOBOUNDARY("____MEINFOBOUNDARY____");
static const std::string MEROOTBOUNDARY("____MEROOTBOUNDARY____");
static const size_t ALL_SAMPLES = ~(size_t)0;

// ----------------------------------------------------------------------
/** Utility function to round @a value to a value divisible by @a unit. */
static inline uint32_t
roundup(uint32_t value, uint32_t unit)
{
  return (value + unit - 1) / unit * unit;
}

static bool isStreamFile(const char* filename)
{
  return rxstreamfile.exactMatch(filename);
}

/** Utility function to read a double from a stream. */
static inline void readDouble(ifstream &iread, double *into)
{
  std::string tmp;
  iread >> tmp;
  *into = strtod(tmp.c_str(), 0);
}

/** Utility function to write a double into a stream. */
static inline void writeDouble(ofstream &iwrite,
			       const char *prefix,
			       double val)
{
  char buf[64];
  snprintf(buf, sizeof(buf), "%s%.*g", prefix, DBL_DIG+2, val);
  iwrite << buf;
}

// ----------------------------------------------------------------------
/** Classify and extract monitor element object properties from @a obj
    originating from @a store and sample @a si.  The result is stored
    into @a info.  The return value indicates whether the monitor
    element was recognised and if so, whether it should be stored into
    the DQM index, or ignored as uninteresting (e.g. ReleaseTag) or
    already handled (e.g. references). */
static MEClass
classifyMonitorElement(DQMStore & /* store */,
		       MonitorElement &obj,
		       MonitorElementInfo &info,
		       VisDQMIndex::Sample &meta,
		       const SampleInfo &si)
{
  info.flags = 0;
  info.tag = 0;
  info.streamidx = 0;
  info.object[0] = 0;
  info.object[1] = 0;
  info.category = 0;
  info.lumibegin = 0;
  info.lumiend = 0;
  info.nameidx = 0;
  info.name.clear();
  info.data.clear();
  info.qreports.clear();
  info.nentries = 0;
  for (int i = 0; i < 3; ++i)
  {
    info.nbins[i] = 0;
    info.mean[i] = 0;
    info.rms[i] = 0;
    info.bounds[i][0] = 0;
    info.bounds[i][1] = 0;
  }
  info.ndata = 0;

  // Skip references, they are handled when reading in data.
  const std::string &name = obj.getFullname();
  if (name.size() > 10 && name.compare(0, 10, "Reference/") == 0)
    return ME_CLASS_SKIP;

  // Validate "Run XYZ/System/Category/Name" format where category is
  // either "Run summary" or "By Lumi Section <N>-<M>".  Ignore buggy
  // "Run XYZ/Reference/Name" entries.
  size_t slash, sys, cat;
  if (name.compare(0, 4, "Run ") == 0
      && (slash = name.find('/')) < name.size()-1
      && (sys = name.find('/', slash+1)) < name.size()-1
      && (cat = name.find('/', sys+1)) < name.size()-1)
  {
    if (name.compare(slash+1, 10, "Reference/") == 0)
      return ME_CLASS_SKIP;

    errno = 0;
    char *end = 0;
    int32_t runnr = strtol(name.c_str()+4, &end, 10);
    if (errno != 0 || !end || *end != '/' || runnr < 0)
      return ME_CLASS_BAD;

    if (si.type == TYPE_DATA && si.runnr != runnr)
      return ME_CLASS_BAD;

    if ((si.type == TYPE_RELVAL || si.type == TYPE_MC) && runnr != 1)
      return ME_CLASS_BAD;

    std::string category(name, sys+1, cat-sys-1);
    if (category == "Run summary")
      info.category = 0;
    else if (category.compare(0, 16, "By Lumi Section ") == 0)
    {
      info.category = 1;

      end = 0;
      errno = 0;
      info.lumibegin = strtoul(category.c_str()+16, &end, 10);
      if (errno != 0 || !end || *end != '-'
	  || ! isdigit((unsigned) end[1]))
	return ME_CLASS_BAD;

      info.lumiend = strtoul(end+1, &end, 10);
      if (errno != 0 || !end || *end)
	return ME_CLASS_BAD;
    }
    else
      return ME_CLASS_BAD;
  }
  else if (name == "ReleaseTag")
    return ME_CLASS_SKIP;
  else
    return ME_CLASS_BAD;

  // Now extract all data into "info".  Save the undecorated full
  // path, any detector tag, quality test results, and various flags.
  info.name.reserve(name.size());
  info.name.append(name, slash+1, sys-slash-1);
  info.name.append(1, '/');
  info.name.append(name, cat+1, std::string::npos);

  if (uint32_t tag = obj.getTag())
  {
    info.tag = tag;
    info.flags |= VisDQMIndex::SUMMARY_PROP_TAGGED;
  }

  obj.packQualityData(info.qreports);

  if (obj.hasError())
    info.flags |= VisDQMIndex::SUMMARY_PROP_REPORT_ERROR;

  if (obj.hasWarning())
    info.flags |= VisDQMIndex::SUMMARY_PROP_REPORT_WARN;

  if (obj.hasOtherReport())
    info.flags |= VisDQMIndex::SUMMARY_PROP_REPORT_OTHER;

  if (obj.isEfficiency())
    info.flags |= VisDQMIndex::SUMMARY_PROP_EFFICIENCY_PLOT;

  // Extract the actual object value and fill in type flag in
  // properties.  Save scalar value as a string - just the value not
  // the decorations normally added by DQMStore or MonitorElement -
  // and extract the ROOT object, any reference and high-level
  // statistics for histogram type objects.
  switch (obj.kind())
  {
  default:
    ASSERT(false);
    return ME_CLASS_BAD;

  case MonitorElement::DQM_KIND_INVALID:
    info.flags |= obj.kind();
    return ME_CLASS_BAD;

  case MonitorElement::DQM_KIND_INT:
  case MonitorElement::DQM_KIND_REAL:
  case MonitorElement::DQM_KIND_STRING:
    info.flags |= obj.kind();
    obj.packScalarData(info.data, "");

    if (info.name.size() > 23
	&& info.name.compare(info.name.size()-23, 23, "/EventInfo/iLumiSection") == 0)
      meta.numLumiSections = std::max(meta.numLumiSections, uint64_t(atoll(info.data.c_str())));
    else if (info.name.size() > 27
	     && info.name.compare(info.name.size()-27, 27, "/EventInfo/processTimeStamp") == 0)
      meta.processedTime = std::max(meta.processedTime, uint64_t(atof(info.data.c_str())*1e9));
    else if (info.name.size() > 26
	     && info.name.compare(info.name.size()-26, 26, "/EventInfo/processedEvents") == 0)
      meta.numEvents = std::max(meta.numEvents, uint64_t(atoll(info.data.c_str())));
    else if (info.name.size() > 28
	     && info.name.compare(info.name.size()-28, 28, "/EventInfo/runStartTimeStamp") == 0)
      meta.runStartTime = std::max(meta.runStartTime, uint64_t(atof(info.data.c_str())*1e9));
    break;

  case MonitorElement::DQM_KIND_TH1F:
  case MonitorElement::DQM_KIND_TH1S:
  case MonitorElement::DQM_KIND_TH1D:
  case MonitorElement::DQM_KIND_TH2F:
  case MonitorElement::DQM_KIND_TH2S:
  case MonitorElement::DQM_KIND_TH2D:
  case MonitorElement::DQM_KIND_TH3F:
  case MonitorElement::DQM_KIND_TPROFILE:
  case MonitorElement::DQM_KIND_TPROFILE2D:
    info.flags |= obj.kind();
    info.object[0] = obj.getRootObject();
    if ((info.object[1] = obj.getRefRootObject()))
      info.flags |= VisDQMIndex::SUMMARY_PROP_HAS_REFERENCE;

    if (TH1 *h = dynamic_cast<TH1 *>(info.object[0]))
    {
      info.nentries = h->GetEntries();
      info.nbins[0] = h->GetNbinsX();
      info.nbins[1] = h->GetNbinsY();
      info.nbins[2] = h->GetNbinsZ();
      if (TAxis *a = h->GetXaxis())
      {
	info.mean[0] = h->GetMean(1);
	info.rms[0] = h->GetRMS(1);
	info.bounds[0][0] = a->GetXmin();
	info.bounds[0][1] = a->GetXmax();
      }
      if (TAxis *a = h->GetYaxis())
      {
	info.mean[1] = h->GetMean(2);
	info.rms[1] = h->GetRMS(2);
	info.bounds[1][0] = a->GetXmin();
	info.bounds[1][1] = a->GetXmax();
      }
      if (TAxis *a = h->GetZaxis())
      {
	info.mean[2] = h->GetMean(3);
	info.rms[2] = h->GetRMS(3);
	info.bounds[2][0] = a->GetXmin();
	info.bounds[2][1] = a->GetXmax();
      }
    }
    break;
  }

  // We are done, tell caller to keep this object.
  return ME_CLASS_KEEP;
}

struct MEClassifyTask
{
  FileInfo				&fi;
  DQMStore				&store;
  std::vector<MonitorElement *>		&mes;
  std::vector<MonitorElementInfo>	minfo;
  VisDQMIndex::Sample			meta;
  int					nerrors;
  pthread_t				thread;
  size_t				begin;
  size_t				end;
  MEClassifyTask(FileInfo &f, DQMStore &s, std::vector<MonitorElement*> &m)
    : fi(f), store(s), mes(m)
  {}
};

static void *
classifyMonitorElementRange(void *arg)
{
  MEClassifyTask &task = *(MEClassifyTask *) arg;
  DEBUG(1, "classifying range [" << task.begin << ", " << task.end
	<< ") in thread " << std::hex << (unsigned long) pthread_self()
	<< std::dec << "\n");

  for (size_t m = task.begin, e = task.end; m < e; ++m)
  {
    MonitorElementInfo info;
    switch (classifyMonitorElement(task.store, *task.mes[m], info,
				   task.meta, *task.fi.sample))
    {
    case ME_CLASS_BAD:
      task.nerrors++;
      std::cerr << task.fi.path.name()
		<< ": " << task.mes[m]->getFullname()
		<< ": warning: monitor element not recognised\n";
      continue;

    case ME_CLASS_SKIP:
      task.nerrors++;
      DEBUG(3, "skipping monitor element '"
	    << task.mes[m]->getFullname() << "'\n");
      continue;

    case ME_CLASS_KEEP:
      DEBUG(3, ""
	    << "flags=" << std::hex << info.flags << std::dec
	    << " tag=" << info.tag
	    << " object[0]=" << (void *) info.object[0]
	    << " object[1]=" << (void *) info.object[1]
	    << " category=" << info.category
	    << " lumi=" << info.lumibegin << ':' << info.lumiend
	    << " name='" << info.name
	    << "' data='" << info.data
	    << "' qreports='" << info.qreports
	    << "' nentries=" << std::setprecision(DBL_DIG+2) << info.nentries
	    << " nbins=" << info.nbins[0]
	    << "/" << info.nbins[1]
	    << "/" << info.nbins[2]
	    << " mean=" << info.mean[0]
	    << "/" << info.mean[1]
	    << "/" << info.mean[2]
	    << " rms=" << info.rms[0]
	    << "/" << info.rms[1]
	    << "/" << info.rms[2]
	    << " bounds=[" << info.bounds[0][0] << ":" << info.bounds[0][1]
	    << ", " << info.bounds[1][0] << ":" << info.bounds[1][1]
	    << ", " << info.bounds[2][0] << ":" << info.bounds[2][1]
	    << "]\n");
      task.minfo.push_back(info);
      break;
    }
  }

  return 0;
}

// ----------------------------------------------------------------------
/** Extract a numeric run number from a regexp string match into @a
    si.  Returns true on success, false on failure.  In case of
    failure an error has already been printed out.

    Note that the indexing uses only the run number from the file name
    to classify the data into a sample.  The data inside the file must
    match the run number and this is verified automatically.  No new
    sample classification occurs at per monitor element level. */
static bool
extractRunNumber(FileInfo &fi, SampleInfo &si, RegexpMatch &m, int matchnr)
{
  char *end = 0;
  std::string runnr = m.matchString(fi.path.name(), matchnr);

  errno = 0;
  si.runnr = strtol(runnr.c_str(), &end, 10);
  int err = errno;

  if (err > 0)
  {
    std::cerr << fi.path.name() << ": error extracting run number: "
	      << strerror(err) << " (error code " << err << ")\n";
    return false;
  }
  else if (! end || *end || si.runnr < 0)
  {
    std::cerr << fi.path.name() << ": unknown error extracting run number\n";
    return false;
  }

  return true;
}

/** Guess data file parametres from the file name.  Classifies the
    file to "real data", "relval" and "other monte-carlo" based on the
    file name.

    For online, sets si.type to #TYPE_DATA and sets the run number.
    Only real data is supported for online naming convention.  The
    dataset name must be given from outside as online does not have
    datasets as such.  Online files are recognised by #rxonline.

    For offline, extracts dataset from the file name into si.dataset.
    Also extracts the run number from the file name into si.runnr.  If
    the run number is greater than one, sets si.type to #TYPE_DATA.
    If the run number is one and the dataset name looks like release
    validation (i.e., matches #rxrelval), sets si.version to CMSSW
    version and si.type to #TYPE_RELVAL, otherwise sets si.type to
    #TYPE_MC.

    Returns true and fills in @a fi on successful match, returns false
    on a failure; the file information may already have been gobbled.
    On failure an error message has already been printed out.

    Note that fi.sample will not be set.  It is assumed the caller
    will locate a sample object that matches @a si. */
static bool
fileInfoFromName(FileInfo &fi, SampleInfo &si)
{
  RegexpMatch m;

  // Check if file name matches online naming convention.  If so, a
  // dataset name must have been provided; proceed to extract the run
  // number from the file name.
  if (rxonline.match(fi.path.name(), 0, 0, &m))
  {
    if (si.dataset.empty())
    {
      std::cerr << fi.path.name()
		<< ": dataset name required for online files\n";
      return false;
    }

    if (extractRunNumber(fi, si, m, 1))
    {
      si.type = TYPE_DATA;
      return true;
    }

    return false;
  }

  // Check if the file name matches offline naming convention.  If so,
  // extract the run number and the dataset name from the file name,
  // and classify the file to "data", "relval" or "monte carlo".
  else if (rxoffline.match(fi.path.name(), 0, 0, &m))
  {
    si.dataset = StringOps::replace(m.matchString(fi.path.name(), 2).c_str(),
				    "__", "/");

    if (! extractRunNumber(fi, si, m, 1))
      return false;

    ASSERT(si.runnr >= 0);
    if (si.runnr > 1)
    {
      si.type = TYPE_DATA;
      return true;
    }
    else if (rxrelval.match(si.dataset, 0, 0, &m))
    {
      si.runnr = 1;
      si.type = TYPE_RELVAL;
      si.version = m.matchString(si.dataset, 1);
      return true;
    }
    else
    {
      si.runnr = 1;
      si.type = TYPE_MC;
      return true;
    }
  }

  // If neither of the above matched, it's not a valid file name.
  else
  {
    std::cerr << fi.path.name()
	      << ": cannot determine file properties from name\n";
    return false;
  }
}

/** Verify that all file parameters have been set or deduced correctly.
    Returns true on success and false on failure.  On failure an error
    message has already been printed out. */
static bool
verifyFileInfo(const FileInfo &fi)
{
  // Check we have correctly assigned a sample to the file name.
  if (fi.path.empty())
  {
    std::cerr << "empty file name not permitted\n";
    return false;
  }

  if (! fi.sample)
  {
    std::cerr << fi.path.name()
	      << ": file has not been classified to any sample\n";
    return false;
  }

  SampleInfo &si = *fi.sample;
  if (! rxdataset.exactMatch(si.dataset))
  {
    std::cerr << fi.path.name() << ": dataset '"
	      << si.dataset << "' not valid\n";
    return false;
  }

  // Match file name against various conventions.
  bool online = rxonline.exactMatch(fi.path.name());
  bool offline = rxoffline.exactMatch(fi.path.name());

  switch (si.type)
  {
  case TYPE_OTHER:
    // Unclassified data is not permitted.
    std::cerr << fi.path.name() << ": file type not deduced\n";
    return false;

  case TYPE_DATA:
    // Check that data classified as "real" data matches expected
    // conventions: either online, or offline with a real run number
    // and dataset name for real data.
    if (si.runnr <= 1)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'data' but run number <= 1\n";
      return false;
    }

    if (! si.version.empty())
    {
      std::cerr << fi.path.name()
		<< ": file type is 'data' but it has non-empty version '"
		<< si.version << "' (only relval can have a version)\n";
      return false;
    }

    if (! online && ! offline)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'data' but file name does not match"
		<< " known conventions\n";
      return false;
    }

    if (offline && si.dataset.compare(0, 7, "/RelVal") == 0)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'data' but file name is relval\n";
      return false;
    }

    return true;

  case TYPE_RELVAL:
    // Check that data classified as a relval matched expected
    // conventions: offline with a dataset, cmssw version name was
    // found, and dataset name matches relval convention.
    if (si.runnr != 1)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'relval' but run number is non-zero ("
		<< si.runnr << ")\n";
      return false;
    }

    if (si.version.empty())
    {
      std::cerr << fi.path.name()
		<< ": file type is 'relval' but version is empty\n";
      return false;
    }

    if (! offline)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'relval' but file name does not match"
		<< " expected convention\n";
      return false;
    }

    if (! rxrelval.exactMatch(si.dataset))
    {
      std::cerr << fi.path.name()
		<< ": file type is 'relval' but dataset name '" << si.dataset
		<< "' does not match release validation convention\n";
      return false;
    }

    return true;

  case TYPE_MC:
    // Check that data classified as simulation data matched expected
    // conventions: offline with a dataset name, and it wasn't relval.
    if (si.runnr != 1)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'mc' but run number is non-zero ("
		<< si.runnr << ")\n";
      return false;
    }

    if (! si.version.empty())
    {
      std::cerr << fi.path.name()
		<< ": file type is 'mc' but it has non-empty version '"
		<< si.version << "' (only relval can have a version)\n";
      return false;
    }

    if (online || ! offline)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'mc' but file name does not match"
		<< " expected convention\n";
      return false;
    }

    if (offline && si.dataset.compare(0, 7, "/RelVal") == 0)
    {
      std::cerr << fi.path.name()
		<< ": file type is 'mc' but file name is relval\n";
      return false;
    }

    return true;

  default:
    // Can't happen
    ASSERT(false);
    return false;
  }
}

// ----------------------------------------------------------------------
/** Initialise a DQM GUI index.  This amounts to creating the
    directory structure and initialising empty index files.  */
static int
initIndex(const Filename &indexdir)
{
  DEBUG(1, "creating index directory " << indexdir << '\n');
  Filename::makedir(indexdir, 0755, true, true);

  DEBUG(1, "initialising index\n");
  VisDQMIndex ix(indexdir);
  ix.initialise();
  return EXIT_SUCCESS;
}


// ----------------------------------------------------------------------
/** Remove monitor element data to sample data files. */
static void
contract(VisDQMIndex &ix,
	 VisDQMIndex::Sample &s,
	 uint64_t nsample,
	 std::list<Filename> &oldfiles,
	 std::list<Filename> &newfiles)
{
  // Copy data to summary and data files, keeping data in key order.
  // If we find a match against NSAMPLE, then skip the copy.
  for (int kind = 1; kind >= 0; --kind)
  {
    VisDQMFile *rfile = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
				s.files[kind] >> 16,
				s.files[kind] & 0xffff,
				VisDQMFile::OPEN_READ);
    VisDQMFile *wfile = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
				s.files[kind] >> 16,
				(s.files[kind] & 0xffff)
				+ (rfile ? 1 : 0),
				VisDQMFile::OPEN_WRITE);

    DEBUG(1, "writing out new data file " << kind
	  << ": in [" << (s.files[kind] >> 16)
	  << ':' << (s.files[kind] & 0xffff)
	  << "]=" << (rfile ? rfile->path().name() : "(none)")
	  << " out " << wfile->path() << '\n');

    VisDQMFile::ReadHead rdhead(rfile, 0);
    VisDQMFile::WriteHead wrhead(wfile);
    uint64_t begin = nsample << 44;
    uint64_t end = (nsample+1) << 44;
    uint64_t rkey;
    void *rstart;
    void *rend;

    // Transfer keys we are not deleting.
    DEBUG(2, "keeping keys up to " << std::hex << begin << std::dec << '\n');
    wrhead.xfer(rdhead, begin, &rkey, &rstart, &rend);

    // Skip keys until we reach the next sample.
    while (! rdhead.isdone() && rkey < end)
    {
      DEBUG(2, "removing object key " << std::hex << rkey << std::dec << '\n');
      rdhead.next();
      if (! rdhead.isdone())
	rdhead.get(&rkey, &rstart, &rend);
    }

    // OK, bulk transfer the rest.
    DEBUG(2, "transferring rest of original contents\n");
    wrhead.xfer(rdhead, ~0ull, &rkey, &rstart, &rend);
    rdhead.finish();
    wrhead.finish();

    // Close input and output files.  If we wrote out a new file,
    // update the sample to indicate we have a new file version.  The
    // caller will update other samples sharing this same data file.
    if (rfile)
    {
      oldfiles.push_back(rfile->path());
      rfile->close();
      s.files[kind]++;
    }

    newfiles.push_back(wfile->path());
    wfile->close();
    delete rfile;
    delete wfile;
  }
}

// ----------------------------------------------------------------------
/** Add monitor element data to sample data files. */
static void
extend(VisDQMIndex &ix,
       VisDQMIndex::Sample &s,
       uint64_t nsample,
       std::vector<MonitorElementInfo> &minfo,
       StringAtomTree &objnames,
       std::list<Filename> &oldfiles,
       std::list<Filename> &newfiles,
       bool streamFile,
       StringAtomTree & rootobjs)
{
  // Produce monitor element permutation by their output ordering.
  // This is mainly by name, but see VisDQMIndex of how the full
  // 64-bit key is generated.
  std::vector<uint32_t> minfoix(minfo.size(), 0);
  for (size_t i = 0, e = minfo.size(); i < e; ++i)
  {
    minfoix[i] = i;
    minfo[i].nameidx = (nsample << 44)
		       | (uint64_t(minfo[i].category) << 40)
		       | (uint64_t(minfo[i].lumiend) << 20)
		       | (StringAtom(&objnames, minfo[i].name).index());
  }
  std::sort(minfoix.begin(), minfoix.end(), OrderByNameIndex(minfo));

  // Copy data to summary and data files, keeping data in key order.
  // First write out serialised data so we can record into summary how
  // much we wrote out.
  for (int kind = 1; kind >= 0; --kind)
  {
    VisDQMFile *rfile = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
				s.files[kind] >> 16,
				s.files[kind] & 0xffff,
				VisDQMFile::OPEN_READ);
    VisDQMFile *wfile = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
				s.files[kind] >> 16,
				(s.files[kind] & 0xffff)
				+ (rfile ? 1 : 0),
				VisDQMFile::OPEN_WRITE);

    DEBUG(1, "writing out new data file " << kind
	  << ": in [" << (s.files[kind] >> 16)
	  << ':' << (s.files[kind] & 0xffff)
	  << "]=" << (rfile ? rfile->path().name() : "(none)")
	  << " out " << wfile->path() << '\n');

    VisDQMFile::ReadHead rdhead(rfile, 0);
    VisDQMFile::WriteHead wrhead(wfile);
    uint64_t ix = 0;
    uint64_t rkey;
    void *rstart;
    void *rend;
    void *wstart;
    void *wend;

    // Keep writing out new entries as long as we haven't written out
    // all the new monitor elements; when we are updating, we may be
    // inserting and replacing monitor elements in any sequence.
    while (ix < minfo.size())
    {
      // Transfer keys we are not updating.
      DEBUG(2, "transferring previous keys up to "
	    << ix << ':' << std::hex << minfo[minfoix[ix]].nameidx
	    << std::dec << '\n');
      wrhead.xfer(rdhead, minfo[minfoix[ix]].nameidx, &rkey, &rstart, &rend);

      // Now write new keys, or replace old objects.
      while (ix < minfo.size()
	     && (rdhead.isdone()
		 || rkey >= minfo[minfoix[ix]].nameidx))
      {
	MonitorElementInfo &info = minfo[minfoix[ix]];
	if (kind == 1)
	{
	  if (! streamFile)
	  {
	    // Serialise and write out the ROOT objects, if any.  Note
	    // that this step is done first, and fills in the statistics
	    // of how large a binary blob we wrote out.
	    if (! (info.object[0] || info.object[1]))
	      info.ndata = 0;
	    else
	    {
	      TBufferFile buffer(TBufferFile::kWrite);
	      for (int i = 0; i < 2; ++i)
		if (info.object[i])
		  buffer.WriteObject(info.object[i]);
		else
		  buffer.WriteObjectAny(0, 0);

	      info.ndata = buffer.Length();
	      DEBUG(3, (rkey == info.nameidx ? "updating" : "inserting")
		    << " data for " << ix << ':'
		    << std::hex << info.nameidx << std::dec
		    << " ('" << info.name << "'): "
		    << info.ndata << " bytes\n");
	      wrhead.allocate(info.nameidx, info.ndata, &wstart, &wend);
	      memcpy(wstart, buffer.Buffer(), info.ndata);
	    }
	  }
	  else
	  {
	    switch (info.flags & DQMNet::DQM_PROP_TYPE_MASK)
	    {
	    case MonitorElement::DQM_KIND_TH1F:
	    case MonitorElement::DQM_KIND_TH1S:
	    case MonitorElement::DQM_KIND_TH1D:
	    case MonitorElement::DQM_KIND_TH2F:
	    case MonitorElement::DQM_KIND_TH2S:
	    case MonitorElement::DQM_KIND_TH2D:
	    case MonitorElement::DQM_KIND_TH3F:
	    case MonitorElement::DQM_KIND_TPROFILE:
	    case MonitorElement::DQM_KIND_TPROFILE2D:

	      DEBUG(3, (rkey == info.nameidx ? "updating" : "inserting")
		    << " data for " << ix << ':'
		    << std::hex << info.nameidx << std::dec
		    << " ('" << info.name << "'): "
		    << info.ndata << " bytes\n");
	      wrhead.allocate(info.nameidx, info.ndata, &wstart, &wend);
	      memcpy(wstart, rootobjs.key(info.streamidx).data(), info.ndata);
	      break;
	    }
	  }
	}
	else
	{
	  // Write out the monitor element summary description.
	  size_t datalen = info.data.size() ? info.data.size()+1 : 0;
	  size_t qreplen = info.qreports.size() ? info.qreports.size()+1 : 0;

	  // Important: while indexing a streamed .dat file, there is
	  // no need to add the extra null character and the end of
	  // data and qreports, since it has already been added and
	  // read back while producing the .dat file. In other words
	  // the size of the strings already includes the null at the
	  // end.

	  if (streamFile)
	  {
	    datalen = datalen ? datalen-1 : datalen;
	    qreplen = qreplen ? qreplen-1 : qreplen;
	  }
	  DEBUG(3, (rkey == info.nameidx ? "updating" : "inserting")
		<< " summary for " << ix << ':'
		<< std::hex << info.nameidx << std::dec
		<< " ('" << info.name << "'): "
		<< sizeof(VisDQMIndex::Summary) << " summary + "
		<< datalen << " data + " << qreplen
		<< " qreports bytes\n");
	  wrhead.allocate(info.nameidx,
			  roundup(sizeof(VisDQMIndex::Summary) + datalen + qreplen,
				  sizeof(uint64_t)),
			  &wstart, &wend);
	  VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) wstart;
	  s->properties = info.flags;
	  s->dataLength = datalen;
	  s->qtestLength = qreplen;
	  s->objectLength = info.ndata;
	  s->tag = info.tag;
	  s->nentries = info.nentries;
	  for (int i = 0; i < 3; ++i)
	  {
	    s->nbins[i] = info.nbins[i];
	    s->mean[i] = info.mean[i];
	    s->rms[i] = info.rms[i];
	    s->bounds[i][0] = info.bounds[i][0];
	    s->bounds[i][1] = info.bounds[i][1];
	  }

	  char *buf = (char *) (s + 1);
	  memcpy(buf, info.data.c_str(), datalen);
	  memcpy(buf+datalen, info.qreports.c_str(), qreplen);
	}

	// Now move to the next monitor element.  If we are updating
	// rather than inserting, move the read head past this object.
	++ix;

	if (! rdhead.isdone() && rkey == info.nameidx)
	{
	  rdhead.next();
	  if (! rdhead.isdone())
	    rdhead.get(&rkey, &rstart, &rend);
	}
      }
    }

    // OK, we've written everything out, bulk transfer the rest.
    DEBUG(2, "transferring rest of original contents\n");
    wrhead.xfer(rdhead, ~0ull, &rkey, &rstart, &rend);
    rdhead.finish();
    wrhead.finish();

    // Close input and output files.  If we wrote out a new file,
    // update the sample to indicate we have a new file version.  The
    // caller will update other samples sharing this same data file.
    if (rfile)
    {
      oldfiles.push_back(rfile->path());
      rfile->close();

      // Increment LSB 16, wrapping overflow within those bits.
      s.files[kind] = (s.files[kind] & 0xffff0000)
		      | ((s.files[kind]+1) & 0x0000ffff);
    }

    newfiles.push_back(wfile->path());
    wfile->close();
    delete rfile;
    delete wfile;
  }
}

/** Read a file streamed out of the index. */
static void
readFileStream(FileInfo &fi,
	       std::string &streamerinfo,
	       size_t &numObjs,
	       uint64_t &numEvents,
	       uint64_t &numLumiSections,
	       uint64_t &processedTime,
	       uint64_t &runStartTime,
	       std::vector<MonitorElementInfo> &minfo,
	       StringAtomTree &rootobjs)
{
  // Now scan the file.
  SampleInfo &si = *fi.sample;
  std::string dummy;
  VisDQMIndex::Sample sr;

  // Read in the file.
  std::ifstream iread(fi.path.name(), std::ios::in);
  if (!iread)
    throw VisDQMError(0, fi.path.name(),
		      StringFormat("failed to read file #%1")
		      .arg(fi.path.name()));
  iread
    >> sr.runNumber
    >> sr.numObjects
    >> numEvents
    >> numLumiSections
    >> runStartTime
    >> processedTime
    >> dummy;
  numObjs = sr.numObjects;
  streamerinfo = unhexlify(dummy);
  ASSERT(dummy == hexlify(streamerinfo));
  ASSERT(sr.runNumber == si.runnr);
  iread >> dummy;
  ASSERT(dummy == MEINFOBOUNDARY);
  minfo.reserve(sr.numObjects);
  // Keep on reading up to next boundary
  MonitorElementInfo cur;
  size_t datalen=0;
  size_t qreplen=0;
  std::string data, qt;
  while (true)
  {
    ASSERT(iread);
    cur.data.clear();
    cur.qreports.clear();
    // Be aware that the >> operator does not read the '\n' from the
    // previous line. Hence do a double read here: it's a horrible
    // hack, but it's not a mistake.
    getline(iread, dummy);
    getline(iread, cur.name);
    if (cur.name == MEROOTBOUNDARY)
      break;
    iread
      >> cur.category
      >> cur.lumibegin
      >> cur.flags
      >> datalen
      >> qreplen
      >> cur.ndata
      >> cur.tag
      >> dummy;
    cur.lumiend = cur.lumibegin;
    cur.nentries = strtod(dummy.data(), 0);
    iread >> cur.nbins[0];
    readDouble(iread, &cur.mean[0]);
    readDouble(iread, &cur.rms[0]);
    readDouble(iread, &cur.bounds[0][0]);
    readDouble(iread, &cur.bounds[0][1]);
    iread >> cur.nbins[1];
    readDouble(iread, &cur.mean[1]);
    readDouble(iread, &cur.rms[1]);
    readDouble(iread, &cur.bounds[1][0]);
    readDouble(iread, &cur.bounds[1][1]);
    iread >> cur.nbins[2];
    readDouble(iread, &cur.mean[2]);
    readDouble(iread, &cur.rms[2]);
    readDouble(iread, &cur.bounds[2][0]);
    readDouble(iread, &cur.bounds[2][1]);
    if (datalen)
    {
      iread >> dummy;
      ASSERT(dummy == "data:");
      iread >> dummy;
      cur.data = unhexlify(dummy);
      ASSERT(cur.data.size() == datalen);
    }
    if (qreplen)
    {
      iread >> dummy;
      ASSERT(dummy == "qt:");
      iread >> dummy;
      cur.qreports = unhexlify(dummy);
      ASSERT(cur.qreports.size() == qreplen);
    }
    minfo.push_back(cur);
  } // end of loop over ME Summary part

  // Now extract the real ROOT object(s), put them in a dedicated
  // StringAtomTree and fill in the streamidx field of the appropriate
  // monitorElementInfo. We deeply rely on the fact that the order in
  // which we stream ROOT hexlified buffers is the same in which we
  // streamed the VisDQMSummary part. This is guaranteed by the
  // strictly increasing ordering used while populating the index in
  // the first place.

  std::vector<MonitorElementInfo>::iterator mi = minfo.begin();
  std::vector<MonitorElementInfo>::iterator me = minfo.end();
  for (; mi != me; ++mi)
  {
    switch (mi->flags & DQMNet::DQM_PROP_TYPE_MASK)
    {
    case MonitorElement::DQM_KIND_INT:
    case MonitorElement::DQM_KIND_REAL:
    case MonitorElement::DQM_KIND_STRING:
      break;
    case MonitorElement::DQM_KIND_TH1F:
    case MonitorElement::DQM_KIND_TH1S:
    case MonitorElement::DQM_KIND_TH1D:
    case MonitorElement::DQM_KIND_TH2F:
    case MonitorElement::DQM_KIND_TH2S:
    case MonitorElement::DQM_KIND_TH2D:
    case MonitorElement::DQM_KIND_TH3F:
    case MonitorElement::DQM_KIND_TPROFILE:
    case MonitorElement::DQM_KIND_TPROFILE2D:
      /* This assert was used to debug the .dat format.
	 iread >> key;
	 std::cout << "key: " << key << " nameidx: " << mi->nameidx << " flags: " << mi->flags << std::endl;
	 assert(key == mi->nameidx);
      */
      iread >> dummy;
      mi->streamidx = StringAtom(&rootobjs, unhexlify(dummy)).index();
    }
    mi->nameidx=0;
  } // Finish reading a single file.
}

/** Add files to a DQM GUI index. */
static int
addFiles(const Filename &indexdir, std::list<FileInfo> &files)
{
  // Grab streamer info before we've opened any ROOT files.
  std::string streamerinfoFromRoot;
  std::string streamerinfoFromStream;
  buildStreamerInfo(streamerinfoFromRoot);
  DEBUG(2, streamerinfoFromRoot.size() << " bytes of streamer info captured\n");

  // Prepare but do not yet open the index.  Remember the time when we
  // started importing samples to the index; use a single time for all
  // the imports at a same time.
  VisDQMIndex ix(indexdir);
  uint64_t now = Time::current().ns();

  // Prepare to use DQMStore.
  DEBUG(1, "initialising monitor element store\n");
  edm::ParameterSet emptyps;
  std::vector<edm::ParameterSet> emptyset;
  edm::ServiceToken services(edm::ServiceRegistry::createSet(emptyset));
  edm::ServiceRegistry::Operate operate(services);
  DQMStore store(emptyps);

  // Now scan all input files one at a time.  Note that we do a
  // separate transaction for each file at a time.
  while (! files.empty())
  {
    VisDQMFile *master = 0;
    VisDQMFile *newmaster = 0;
    FileInfo &fi = files.front();
    SampleInfo &si = *fi.sample;
    std::list<Filename> newfiles;
    std::list<Filename> oldfiles;
    bool streamFile = isStreamFile(fi.path.name());
    DEBUG(1, "importing " << fi.path.name()
	  << ": sample [#" << si.index
	  << ", type '"
	  << (si.type == TYPE_DATA ? "data"
	      : si.type == TYPE_RELVAL ? "relval"
	      : si.type == TYPE_MC ? "mc" : "other")
	  << "', runnr " << si.runnr
	  << ", version '" << si.version
	  << "', dataset '" << si.dataset
	  << "']\n");

    try
    {
      // Start index update transaction.
      DEBUG(1, "starting index update\n");
      ix.beginUpdate(master, newmaster);
      newfiles.push_back(newmaster->path());

      // Read in the file with appropriate format.
      std::vector<MonitorElementInfo> minfo;
      StringAtomTree rootobjs(2500000);
      size_t    numObjs = 0;
      uint64_t 	numEvents = 0;
      uint64_t 	numLumiSections = 0;
      uint64_t 	runStartTime = 0;
      uint64_t 	processedTime = 0;
      if (!streamFile)
      {
	store.open(fi.fullpath.name());

	// Extract monitor element information somewhat parallelised.
	static const size_t NUM_TASKS = 4;
	std::vector<MonitorElement *> mes = store.getAllContents("");
	numObjs = mes.size();
	MEClassifyTask tasks[NUM_TASKS] = { MEClassifyTask(fi, store, mes),
					    MEClassifyTask(fi, store, mes),
					    MEClassifyTask(fi, store, mes),
					    MEClassifyTask(fi, store, mes) };
	size_t mesPerTask = (mes.size() + NUM_TASKS - 1) / NUM_TASKS;

	for (size_t i = 0; i < NUM_TASKS; ++i)
	{
	  tasks[i].begin = std::min(i * mesPerTask, mes.size());
	  tasks[i].end = std::min((i+1) * mesPerTask, mes.size());
	  tasks[i].minfo.reserve(tasks[i].end - tasks[i].begin);
	  memset(&tasks[i].meta, 0, sizeof(tasks[i].meta));
	  tasks[i].nerrors = 0;
	  if (pthread_create(&tasks[i].thread, 0, &classifyMonitorElementRange, &tasks[i]))
	    throw VisDQMError(0, fi.path.name(),
			      StringFormat("failed to create classification thread #%1")
			      .arg(i));
	}

	minfo.reserve(mes.size());
	for (size_t i = 0; i < NUM_TASKS; ++i)
	{
	  if (pthread_join(tasks[i].thread, 0))
	    throw VisDQMError(0, fi.path.name(),
			      StringFormat("failed to join classification thread #%1")
			      .arg(i));

	  tasks[0].meta.numEvents
	    = std::max(tasks[0].meta.numEvents,
		       tasks[i].meta.numEvents);
	  tasks[0].meta.numLumiSections
	    = std::max(tasks[0].meta.numLumiSections,
		       tasks[i].meta.numLumiSections);
	  tasks[0].meta.processedTime
	    = std::max(tasks[0].meta.processedTime,
		       tasks[i].meta.processedTime);
	  tasks[0].meta.runStartTime
	    = std::max(tasks[0].meta.runStartTime,
		       tasks[i].meta.runStartTime);

	  minfo.insert(minfo.end(), tasks[i].minfo.begin(), tasks[i].minfo.end());
	  tasks[i].minfo.resize(0);
	}
	numEvents = tasks[0].meta.numEvents;
	numLumiSections = tasks[0].meta.numLumiSections;
	processedTime = tasks[0].meta.processedTime;
	runStartTime =  tasks[0].meta.runStartTime;
      }
      else
	readFileStream(fi, streamerinfoFromStream, numObjs,
		       numEvents, numLumiSections,
		       processedTime, runStartTime,
		       minfo, rootobjs);

      // Grab various strings tables from the master file.
      DEBUG(1, "reading string tables\n");
      StringAtomTree vnames(10000);
      StringAtomTree dsnames(100000);
      StringAtomTree pathnames(1000000);
      StringAtomTree objnames(2500000);
      StringAtomTree streamers(100);

      readStrings(pathnames, master, VisDQMIndex::MASTER_SOURCE_PATHNAME);
      readStrings(dsnames, master, VisDQMIndex::MASTER_DATASET_NAME);
      readStrings(vnames, master, VisDQMIndex::MASTER_CMSSW_VERSION);
      readStrings(objnames, master, VisDQMIndex::MASTER_OBJECT_NAME);
      readStrings(streamers, master, VisDQMIndex::MASTER_TSTREAMERINFO);

      // Make sure the first CMSSW version is always empty string.
      if (StringAtom(&vnames, "").index() != 0)
      {
	std::cerr << indexdir << ": error: inconsistent index, the first"
		  << " cmssw version is not an empty string\n";
	abort();
      }

      // Copy samples to a temporary array.  If the sample is already
      // in the index, update it in place.  Track which data files
      // will hold the sample data; for new samples the last files in
      // the index, for existing samples remember its (updated) files.
      DEBUG(1, "locating and copying samples\n");
      bool found = false;
      uint32_t datafile[2] = { 0, 0 };
      StringAtom sadataset(&dsnames, si.dataset);
      StringAtom saversion(&vnames, si.version);
      StringAtom sapath(&pathnames, fi.path.name());

      // Streamerinfo handling from ROOT is kind of tricky. If we have
      // to index in the same job multiple .root and .dat files, we
      // should really be using:
      // for ROOT files: the original streamerinfo bundled to the ROOT
      // release that comes with the current GUI's release, captured
      // before opening any ROOT files.
      // for STREAM files: the original streamerinfo used to stream
      // the ROOT objects.

      size_t sastreamerIndex = 0;
      if (!streamFile)
      {
	StringAtom sastreamer(&streamers, streamerinfoFromRoot);
	sastreamerIndex = sastreamer.index();
      }
      else
      {
	StringAtom sastreamer(&streamers, streamerinfoFromStream);
	sastreamerIndex = sastreamer.index();
      }
      std::vector<VisDQMIndex::Sample> samples;
      samples.reserve(10000);

      for (VisDQMFile::ReadHead rdhead
	     (master, VisDQMIndex::MASTER_SAMPLE_RECORD);
	   ! rdhead.isdone(); rdhead.next())
      {
	// Extract next sample.  Stop when we hit other tables.
	void *rdbegin, *rdend;
	uint64_t key, hipart;
	rdhead.get(&key, &rdbegin, &rdend);
	hipart = key & 0xffffffff00000000ull;
	if (hipart != VisDQMIndex::MASTER_SAMPLE_RECORD)
	  break;

	// Add the sample to a list so we can revise file id.
	samples.push_back(*(VisDQMIndex::Sample *)rdbegin);
	VisDQMIndex::Sample &s = samples.back();

	DEBUG(2, "considering sample #" << (samples.size()-1)
	      << ": dataset '" << dsnames.key(s.datasetNameIdx)
	      << "', version='" << vnames.key(s.cmsswVersion).c_str()
	      << "', runnr=" << s.runNumber
	      << "\n");

	// If we've not yet seen sample we are looking for, check if
	// this one matches.  Note that we keep looping to collect all
	// samples to write them back out.
	if (! found)
	{
	  // Check for a match with our dataset spec.
	  found = (s.datasetNameIdx == sadataset.index()
		   && s.cmsswVersion == saversion.index()
		   && s.runNumber == si.runnr);

	  // If a match, update in place and remember which files we
	  // used.  Otherwise keep tracking for the "last" file used.
	  if (found)
	  {
	    DEBUG(2, "sample matches current one, updating\n");
	    if (s.streamerInfoIdx != sastreamerIndex)
	      throw VisDQMError(0, fi.path.name(),
				StringFormat("cannot update sample because"
					     " streamer info has changed"
					     ", previous streamer info #%1"
					     ", new is #%2")
				.arg(s.streamerInfoIdx)
				.arg(sastreamerIndex));

	    s.lastImportTime = now;
	    s.sourceFileIdx = sapath.index();
	    s.importVersion++;
	    extend(ix, s, samples.size()-1, minfo,
		   objnames, oldfiles, newfiles, streamFile, rootobjs);
	    datafile[0] = s.files[0];
	    datafile[1] = s.files[1];
	  }
	  else
	  {
	    for (int i = 0; i < 2; ++i)
	      if (s.files[i] > datafile[i])
		datafile[i] = s.files[i];
	  }
	}
      }

      // If this sample wasn't already in the index, add a new one at
      // the end.  Append data to the last file used in the index,
      // provided it's not too large; otherwise start a new file.
      // Remember which data files were modified here.
      if (! found)
      {
	// Start a new data file if the last used file is too large.
	for (int i = 0; i < 2; ++i)
	  if (ix.size(i, datafile[i] >> 16, datafile[i] & 0xffff) > 500000000)
	    datafile[i] = (datafile[i] & 0xffff0000) + 0x10000;

	// Initialise sample data structure.
	samples.push_back(VisDQMIndex::Sample());
	VisDQMIndex::Sample &s = samples.back();
	memset(&s, 0, sizeof(VisDQMIndex::Sample));

	DEBUG(2, "adding new sample #" << (samples.size()-1)
	      << ", info-file #" << (datafile[0] >> 16)
	      << ':' << (datafile[0] & 0xffff)
	      << ", data-file #" << (datafile[1] >> 16)
	      << ':' << (datafile[1] & 0xffff)
	      << '\n');

	s.firstImportTime = now;
	s.lastImportTime = now;
	s.importVersion = 1;
	s.files[0] = datafile[0];
	s.files[1] = datafile[1];
	s.sourceFileIdx = sapath.index();
	s.datasetNameIdx = sadataset.index();
	s.streamerInfoIdx = sastreamerIndex;
	s.cmsswVersion = saversion.index();
	s.runNumber = si.runnr;
	s.numObjects = numObjs;
	s.numEvents = numEvents;
	s.numLumiSections = numLumiSections;
	s.runStartTime = runStartTime;
	s.processedTime = processedTime;
	extend(ix, s, samples.size()-1, minfo,
	       objnames, oldfiles, newfiles, streamFile, rootobjs);
	datafile[0] = s.files[0];
	datafile[1] = s.files[1];
      }

      // Now scan all samples and update the file version on the ones
      // which shared a data file we just updated (and bumped version).
      for (size_t i = 0, e = samples.size(); i != e; ++i)
	for (int j = 0; j < 2; ++j)
	  if ((samples[i].files[j] & 0xffff0000) == (datafile[j] & 0xffff0000))
	    samples[i].files[j] = datafile[j];

      // Write out the samples and other tables and commit transaction.
      VisDQMFile::WriteHead wrhead(newmaster);
      for (size_t i = 0, e = samples.size(); i != e; ++i)
      {
	void *wrbegin, *wrend;
	wrhead.allocate(VisDQMIndex::MASTER_SAMPLE_RECORD + i,
			sizeof(VisDQMIndex::Sample), &wrbegin, &wrend);
	memcpy(wrbegin, &samples[i], sizeof(VisDQMIndex::Sample));
      }

      DEBUG(1, "adding " << (pathnames.size()-1) << " path names\n");
      writeStrings(wrhead, pathnames, VisDQMIndex::MASTER_SOURCE_PATHNAME);

      DEBUG(1, "adding " << (dsnames.size()-1) << " dataset names\n");
      writeStrings(wrhead, dsnames, VisDQMIndex::MASTER_DATASET_NAME);

      DEBUG(1, "adding " << (vnames.size()-1) << " version names\n");
      writeStrings(wrhead, vnames, VisDQMIndex::MASTER_CMSSW_VERSION, 0);

      DEBUG(1, "adding " << (objnames.size()-1) << " object names\n");
      writeStrings(wrhead, objnames, VisDQMIndex::MASTER_OBJECT_NAME);

      DEBUG(1, "adding " << (streamers.size()-1) << " streamers\n");
      writeStrings(wrhead, streamers, VisDQMIndex::MASTER_TSTREAMERINFO);

      DEBUG(1, "committing output\n");
      wrhead.finish();
      ix.commitUpdate();
      newfiles.clear();

      // Remove old data file versions now.
      std::list<Filename>::iterator fni, fne;
      for (fni = oldfiles.begin(), fne = oldfiles.end(); fni != fne; ++fni)
      {
	DEBUG(1, "removing old file " << fni->name() << std::endl);
	Filename::remove(*fni, false, true);
      }
    }

    // If we had an error, report it, roll back as much as possible,
    // cancel index transaction, and quit.
    catch (std::exception &err)
    {
      std::list<Filename>::iterator i, e;
      std::cerr << fi.path.name() << ": error reading file: "
		<< err.what() << '\n';
      for (i = newfiles.begin(), e = newfiles.end(); i != e; ++i)
      {
	DEBUG(1, "undoing new file " << i->name() << std::endl);
	Filename::remove(*i, false, true);
      }

      if (master)
      {
	DEBUG(1, "cancelling index transaction\n");
	ix.cancelUpdate();
	return EXIT_FAILURE;
      }
    }

    // Now clear the DQM store for the next file.
    store.rmdir("");
    files.pop_front();
  }

  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------
/** Remove files from a DQM GUI index. */
static int
removeFiles(const Filename &indexdir, const std::string &dataset, int32_t runnr)
{
  // Prepare but do not yet open the index.
  std::string streamerinfo;
  VisDQMIndex ix(indexdir);
  VisDQMFile *master = 0;
  VisDQMFile *newmaster = 0;
  std::list<Filename> newfiles;
  std::list<Filename> oldfiles;
  DEBUG(1, "deleting runnr " << runnr << ", dataset '" << dataset << "'\n");

  try
  {
    // Start index update transaction.
    DEBUG(1, "starting index update\n");
    ix.beginUpdate(master, newmaster);
    newfiles.push_back(newmaster->path());

    // Grab various strings tables from the master file.
    DEBUG(1, "reading string tables\n");
    StringAtomTree vnames(10000);
    StringAtomTree dsnames(100000);
    StringAtomTree pathnames(1000000);
    StringAtomTree objnames(2500000);
    StringAtomTree streamers(100);

    readStrings(pathnames, master, VisDQMIndex::MASTER_SOURCE_PATHNAME);
    readStrings(dsnames, master, VisDQMIndex::MASTER_DATASET_NAME);
    readStrings(vnames, master, VisDQMIndex::MASTER_CMSSW_VERSION);
    readStrings(objnames, master, VisDQMIndex::MASTER_OBJECT_NAME);
    readStrings(streamers, master, VisDQMIndex::MASTER_TSTREAMERINFO);

    // Make sure the first CMSSW version is always empty string.
    if (StringAtom(&vnames, "").index() != 0)
    {
      std::cerr << indexdir << ": error: inconsistent index, the first"
		<< " cmssw version is not an empty string\n";
      abort();
    }

    // Copy samples to a temporary array.  Track which data files used
    // to hold the sample data so we can update file versions.
    DEBUG(1, "locating and removing samples\n");
    bool found = false;
    uint32_t datafile[2] = { 0, 0 };
    StringAtom sadataset(&dsnames, dataset, StringAtom::TestOnly);
    std::vector<VisDQMIndex::Sample> samples;
    samples.reserve(10000);

    for (VisDQMFile::ReadHead rdhead
	   (master, VisDQMIndex::MASTER_SAMPLE_RECORD);
	 ! rdhead.isdone(); rdhead.next())
    {
      // Extract next sample.  Stop when we hit other tables.
      void *rdbegin, *rdend;
      uint64_t key, hipart;
      rdhead.get(&key, &rdbegin, &rdend);
      hipart = key & 0xffffffff00000000ull;
      if (hipart != VisDQMIndex::MASTER_SAMPLE_RECORD)
	break;

      // Add the sample to a list so we can revise file id.
      samples.push_back(*(VisDQMIndex::Sample *)rdbegin);
      VisDQMIndex::Sample &s = samples.back();

      DEBUG(2, "considering sample #" << (samples.size()-1)
	    << ": dataset '" << dsnames.key(s.datasetNameIdx)
	    << "', runnr=" << s.runNumber
	    << "\n");

      // If we've not yet seen sample we are looking for, check if
      // this one matches.  Note that we keep looping to collect all
      // samples to write them back out.
      if (! found)
      {
	// Check for a match with our dataset spec.
	found = (s.datasetNameIdx == sadataset.index()
		 && s.runNumber == runnr);

	// If got a match, delete the sample's data and invalidate
	// the sample in the index.  We still write it out to avoid
	// destabilising the sample numbering; the server knows to
	// skip it when retrieving samples.
	if (found)
	{
	  DEBUG(2, "sample matches current one, removing\n");
	  contract(ix, s, samples.size()-1, oldfiles, newfiles);
	  datafile[0] = s.files[0];
	  datafile[1] = s.files[1];
	  s.numObjects = 0;
	}
	else
	{
	  for (int i = 0; i < 2; ++i)
	    if (s.files[i] > datafile[i])
	      datafile[i] = s.files[i];
	}
      }
    }

    // Now scan all samples and update the file version on the ones
    // which shared a data file we just updated (and bumped version).
    for (size_t i = 0, e = samples.size(); i != e; ++i)
      for (int j = 0; j < 2; ++j)
	if ((samples[i].files[j] & 0xffff0000) == (datafile[j] & 0xffff0000))
	  samples[i].files[j] = datafile[j];

    // Write out the samples and other tables and commit transaction.
    VisDQMFile::WriteHead wrhead(newmaster);
    for (size_t i = 0, e = samples.size(); i != e; ++i)
    {
      void *wrbegin, *wrend;
      wrhead.allocate(VisDQMIndex::MASTER_SAMPLE_RECORD + i,
		      sizeof(VisDQMIndex::Sample), &wrbegin, &wrend);
      memcpy(wrbegin, &samples[i], sizeof(VisDQMIndex::Sample));
    }

    DEBUG(1, "adding " << (pathnames.size()-1) << " path names\n");
    writeStrings(wrhead, pathnames, VisDQMIndex::MASTER_SOURCE_PATHNAME);

    DEBUG(1, "adding " << (dsnames.size()-1) << " dataset names\n");
    writeStrings(wrhead, dsnames, VisDQMIndex::MASTER_DATASET_NAME);

    DEBUG(1, "adding " << (vnames.size()-1) << " version names\n");
    writeStrings(wrhead, vnames, VisDQMIndex::MASTER_CMSSW_VERSION, 0);

    DEBUG(1, "adding " << (objnames.size()-1) << " object names\n");
    writeStrings(wrhead, objnames, VisDQMIndex::MASTER_OBJECT_NAME);

    DEBUG(1, "adding " << (streamers.size()-1) << " streamers\n");
    writeStrings(wrhead, streamers, VisDQMIndex::MASTER_TSTREAMERINFO);

    DEBUG(1, "committing output\n");
    wrhead.finish();
    ix.commitUpdate();
    newfiles.clear();

    // Remove old data file versions now.
    std::list<Filename>::iterator fni, fne;
    for (fni = oldfiles.begin(), fne = oldfiles.end(); fni != fne; ++fni)
    {
      DEBUG(1, "removing old file " << fni->name() << std::endl);
      Filename::remove(*fni, false, true);
    }
  }

  // If we had an error, report it, roll back as much as possible,
  // cancel index transaction, and quit.
  catch (std::exception &err)
  {
    std::list<Filename>::iterator i, e;
    std::cerr << app.name() << ": error: " << err.what() << '\n';
    for (i = newfiles.begin(), e = newfiles.end(); i != e; ++i)
    {
      DEBUG(1, "undoing new file " << i->name() << std::endl);
      Filename::remove(*i, false, true);
    }

    if (master)
    {
      DEBUG(1, "cancelling index transaction\n");
      ix.cancelUpdate();
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------
/** Add monitor element data to sample data files. */
static void
remapdata(VisDQMIndex &ix,
	  VisDQMIndex &other,
	  VisDQMIndex::Sample &s,
	  uint64_t origsample,
	  uint64_t newsample,
	  const std::vector<size_t> &objnames2ix,
          uint32_t *datafile,
          VisDQMFile *wfile[],
	  std::list<Filename> &oldfiles,
	  std::list<Filename> &newfiles)
{
  // Copy data to summary and data files, keeping data in key order.
  // Just remap the sample and object name index parts as we go.
  // However we must re-sort monitor element name part as the two
  // object name indices are not guaranteed to be in the same order.
  for (int kind = 1; kind >= 0; --kind)
  {
    VisDQMFile *rfile = other.open(VisDQMIndex::MASTER_FILE_INFO + kind,
				   s.files[kind] >> 16,
				   s.files[kind] & 0xffff,
				   VisDQMFile::OPEN_READ);
    VisDQMFile * oldfile = 0;
    uint64_t rkey;
    void *rbegin, *rend;
    void *wbegin, *wend;
    uint64_t begin = origsample << 44;
    std::vector< std::pair<uint64_t, uint64_t> > remapping;
    remapping.reserve(s.numObjects);

    if (wfile[kind] == 0)
    {
      oldfile = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
                        datafile[kind] >> 16,
                        datafile[kind] & 0xffff,
                        VisDQMFile::OPEN_READ);

      // Transfer any original file contents intact.
      if (oldfile)
      {
	if (ix.size(kind, datafile[kind] >> 16, datafile[kind] & 0xffff) < 500000000)
	{
	  DEBUG(2, "transferring original file contents first\n");
	  datafile[kind] = datafile[kind] + 1;
	  wfile[kind] = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
				datafile[kind] >> 16,
				(datafile[kind] & 0xffff),
				VisDQMFile::OPEN_WRITE);
	  VisDQMFile::WriteHead wrhead(wfile[kind]);
	  VisDQMFile::ReadHead rdold(oldfile, 0);
	  wrhead.xfer(rdold, ~0ull, &rkey, &rbegin, &rend);
	  wrhead.finish();
	}
	else
	{
	  // If the oldfile was too big, we do not need to transfer
	  // its content to the new file and we must not include it in
	  // the list of old files to be deleted, since it has not been
	  // superseeded by a new version.
	  oldfile->close();
	  oldfile = 0;
	  datafile[kind] = (datafile[kind] & 0xffff0000) + 0x10000;
	}
      }
      if (wfile[kind] == 0)
      {
        ASSERT((datafile[kind] & 0x0000ffff) == 0);
        wfile[kind] = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
			      datafile[kind] >> 16,
			      (datafile[kind] & 0xffff),
			      VisDQMFile::OPEN_WRITE);
      }
    }
    else
    {
      // Start a new data file if the last used file is too large.
      if (ix.size(kind, datafile[kind] >> 16, datafile[kind] & 0xffff) > 500000000)
      {
        wfile[kind]->close();
        delete wfile[kind];
        datafile[kind] = (datafile[kind] & 0xffff0000) + 0x10000;
        wfile[kind] = ix.open(VisDQMIndex::MASTER_FILE_INFO + kind,
                              datafile[kind] >> 16,
                              (datafile[kind] & 0xffff),
                              VisDQMFile::OPEN_WRITE);
        newfiles.push_back(wfile[kind]->path());
      }
    }
    DEBUG(1, "appending new data file " << kind
          << ": in [" << (s.files[kind] >> 16)
          << ':' << (s.files[kind] & 0xffff)
          << "]=" << rfile->path().name()
          << " out " << wfile[kind]->path()
          << ", replaces " << (oldfile ? oldfile->path().name() : "(none)")
          << '\n');



    VisDQMFile::WriteHead wrhead(wfile[kind]);

    // Build remap table of objects to new object index.
    // We don't read the actual objects yet.
    DEBUG(2, "remapping objects starting at "
	  << std::hex << begin << std::dec << "\n");
    for (VisDQMFile::ReadHead rdhead(rfile, begin);
	 ! rdhead.isdone(); rdhead.next())
    {
      // Check if the next object is still for this sample.
      rdhead.get(&rkey, &rbegin, &rend);
      uint64_t keyparts[4] = { (rkey >> 44) & 0xfffff, (rkey >> 40) & 0xf,
			       (rkey >> 20) & 0xfffff, rkey & 0xfffff };
      if (keyparts[0] != origsample)
	break;

      uint64_t wkey = ((newsample << 44)
		       | (keyparts[1] << 40)
		       | (keyparts[2] << 20)
		       | objnames2ix[keyparts[3]]);

      DEBUG(2, "mapping object key " << std::hex << rkey
	    << " to key " << wkey << std::dec << '\n');
      remapping.push_back(std::make_pair(wkey, rkey));
    }

    // Resort object keys into the desired order.
    std::sort(remapping.begin(), remapping.end());

    // Now write the objects out again, in new order.
    // This assumes we have a VisDQMCache for efficiency.
    for (size_t i = 0, e = remapping.size(); i != e; ++i)
    {
      VisDQMFile::ReadHead rdhead(rfile, remapping[i].second);
      ASSERT(! rdhead.isdone());

      rdhead.get(&rkey, &rbegin, &rend);
      ASSERT(rkey == remapping[i].second);

      size_t size = ((const char *) rend - (const char *) rbegin);

      DEBUG(2, "copying object key " << std::hex << rkey
	    << " to key " << remapping[i].first << std::dec
	    << ", " << size << " bytes\n");
      wrhead.allocate(remapping[i].first, size, &wbegin, &wend);
      memcpy(wbegin, rbegin, size);
    }

    // OK, we've written everything out.
    wrhead.finish();

    // Close input and output files, update the sample to indicate we
    // have a new file version.  The caller will update other samples
    // sharing this same data file.
    rfile->close();
    s.files[kind] = datafile[kind];

    if (oldfile)
    {
      oldfiles.push_back(oldfile->path());
      oldfile->close();
    }

    delete rfile;
  }
}

/** Merge DQM GUI index to another. */
static int
mergeIndexes(const Filename &indexdir, std::list<Filename> &mergeix)
{
  // Prepare but do not yet open the target index.
  VisDQMIndex ix(indexdir);

  // Merge each index at a time, in a separate transactions per merge.
  while (! mergeix.empty())
  {
    VisDQMFile *master = 0;
    VisDQMFile *newmaster = 0;
    VisDQMFile *othermaster = 0;
    VisDQMCache cache(3*1024*1024*1024ul);
    VisDQMIndex other(mergeix.front(), &cache);
    std::list<Filename> newfiles;
    std::list<Filename> oldfiles;
    DEBUG(1, "importing " << mergeix.front().name() << "\n");

    try
    {
      // Start index update transaction.
      DEBUG(1, "starting index update\n");
      ix.beginUpdate(master, newmaster);
      newfiles.push_back(newmaster->path());
      other.beginRead(othermaster);

      // Grab various strings tables from the master files.
      // Build mapping tables from imported index to target.
      DEBUG(1, "reading string tables\n");
      StringAtomTree vnames(10000),     othvnames(10000);
      StringAtomTree dsnames(100000),    othdsnames(100000);
      StringAtomTree pathnames(1000000), othpathnames(1000000);
      StringAtomTree objnames(2500000), othobjnames(2500000);
      StringAtomTree streamers(100),    othstreamers(100);

      readStrings(pathnames, master, VisDQMIndex::MASTER_SOURCE_PATHNAME);
      readStrings(dsnames, master, VisDQMIndex::MASTER_DATASET_NAME);
      readStrings(vnames, master, VisDQMIndex::MASTER_CMSSW_VERSION);
      readStrings(objnames, master, VisDQMIndex::MASTER_OBJECT_NAME);
      readStrings(streamers, master, VisDQMIndex::MASTER_TSTREAMERINFO);

      readStrings(othpathnames, othermaster, VisDQMIndex::MASTER_SOURCE_PATHNAME);
      readStrings(othdsnames, othermaster, VisDQMIndex::MASTER_DATASET_NAME);
      readStrings(othvnames, othermaster, VisDQMIndex::MASTER_CMSSW_VERSION);
      readStrings(othobjnames, othermaster, VisDQMIndex::MASTER_OBJECT_NAME);
      readStrings(othstreamers, othermaster, VisDQMIndex::MASTER_TSTREAMERINFO);

      std::vector<size_t> vnames2ix(othvnames.size());
      std::vector<size_t> dsnames2ix(othdsnames.size());
      std::vector<size_t> pathnames2ix(othpathnames.size());
      std::vector<size_t> objnames2ix(othobjnames.size());
      std::vector<size_t> streamers2ix(othstreamers.size());

      for (size_t i = 1, e = othvnames.size(); i < e; ++i)
	vnames2ix[i] = vnames.insert(othvnames.key(i));

      for (size_t i = 1, e = othdsnames.size(); i < e; ++i)
	dsnames2ix[i] = dsnames.insert(othdsnames.key(i));

      for (size_t i = 1, e = othpathnames.size(); i < e; ++i)
	pathnames2ix[i] = pathnames.insert(othpathnames.key(i));

      for (size_t i = 1, e = othobjnames.size(); i < e; ++i)
	objnames2ix[i] = objnames.insert(othobjnames.key(i));

      for (size_t i = 1, e = othstreamers.size(); i < e; ++i)
	streamers2ix[i] = streamers.insert(othstreamers.key(i));

      // Make sure the first CMSSW version is always empty string.
      if (StringAtom(&vnames, "").index() != 0)
      {
	std::cerr << indexdir << ": error: inconsistent index, the first"
		  << " cmssw version is not an empty string\n";
	abort();
      }

      // Copy and transform all samples and their monitor elements to
      // use new remapped string index values.  Overlapping is *NOT*
      // allowed, the samples being added from 'other' may not already
      // exist in 'ix'.  Hence we know we only add new data.  So first
      // copy existing samples in 'ix', then add those from 'other'.
      DEBUG(1, "merging samples\n");
      uint32_t datafile[2] = { 0, 0 };
      std::vector<VisDQMIndex::Sample> samples;
      samples.reserve(10000);

      for (VisDQMFile::ReadHead rdhead
	     (master, VisDQMIndex::MASTER_SAMPLE_RECORD);
	   ! rdhead.isdone(); rdhead.next())
      {
	// Extract next sample.  Stop when we hit other tables.
	void *rdbegin, *rdend;
	uint64_t key, hipart;
	rdhead.get(&key, &rdbegin, &rdend);
	hipart = key & 0xffffffff00000000ull;
	if (hipart != VisDQMIndex::MASTER_SAMPLE_RECORD)
	  break;

	// Add the sample to a list so we can revise file id.
	samples.push_back(*(VisDQMIndex::Sample *)rdbegin);
	VisDQMIndex::Sample &s = samples.back();
        DEBUG(1, "Adding sample " << samples.size()-1
              << " from the master index\n");

	// Remember last data files seen.
	for (int i = 0; i < 2; ++i)
	  if (s.files[i] > datafile[i])
	    datafile[i] = s.files[i];
      }

      uint32_t delta = samples.size();
      VisDQMFile *wfile[2] = {0,0};

      for (VisDQMFile::ReadHead rdhead
	     (othermaster, VisDQMIndex::MASTER_SAMPLE_RECORD);
	   ! rdhead.isdone(); rdhead.next())
      {
	// Extract next sample.  Stop when we hit other tables.
	void *rdbegin, *rdend;
	uint64_t key, hipart;
	rdhead.get(&key, &rdbegin, &rdend);
	hipart = key & 0xffffffff00000000ull;
	if (hipart != VisDQMIndex::MASTER_SAMPLE_RECORD)
	  break;

	// Add the sample to a list so we can revise file id.
	samples.push_back(*(VisDQMIndex::Sample *)rdbegin);
	VisDQMIndex::Sample &s = samples.back();

	// For each sample stored in the other index we transfer the
	// content of data and info files. We do not close the output
	// file at every sample boundary, but we keep it open until
	// its size reaches the suggested maximum. This has the side
	// effect of not incrementing the version number linearly with
	// the number of samples; we will end up with one unique
	// data/info file for all the transferred samples with
	// version+1 w.r.t. the previous one. As soon as we reach the
	// maximum size we increment the index number and keep the
	// version number fixed at 0. This should also speed up the
	// whole process since no transfer from version X to version
	// X+1 is needed at every sample boundary.

	// Remap contents.
	s.sourceFileIdx = pathnames2ix[s.sourceFileIdx];
	s.datasetNameIdx = dsnames2ix[s.datasetNameIdx];
	s.streamerInfoIdx = streamers2ix[s.streamerInfoIdx];
	s.cmsswVersion = vnames2ix[s.cmsswVersion];
	remapdata(ix, other, s, samples.size()-delta-1, samples.size()-1,
		  objnames2ix, datafile, wfile, oldfiles, newfiles);
	datafile[0] = s.files[0];
	datafile[1] = s.files[1];

	// Now scan all samples and update the file version on the ones
	// which shared a data file we just updated (and bumped version).
	for (size_t i = 0, e = samples.size(); i != e; ++i)
	  for (int j = 0; j < 2; ++j)
	    if ((samples[i].files[j] & 0xffff0000) == (datafile[j] & 0xffff0000))
	      samples[i].files[j] = datafile[j];
      }

      // Now close both output files
      for (int j = 0; j < 2; ++j)
        if (wfile[j])
        {
          wfile[j]->close();
          delete wfile[j];
          wfile[j] = 0;
        }

      // Write out the samples and other tables and commit transaction.
      VisDQMFile::WriteHead wrhead(newmaster);
      for (size_t i = 0, e = samples.size(); i != e; ++i)
      {
	void *wrbegin, *wrend;
	wrhead.allocate(VisDQMIndex::MASTER_SAMPLE_RECORD + i,
			sizeof(VisDQMIndex::Sample), &wrbegin, &wrend);
	memcpy(wrbegin, &samples[i], sizeof(VisDQMIndex::Sample));
      }

      DEBUG(1, "adding " << (pathnames.size()-1) << " path names\n");
      writeStrings(wrhead, pathnames, VisDQMIndex::MASTER_SOURCE_PATHNAME);

      DEBUG(1, "adding " << (dsnames.size()-1) << " dataset names\n");
      writeStrings(wrhead, dsnames, VisDQMIndex::MASTER_DATASET_NAME);

      DEBUG(1, "adding " << (vnames.size()-1) << " version names\n");
      writeStrings(wrhead, vnames, VisDQMIndex::MASTER_CMSSW_VERSION, 0);

      DEBUG(1, "adding " << (objnames.size()-1) << " object names\n");
      writeStrings(wrhead, objnames, VisDQMIndex::MASTER_OBJECT_NAME);

      DEBUG(1, "adding " << (streamers.size()-1) << " streamers\n");
      writeStrings(wrhead, streamers, VisDQMIndex::MASTER_TSTREAMERINFO);

      DEBUG(1, "committing output\n");
      wrhead.finish();
      ix.commitUpdate();
      other.finishRead();
      newfiles.clear();

      // Remove old data file versions now.
      std::list<Filename>::iterator fni, fne;
      for (fni = oldfiles.begin(), fne = oldfiles.end(); fni != fne; ++fni)
      {
	DEBUG(1, "removing old file " << fni->name() << std::endl);
	Filename::remove(*fni, false, true);
      }
    }

    // If we had an error, report it, roll back as much as possible,
    // cancel index transaction, and quit.
    catch (std::exception &err)
    {
      std::list<Filename>::iterator i, e;
      std::cerr << mergeix.front().name() << ": error reading index: "
		<< err.what() << '\n';
      for (i = newfiles.begin(), e = newfiles.end(); i != e; ++i)
      {
	DEBUG(1, "undoing new file " << i->name() << std::endl);
	Filename::remove(*i, false, true);
      }

      if (othermaster)
      {
	DEBUG(1, "cancelling index read\n");
	other.finishRead();
      }

      if (master)
      {
	DEBUG(1, "cancelling index transaction\n");
	ix.cancelUpdate();
	return EXIT_FAILURE;
      }
    }

    // Now go to next index.
    mergeix.pop_front();
  }

  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------
/** Dump parts of a DQM GUI index.  */
static int
dumpIndex(const Filename &indexdir, DumpType what, size_t sampleid)
{
  VisDQMFile *master;
  VisDQMIndex ix(indexdir);
  std::list<VisDQMIndex::Sample> samples;
  StringAtomTree vnames(10000);
  StringAtomTree dsnames(100000);
  StringAtomTree pathnames(1000000);
  StringAtomTree objnames(2500000);
  StringAtomTree streamers(100);
  DQMNet::QReports qreports;

  // Read the master catalogue. We need all the info anyway.
  DEBUG(1, "starting index read\n");
  ix.beginRead(master);
  for (VisDQMFile::ReadHead rdhead(master, 0); ! rdhead.isdone(); rdhead.next())
  {
    void *begin;
    void *end;
    uint64_t key;
    uint64_t hipart;

    rdhead.get(&key, &begin, &end);
    hipart = key & 0xffffffff00000000ull;
    if (hipart == VisDQMIndex::MASTER_SAMPLE_RECORD)
      samples.push_back(*(const VisDQMIndex::Sample *)begin);
    else
      break;
  }

  readStrings(pathnames, master, VisDQMIndex::MASTER_SOURCE_PATHNAME);
  readStrings(dsnames, master, VisDQMIndex::MASTER_DATASET_NAME);
  readStrings(vnames, master, VisDQMIndex::MASTER_CMSSW_VERSION);
  readStrings(objnames, master, VisDQMIndex::MASTER_OBJECT_NAME);
  readStrings(streamers, master, VisDQMIndex::MASTER_TSTREAMERINFO);

  // Now output the selected parts, walking over one sample at a time.
  std::list<VisDQMIndex::Sample>::iterator si;
  std::list<VisDQMIndex::Sample>::iterator se;

  // Dump master catalogue contents.
  if (what == DUMP_ALL || what == DUMP_CATALOGUE)
  {
    si = samples.begin();
    se = samples.end();
    for (size_t n = 0; si != se; ++si, ++n)
      if (n == sampleid || sampleid == ALL_SAMPLES)
	std::cout
	  << "SAMPLE #" << n
	  << " first-import=" << si->firstImportTime
	  << " last-import=" << si->lastImportTime
	  << " import-version=" << si->importVersion
	  << " info-file=#"
	  << ((si->files[VisDQMIndex::MASTER_FILE_INFO] >> 16) & 0xffff)
	  << ':' << (si->files[VisDQMIndex::MASTER_FILE_INFO] & 0xffff)
	  << " data-file=#"
	  << ((si->files[VisDQMIndex::MASTER_FILE_DATA] >> 16) & 0xffff)
	  << ':' << (si->files[VisDQMIndex::MASTER_FILE_DATA] & 0xffff)
	  << " src-file=#" << si->sourceFileIdx
	  << '/' << pathnames.key(si->sourceFileIdx)
	  << " dataset-name=#" << si->datasetNameIdx
	  << '/' << dsnames.key(si->datasetNameIdx)
	  << " streamer-info=#" << si->streamerInfoIdx
	  << '/' << streamers.key(si->streamerInfoIdx).size()
	  << " runnr=" << si->runNumber
	  << " cmssw-version='" << vnames.key(si->cmsswVersion).c_str()
	  << "' num-objects=" << si->numObjects
	  << " num-events=" << si->numEvents
	  << " num-lumi-sections=" << si->numLumiSections
	  << " run-start-time=" << si->runStartTime
	  << " processed-time=" << si->processedTime
	  << '\n';

    for (size_t i = 1, e = pathnames.size(); i != e; ++i)
      std::cout << "SOURCE-FILE #" << i
		<< "='" << pathnames.key(i) << "'\n";

    for (size_t i = 1, e = dsnames.size(); i != e; ++i)
      std::cout << "DATASET-NAME #" << i
		<< "='" << dsnames.key(i) << "'\n";

    for (size_t i = 1, e = vnames.size(); i != e; ++i)
      std::cout << "CMSSW-VERSION #" << i
		<< "='" << vnames.key(i) << "'\n";

    for (size_t i = 1, e = objnames.size(); i != e; ++i)
      std::cout << "OBJECT-NAME #" << i
		<< "='" << objnames.key(i) << "'\n";

    for (size_t i = 1, e = streamers.size(); i != e; ++i)
    {
      MD5Digest md5;
      const std::string &data = streamers.key(i);
      md5.update(&data[0], data.size());
      std::cout << "STREAMER #" << (i-1)
		<< "=[len:" << data.size()
		<< ", md5:" << md5.format() << "]\n";
    }
  }

  // Dump summary information for every monitor element.
  if (what == DUMP_ALL || what == DUMP_INFO)
  {
    si = samples.begin();
    se = samples.end();
    for (size_t n = 0; si != se; ++si, ++n)
    {
      if (sampleid != ALL_SAMPLES && n != sampleid)
	continue;
      std::cout << "BEGIN INFO #" << n << '\n';
      if (VisDQMFile *f
	  = ix.open(VisDQMIndex::MASTER_FILE_INFO,
		    si->files[VisDQMIndex::MASTER_FILE_INFO] >> 16,
		    si->files[VisDQMIndex::MASTER_FILE_INFO] & 0xffff,
		    VisDQMFile::OPEN_READ))
      {
	for (VisDQMFile::ReadHead rdhead(f, (uint64_t) n << 44);
	     ! rdhead.isdone(); rdhead.next())
	{
	  uint64_t key;
	  void *begin;
	  void *end;

	  rdhead.get(&key, &begin, &end);
	  uint64_t keyparts[4] = { (key >> 44) & 0xfffff, (key >> 40) & 0xf,
				   (key >> 20) & 0xfffff, key & 0xfffff };
	  if (keyparts[0] == n)
	  {
	    VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
	    uint32_t type = s->properties & VisDQMIndex::SUMMARY_PROP_TYPE_MASK;
	    uint32_t report = s->properties & VisDQMIndex::SUMMARY_PROP_REPORT_MASK;
	    const char *data = (s->dataLength ? (const char *) (s+1) : "");
	    const char *qdata
	      = (s->qtestLength ? ((const char *) (s+1) + s->dataLength) : "");

	    std::cout
	      << "ME [sample:" << keyparts[0] << ", category:" << keyparts[1]
	      << ", lumiend:" << keyparts[2] << ", nameidx:" << keyparts[3]
	      << "] name='" << objnames.key(keyparts[3])
	      << "' properties=" << std::hex << s->properties << std::dec << "["
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_INVALID ? "INVALID"
		  : (type <= VisDQMIndex::SUMMARY_PROP_TYPE_SCALAR) ? "SCALAR"
		  : "ROOT")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_INT ? ", INT" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_REAL ? ", REAL" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_STRING ? ", STRING" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TH1F ? ", TH1F" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TH1S ? ", TH1S" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TH2F ? ", TH2F" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TH2S ? ", TH2S" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TH3F ? ", TH3F" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TH3S ? ", TH3S" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TPROF ? ", TPROF" : "")
	      << (type == VisDQMIndex::SUMMARY_PROP_TYPE_TPROF2D ? ", TPROF2D" : "")
	      << ((report & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) ? ", ALARM" : "")
	      << ((report & VisDQMIndex::SUMMARY_PROP_REPORT_ERROR) ? ", ERROR" : "")
	      << ((report & VisDQMIndex::SUMMARY_PROP_REPORT_WARN) ? ", WARN" : "")
	      << ((report & VisDQMIndex::SUMMARY_PROP_REPORT_OTHER) ? ", OTHER" : "")
	      << ((s->properties & VisDQMIndex::SUMMARY_PROP_EFFICIENCY_PLOT) ? ", EFF" : "")
	      << ((s->properties & VisDQMIndex::SUMMARY_PROP_HAS_REFERENCE)
		  ? ", HAS-REFERENCE" : "")
	      << ((s->properties & VisDQMIndex::SUMMARY_PROP_TAGGED)
		  ? ", TAGGED" : "")
	      << "] data-length=" << s->dataLength
	      << " qtest-length=" << s->qtestLength
	      << " object-length=" << s->objectLength
	      << " tag=" << s->tag
	      << " nentries=" << s->nentries
	      << " axis-x=[nbins:" << s->nbins[0]
	      << ", mean:" << s->mean[0]
	      << ", rms:" << s->rms[0]
	      << ", min:" << s->bounds[0][0]
	      << ", max:" << s->bounds[0][1]
	      << "] axis-y=[nbins:" << s->nbins[1]
	      << ", mean:" << s->mean[1]
	      << ", rms:" << s->rms[1]
	      << ", min:" << s->bounds[1][0]
	      << ", max:" << s->bounds[1][1]
	      << "] axis-z=[nbins:" << s->nbins[2]
	      << ", mean:" << s->mean[2]
	      << ", rms:" << s->rms[2]
	      << ", min:" << s->bounds[2][0]
	      << ", max:" << s->bounds[2][1]
	      << "] data='" << data
	      << "' qreports=[";

	    qreports.clear();
	    uint32_t flags = 0;
	    const char *sep = "";
            DQMNet::QReports::const_iterator qi, qe;
	    DQMNet::unpackQualityData(qreports, flags, qdata);
            for (qi = qreports.begin(), qe = qreports.end(); qi != qe; ++qi)
	    {
	      char buf[64];
	      sprintf(buf, "%.*g", DBL_DIG+2, qi->qtresult);
	      std::cout << sep << "[status:" << qi->code
			<< ", result:" << buf
			<< ", name:'" << qi->qtname
			<< "', algorithm:'" << qi->algorithm
			<< "', message:'" << qi->message
			<< "']";
	      sep = ", ";
	    }

	    std::cout << "]\n";
	  }
	  else
	    break;
	}

	delete f;
      }
      else
      {
	std::cout << "WARNING: data file ["
		  << (si->files[VisDQMIndex::MASTER_FILE_INFO] >> 16) << ':'
		  << (si->files[VisDQMIndex::MASTER_FILE_INFO] & 0xffff)
		  << " disappeared before it could be read\n";
      }

      std::cout << "END INFO #" << n << '\n';
    }
  }

  // Dump actual monitor element objects.  We don't really dump the
  // ROOT objects as it would be meaningless, just a MD5 hash of the
  // serialised data.  We do however exercise de-serialising the data
  // back in as ROOT objects and print pointers and type information
  // to prove it worked.
  if (what == DUMP_ALL || what == DUMP_DATA)
  {
    si = samples.begin();
    se = samples.end();
    for (size_t n = 0; si != se; ++si, ++n)
    {
      if (sampleid != ALL_SAMPLES && n != sampleid)
	continue;
      std::cout << "BEGIN DATA #" << n << '\n';
      loadStreamerInfo(streamers.key(si->streamerInfoIdx));

      if (VisDQMFile *f
	  = ix.open(VisDQMIndex::MASTER_FILE_DATA,
		    si->files[VisDQMIndex::MASTER_FILE_DATA] >> 16,
		    si->files[VisDQMIndex::MASTER_FILE_DATA] & 0xffff,
		    VisDQMFile::OPEN_READ))
      {
	for (VisDQMFile::ReadHead rdhead(f, (uint64_t) n << 44);
	     ! rdhead.isdone(); rdhead.next())
	{
	  uint64_t key;
	  void *begin;
	  void *end;

	  rdhead.get(&key, &begin, &end);
	  uint64_t keyparts[4] = { (key >> 44) & 0xfffff, (key >> 40) & 0xf,
				   (key >> 20) & 0xfffff, key & 0xfffff };
	  if (keyparts[0] == n)
	  {
	    size_t len = (char *) end - (char *) begin;
	    MD5Digest md5;
	    md5.update(begin, len);

	    TBufferFile buf(TBufferFile::kRead, len, begin, kFALSE);
	    buf.Reset();
	    TObject *obj = extractNextObject(buf);
	    TObject *ref = extractNextObject(buf);

	    std::cout
	      << "ME [sample:" << keyparts[0] << ", category:" << keyparts[1]
	      << ", lumiend:" << keyparts[2] << ", nameidx:" << keyparts[3]
	      << "] data-length=" << len
	      << " object=[ptr:" << (void *) obj
	      << ", type:" << (obj ? typeid(*obj).name() : "(none)")
	      << ", name:'" << (obj ? obj->GetName() : "")
	      << "'] ref=[ptr:" << (void *) ref
	      << ", type:" << (ref ? typeid(*ref).name() : "(none)")
	      << ", name:'" << (ref ? ref->GetName() : "")
	      << "'] md5=" << md5.format() << "\n";
	  }
	  else
	    break;
	}

	delete f;
      }
      else
      {
	std::cout << "WARNING: data file ["
		  << (si->files[VisDQMIndex::MASTER_FILE_DATA] >> 16) << ':'
		  << (si->files[VisDQMIndex::MASTER_FILE_DATA] & 0xffff)
		  << " disappeared before it could be read\n";
      }

      std::cout << "END DATA #" << n << '\n';
    }
  }

  ix.finishRead();
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------
/** Stream a single sample from the DQM GUI Index into an intermediate
    file that can be used to populate the index again. Avoid any
    intersection with ROOT to go parallel. The output file name will
    be identical to the input filename used to originally import the
    sample, with the extension replaced by .dat.  */
static int
streamout(const Filename &indexdir, size_t sampleid)
{
  VisDQMFile *master;
  VisDQMIndex ix(indexdir);
  std::list<VisDQMIndex::Sample> samples;
  StringAtomTree pathnames(1000000);
  StringAtomTree objnames(2500000);
  StringAtomTree streamers(100);
  DQMNet::QReports qreports;

  // no way in dumping more than one sample at a time, for the moment.
  ASSERT(sampleid != ALL_SAMPLES);

  // Read the master catalogue. We need all the info anyway.
  DEBUG(1, "starting index read\n");
  ix.beginRead(master);
  for (VisDQMFile::ReadHead rdhead(master, 0); ! rdhead.isdone(); rdhead.next())
  {
    void *begin;
    void *end;
    uint64_t key;
    uint64_t hipart;

    rdhead.get(&key, &begin, &end);
    hipart = key & 0xffffffff00000000ull;
    if (hipart == VisDQMIndex::MASTER_SAMPLE_RECORD)
      samples.push_back(*(const VisDQMIndex::Sample *)begin);
    else
      break;
  }

  readStrings(pathnames, master, VisDQMIndex::MASTER_SOURCE_PATHNAME);
  readStrings(objnames, master, VisDQMIndex::MASTER_OBJECT_NAME);
  readStrings(streamers, master, VisDQMIndex::MASTER_TSTREAMERINFO);

  // Now output the selected parts, walking over one sample at a time.
  std::list<VisDQMIndex::Sample>::iterator si;
  std::list<VisDQMIndex::Sample>::iterator se;

  si = samples.begin();
  se = samples.end();
  for (size_t n = 0; si != se; ++si, ++n)
  {
    if (n != sampleid)
      continue;
    size_t delim;
    std::string fname;
    delim = pathnames.key(si->sourceFileIdx).rfind("/");
    if (delim != std::string::npos)
      fname = pathnames.key(si->sourceFileIdx).substr(delim+1);
    else
      fname = pathnames.key(si->sourceFileIdx).substr(0, std::string::npos);
    delim = fname.rfind(".");
    // Change whatever file extension was used to register the file
    // into .dat
    if (delim != std::string::npos)
      fname.replace(delim, fname.size()-delim, ".dat");

    std::ofstream out(fname.c_str(), std::ios::out);
    if (!out)
      throw VisDQMError(0, fname,
                        StringFormat("failed to open file #%1")
                        .arg(fname));

    out
      << si->runNumber
      << " " << si->numObjects
      << " " << si->numEvents
      << " " << si->numLumiSections
      << " " << si->runStartTime
      << " " << si->processedTime
      << " " << hexlify(streamers.key(si->streamerInfoIdx));
    out << "\n" << MEINFOBOUNDARY << "\n";
    VisDQMFile * f = ix.open(VisDQMIndex::MASTER_FILE_INFO,
                             si->files[VisDQMIndex::MASTER_FILE_INFO] >> 16,
                             si->files[VisDQMIndex::MASTER_FILE_INFO] & 0xffff,
                             VisDQMFile::OPEN_READ);
    VisDQMFile * ff = ix.open(VisDQMIndex::MASTER_FILE_DATA,
                              si->files[VisDQMIndex::MASTER_FILE_DATA] >> 16,
                              si->files[VisDQMIndex::MASTER_FILE_DATA] & 0xffff,
                              VisDQMFile::OPEN_READ);
    if (f)
    {
      for (VisDQMFile::ReadHead rdhead(f, (uint64_t) n << 44);
           ! rdhead.isdone(); rdhead.next())
      {
        uint64_t key;
        void *begin;
        void *end;
        DQMNet::DataBlob rawdata;

        rdhead.get(&key, &begin, &end);
        uint64_t keyparts[4] = { (key >> 44) & 0xfffff, (key >> 40) & 0xf,
                                 (key >> 20) & 0xfffff, key & 0xfffff };
        if (keyparts[0] == n)
        {
          VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
          const char *data = (s->dataLength ? (const char *) (s+1) : "");
          const char *qdata
            = (s->qtestLength ? ((const char *) (s+1) + s->dataLength) : "");

          out
            << objnames.key(keyparts[3]) << "\n"
	    << keyparts[1]
            << " " << keyparts[2]
            << " " << s->properties
            << " " << s->dataLength
            << " " << s->qtestLength
            << " " << s->objectLength
            << " " << s->tag
            << " " << s->nentries
            << " " << s->nbins[0];
	  writeDouble(out, " ", s->mean[0]);
	  writeDouble(out, " ", s->rms[0]);
	  writeDouble(out, " ", s->bounds[0][0]);
	  writeDouble(out, " ", s->bounds[0][1]);
	  out << " " << s->nbins[1];
	  writeDouble(out, " ", s->mean[1]);
	  writeDouble(out, " ", s->rms[1]);
	  writeDouble(out, " ", s->bounds[1][0]);
	  writeDouble(out, " ", s->bounds[1][1]);
	  out  << " " << s->nbins[2];
	  writeDouble(out, " ", s->mean[2]);
	  writeDouble(out, " ", s->rms[2]);
	  writeDouble(out, " ", s->bounds[2][0]);
	  writeDouble(out, " ", s->bounds[2][1]);
          if (s->dataLength)
	  {
            std::string sdata;
            sdata.reserve(s->dataLength);
            sdata.append(data, s->dataLength);
            out << " data: " << hexlify(sdata);
	  }
          if (s->qtestLength)
          {
            std::string qt;
            qt.reserve(s->qtestLength);
            qt.append(qdata, s->qtestLength);
            out << " qt: " << hexlify(qt);
          }
          out << std::endl;
        }
        else
          break;
      }
      delete f;
    }
    else
    {
      std::cout << "WARNING: data file ["
                << (si->files[VisDQMIndex::MASTER_FILE_INFO] >> 16) << ':'
                << (si->files[VisDQMIndex::MASTER_FILE_INFO] & 0xffff)
                << " disappeared before it could be read\n";
    }

    if (ff)
    {
      out << MEROOTBOUNDARY << "\n";
      for (VisDQMFile::ReadHead rddata(ff, (uint64_t) n << 44);
           ! rddata.isdone(); rddata.next())
      {
        uint64_t key;
        void *begin;
        void *end;
        DQMNet::DataBlob rawdata;

        rddata.get(&key, &begin, &end);
        uint64_t keyparts[4] = { (key >> 44) & 0xfffff, (key >> 40) & 0xf,
                                 (key >> 20) & 0xfffff, key & 0xfffff };
        if (keyparts[0] == n)
        {
	  rawdata.clear();
	  rawdata.insert(rawdata.end(),
			 (unsigned char *) begin,
			 (unsigned char *) end);
	  out << hexlify(rawdata);
	  out << std::endl;
	}
	else
	  break;
      }
      delete ff;
    }
    else
    {
      std::cout << "WARNING: data file ["
		<< (si->files[VisDQMIndex::MASTER_FILE_DATA] >> 16) << ':'
		<< (si->files[VisDQMIndex::MASTER_FILE_DATA] & 0xffff)
		<< " disappeared before it could be read\n";
    }
  }
  ix.finishRead();
  return EXIT_SUCCESS;
}

// ----------------------------------------------------------------------
/** Show a message on how to use this program. */
static int
showusage(void)
{
  std::cerr << "Usage: " << app.name()
	    << " [--[no-]debug] TASK OPTIONS\n\n  "
	    << app.name() << " [OPTIONS] create INDEX-DIRECTORY\n  "
	    << app.name() << " [OPTIONS] add [--dataset DATASET-NAME] INDEX-DIRECTORY [ROOT|DAT FILE...]\n  "
	    << app.name() << " [OPTIONS] remove --dataset DATASET-NAME --run RUNNR INDEX-DIRECTORY\n  "
	    << app.name() << " [OPTIONS] merge INDEX-DIRECTORY [IMPORT-INDEX-DIRECTORY...]\n  "
	    << app.name() << " [OPTIONS] dump [--sample SAMPLE-ID] INDEX-DIRECTORY [{ catalogue | info | data | all }]\n  "
	    << app.name() << " [OPTIONS] stream --sample SAMPLE-ID INDEX-DIRECTORY\n";
  return EXIT_FAILURE;
}

/** The main program.  Process and verify arguments and call various
    task-specific implementations. */
int main(int argc, char **argv)
{
  // Install base debugging support.
  DebugAids::failHook(&onAssertFail);
  Signal::handleFatal(argv[0], IOFD_INVALID, 0, 0, FATAL_OPTS);

  // Re-capture signals from ROOT after ROOT has initialised.
  ROOT::GetROOT();
  for (int sig = 1; sig < NSIG; ++sig) Signal::revert(sig);
  Signal::handleFatal(argv[0], IOFD_INVALID, 0, 0, FATAL_OPTS);

  // Check and process arguments.
  app = Filename(argv[0]).nondirectory();
  TaskType task;
  DumpType dumpwhat = DUMP_CATALOGUE;
  Filename indexdir;
  int32_t runnr = -1;
  std::string dataset;
  size_t sampleid = ALL_SAMPLES;
  std::list<SampleInfo> samples;
  std::list<FileInfo> files;
  std::list<Filename> mergeix;
  int arg;

  // Check top-level arguments.
  for (arg = 1; arg < argc; ++arg)
    if (! strcmp(argv[arg], "--no-debug"))
      debug = 0;
    else if (! strcmp(argv[arg], "--debug")
	     || ! strcmp(argv[arg], "-d"))
      debug++;
    else if (! strcmp(argv[arg], "--help"))
      return showusage();
    else if (! strcmp(argv[arg], "--"))
    {
      ++arg;
      break;
    }
    else if (argv[arg][0] == '-')
    {
      std::cerr << app.name() << ": unexpected option '" << argv[arg] << "'\n";
      return showusage();
    }
    else
      break;

  // Check which task we should execute.
  if (arg < argc)
  {
    if (! strcmp(argv[arg], "create"))
      ++arg, task = TASK_CREATE;
    else if (! strcmp(argv[arg], "add"))
      ++arg, task = TASK_ADD;
    else if (! strcmp(argv[arg], "remove"))
      ++arg, task = TASK_REMOVE;
    else if (! strcmp(argv[arg], "merge"))
      ++arg, task = TASK_MERGE;
    else if (! strcmp(argv[arg], "dump"))
      ++arg, task = TASK_DUMP;
    else if (! strcmp(argv[arg], "stream"))
      ++arg, task = TASK_STREAM;
    else
    {
      std::cerr << app.name() << ": unrecognised task parameter '"
		<< argv[arg] << "', expected one of create, add or remove\n";
      return showusage();
    }
  }
  else
  {
    std::cerr << app.name() << ": not enough arguments\n";
    return showusage();
  }

  // Check for task specific options.
  if (task == TASK_ADD)
  {
    for ( ; arg < argc; ++arg)
      if (! strcmp(argv[arg], "--dataset"))
      {
	if (arg < argc-1)
	  dataset = argv[++arg];
	else
	{
	  std::cerr << app.name() << ": --dataset option requires a value\n";
	  return showusage();
	}
      }
      else if (! strcmp(argv[arg], "--"))
      {
	++arg;
	break;
      }
      else if (argv[arg][0] == '-')
      {
	std::cerr << app.name() << ": unexpected option '" << argv[arg] << "'\n";
	return showusage();
      }
      else
	break;
  }
  else if (task == TASK_REMOVE)
  {
    for ( ; arg < argc; ++arg)
      if (! strcmp(argv[arg], "--dataset"))
      {
	if (arg < argc-1)
	  dataset = argv[++arg];
	else
	{
	  std::cerr << app.name() << ": --dataset option requires a value\n";
	  return showusage();
	}
      }
      else if (! strcmp(argv[arg], "--run"))
      {
	if (arg < argc-1)
	{
	  errno = 0;
	  char *end = 0;
	  runnr = strtol(argv[++arg], &end, 10);
	  if (errno != 0 || !end || *end || runnr < 0)
	  {
	    std::cerr << app.name() << ": invalid run number '" << argv[arg] << "'\n";
	    return showusage();
	  }
	}
	else
	{
	  std::cerr << app.name() << ": --run option requires a value\n";
	  return showusage();
	}
      }
      else if (! strcmp(argv[arg], "--"))
      {
	++arg;
	break;
      }
      else if (argv[arg][0] == '-')
      {
	std::cerr << app.name() << ": unexpected option '" << argv[arg] << "'\n";
	return showusage();
      }
      else
	break;
  }
  else if (task == TASK_DUMP)
  {
    for ( ; arg < argc; ++arg)
      if (! strcmp(argv[arg], "--sample"))
      {
	char *end = 0;
	if (arg < argc-1)
	  sampleid = strtoul(argv[++arg], &end, 10);
	else
	{
	  std::cerr << app.name() << ": --sample option requires a value\n";
	  return showusage();
	}
      }
      else
	break;
  }
  else if (task == TASK_STREAM)
  {
    for ( ; arg < argc; ++arg)
    {
      if (! strcmp(argv[arg], "--sample"))
      {
	char *end = 0;
	if (arg < argc-1)
	  sampleid = strtoul(argv[++arg], &end, 10);
	else
	{
	  std::cerr << app.name() << ": --sample option requires a value\n";
	  return showusage();
	}
	arg++;
	break;
      }
      else
      {
	std::cerr << app.name() << ": --sample option is mandatory when streaming a sample\n";
	return showusage();
      }
    }
  }

  // Next option should be the index directory.
  if (arg < argc)
    indexdir = argv[arg++];
  else
  {
    std::cerr << app.name() << ": not enough arguments\n";
    return showusage();
  }

  // Now check for remaining (non-option) task parameters.
  if (task == TASK_CREATE)
  {
    if (arg != argc)
    {
      std::cerr << app.name() << ": too many arguments to 'create' task\n";
      return showusage();
    }

    if (indexdir.exists())
    {
      std::cerr << indexdir.name() << ": directory already exists\n";
      return EXIT_FAILURE;
    }

    DEBUG(1, "index '" << indexdir.name() << "'\n");
  }
  else if (task == TASK_REMOVE)
  {
    if (arg != argc)
    {
      std::cerr << app.name() << ": too many arguments to 'remove' task\n";
      return showusage();
    }

    if (! indexdir.exists())
    {
      std::cerr << indexdir.name() << ": no such directory\n";
      return EXIT_FAILURE;
    }

    if (! indexdir.isDirectory())
    {
      std::cerr << indexdir.name() << ": not a directory\n";
      return EXIT_FAILURE;
    }

    if (runnr < 0)
    {
      std::cerr << app.name() << ": no run number given\n";
      return EXIT_FAILURE;
    }

    if (dataset.empty())
    {
      std::cerr << app.name() << ": no dataset name given\n";
      return EXIT_FAILURE;
    }

    if (! rxdataset.exactMatch(dataset))
    {
      std::cerr << dataset << ": invalid dataset name\n";
      return EXIT_FAILURE;
    }

    DEBUG(1, "index '" << indexdir.name()
	  << "', dataset '" << dataset
	  << "', run " << runnr << "\n");
  }
  else if (task == TASK_MERGE)
  {
    if (! indexdir.exists())
    {
      std::cerr << indexdir.name() << ": no such directory\n";
      return EXIT_FAILURE;
    }

    if (! indexdir.isDirectory())
    {
      std::cerr << indexdir.name() << ": not a directory\n";
      return EXIT_FAILURE;
    }

    DEBUG(1, "index '" << indexdir.name() << "\n");

    // Build list of index directories to import.
    for ( ; arg < argc; ++arg)
    {
      Filename otherix(argv[arg]);
      if (! otherix.exists())
      {
	std::cerr << otherix.name() << ": no such directory\n";
	return EXIT_FAILURE;
      }

      if (! otherix.isDirectory())
      {
	std::cerr << otherix.name() << ": not a directory\n";
	return EXIT_FAILURE;
      }

      mergeix.push_back(otherix);
    }
  }
  else if (task == TASK_ADD)
  {
    if (! indexdir.exists())
    {
      std::cerr << indexdir.name() << ": no such directory\n";
      return EXIT_FAILURE;
    }

    if (! indexdir.isDirectory())
    {
      std::cerr << indexdir.name() << ": not a directory\n";
      return EXIT_FAILURE;
    }

    if (! dataset.empty() && ! rxdataset.exactMatch(dataset))
    {
      std::cerr << dataset << ": invalid dataset name\n";
      return EXIT_FAILURE;
    }

    DEBUG(1, "index '" << indexdir.name()
	  << "', default dataset '" << dataset << "'\n");

    // Build list of files to process, determine and verify all file
    // parameters in the process.
    for ( ; arg < argc; ++arg)
    {
      SampleInfo si;
      si.type = TYPE_OTHER;
      si.dataset = dataset;
      si.runnr = 0;
      si.index = 0;

      files.push_back(FileInfo());
      FileInfo &fi = files.back();
      std::string filename(argv[arg]);
      fi.fullpath = filename;
      size_t hash = filename.find('#');
      if (hash != std::string::npos)
      {
	fi.path = filename.substr(hash+1, std::string::npos);
	fi.container = filename.substr(0, hash);
      }
      else
      {
	fi.path = argv[arg];
	fi.container = argv[arg];
      }
      fi.sample = 0;

      DEBUG(1, "considering file '" << fi.container.name() << "'\n");
      if (! fi.container.exists() || ! fi.container.isRegular())
      {
	std::cerr << fi.container.name() << ": no such file\n";
	return EXIT_FAILURE;
      }

      if (! fi.container.isReadable())
      {
	std::cerr << fi.container.name() << ": file not readable\n";
	return EXIT_FAILURE;
      }

      if (! fileInfoFromName(fi, si))
	return EXIT_FAILURE;

      std::list<SampleInfo>::iterator di, de;
      for (di = samples.begin(), de = samples.end(); di != de; ++di)
	if (di->type == si.type
	    && di->runnr == si.runnr
	    && di->dataset == si.dataset
	    && di->version == si.version)
	{
	  fi.sample = &*di;
	  break;
	}

      if (di == de)
      {
	samples.push_back(si);
	fi.sample = &samples.back();
	fi.sample->index = samples.size()-1;
      }

      if (! verifyFileInfo(fi))
	return EXIT_FAILURE;
    }
  }
  else if (task == TASK_DUMP)
  {
    if (! indexdir.exists())
    {
      std::cerr << indexdir.name() << ": no such directory\n";
      return EXIT_FAILURE;
    }

    if (! indexdir.isDirectory())
    {
      std::cerr << indexdir.name() << ": not a directory\n";
      return EXIT_FAILURE;
    }

    dumpwhat = DUMP_CATALOGUE;
    if (arg < argc)
    {
      if (! strcmp(argv[arg], "catalogue"))
	++arg, dumpwhat = DUMP_CATALOGUE;
      else if (! strcmp(argv[arg], "info"))
	++arg, dumpwhat = DUMP_INFO;
      else if (! strcmp(argv[arg], "data"))
	++arg, dumpwhat = DUMP_DATA;
      else if (! strcmp(argv[arg], "all"))
	++arg, dumpwhat = DUMP_ALL;
      else
      {
	std::cerr << app.name() << ": unexpected dump option '"
		  << argv[arg] << "', expected one of index, info,"
		  << " data, all\n";
	return EXIT_FAILURE;
      }

      if (arg != argc)
      {
	std::cerr << app.name() << ": too many arguments to 'dump' task\n";
	return EXIT_FAILURE;
      }
    }
  }
  else if (task == TASK_STREAM)
  {
    if (! indexdir.exists())
    {
      std::cerr << indexdir.name() << ": no such directory\n";
      return EXIT_FAILURE;
    }

    if (! indexdir.isDirectory())
    {
      std::cerr << indexdir.name() << ": not a directory\n";
      return EXIT_FAILURE;
    }
  }
  else
  {
    std::cerr << app.name() << ": internal error at line " << __LINE__ << '\n';
    return EXIT_FAILURE;
  }

  // Now execute the task.
  try
  {
    if (task == TASK_CREATE)
      return initIndex(indexdir);
    else if (task == TASK_ADD)
      return addFiles(indexdir, files);
    else if (task == TASK_REMOVE)
      return removeFiles(indexdir, dataset, runnr);
    else if (task == TASK_MERGE)
      return mergeIndexes(indexdir, mergeix);
    else if (task == TASK_DUMP)
      return dumpIndex(indexdir, dumpwhat, sampleid);
    else if (task == TASK_STREAM)
      return streamout(indexdir, sampleid);
    else
    {
      std::cerr << app.name() << ": internal error, unknown task\n";
      return EXIT_FAILURE;
    }
  }
  catch (Error &e)
  {
    std::cerr << app.name() << ": error: " << e.explain() << "\n";
    return EXIT_FAILURE;
  }

  // Unreachable.
  return EXIT_SUCCESS;
}
