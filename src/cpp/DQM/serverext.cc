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
#include "boost/gil/image.hpp"
#include "boost/gil/typedefs.hpp"
#undef HAVE_PROTOTYPES // conflict between python and libjpeg
#undef HAVE_STDLIB_H // conflict between python and libjpeg
#include "boost/gil/extension/io/jpeg_io.hpp"
#include "boost/gil/extension/io/png_io.hpp"
#include "boost/gil/extension/numeric/kernel.hpp"
#include "boost/gil/extension/numeric/convolve.hpp"
#include "boost/gil/extension/numeric/sampler.hpp"
#include "boost/gil/extension/numeric/resample.hpp"
#include "boost/lambda/algorithm.hpp"
#include "boost/lambda/casts.hpp"
#include "rtgu/image/rescale.hpp"
#include "rtgu/image/filters.hpp"

#include <set>
#include <cfloat>
#include <fstream>
#include <stdexcept>
#include <ext/hash_map>
#include <dlfcn.h>
#include <math.h>

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


namespace boost { namespace gil {
  /** Kernel for sinc filter of radius 2.0.  NB: Some say sinc(x) is
      sin(x)/x.  Others say it's sin(PI*x)/(PI*x), a horizontal
      compression of the former which is zero at integer values.  We
      use the latter.  */
  struct sinc_filter
  {
    static float width(void) { return 2.0f; }
    static float filter(float x)
      {
	if (x == 0.0) return 1.0;
	return sinf(M_PI*x)/(M_PI*x);
      }
  };

  /** Kernel for Lanczos filter of radius 3.0. */
  struct lanczos_filter
  {
    static float width(void) { return 3.0f; }
    static float filter(float x)
      {
	if (x < 0.0f)
	  x = -x;
	if (x < 3.0f)
	  return sinc_filter::filter(x) * sinc_filter::filter(x/3.0);
	return 0.0f;
      }
  };

  template <typename DstPixel, typename SrcPixel>
  class blendop
  {
    typedef typename channel_type<DstPixel>::type DstChannel;
    typedef typename channel_type<SrcPixel>::type SrcChannel;
    typedef channel_traits<DstChannel> DstInfo;
    typedef channel_traits<SrcChannel> SrcInfo;
    float f_;
  public:
    blendop(float f) : f_(f) {}
    DstChannel operator()(SrcChannel a, SrcChannel b) const
      { return channel_convert<DstChannel>
	  (std::min(SrcInfo::max_value(),
		    std::max(SrcInfo::min_value(),
			     SrcChannel(a + f_ * (b - a))))); }
  };

  /** Blend two source views @a and @b to a destination view @a dst
      with a blending coefficient @a f.  The result destination image
      will be $a + f * (b - a)$.  Hence f=0 results in source a, f=1
      results in source b, f values (0, 1) blend the sources a and b,
      and f>1 results in boosting source b and subtracting some part
      of source a. */
  template <typename SrcView1, typename SrcView2, typename Scalar, typename DstView>
  inline void blend(const SrcView1 &a, const SrcView2 &b, Scalar f, DstView &dst)
  {
    typedef typename SrcView1::value_type SrcPixel;
    typedef typename DstView::value_type DstPixel;
    assert(a.dimensions() == dst.dimensions());
    assert(b.dimensions() == dst.dimensions());
    int height = a.height();
    for (int rr = 0; rr < height; ++rr)
    {
      typename SrcView1::x_iterator ai = a.row_begin(rr);
      typename SrcView1::x_iterator ae = a.row_end(rr);
      typename SrcView2::x_iterator bi = b.row_begin(rr);
      typename DstView::x_iterator di = dst.row_begin(rr);
      for ( ; ai != ae; ++ai, ++bi, ++di)
	static_transform(*ai, *bi, *di, blendop<DstPixel,SrcPixel>(f));
    }
  }

  /** Sharpen source image @a src by @a amount and write the result
      into destination image @a dst.  Performs the sharpening using an
      operation known as unsharp masking: subtracting a slightly
      smoothed version of the image from the image itself.
      Intermediate results are in 32-bit float pixel format. */
  inline void sharpen(rgb8_view_t &src, rgb8_view_t &dst, float amount)
  {
    static const float SMOOTH[] = { 1./7., 5./7., 1./7. };
    kernel_1d_fixed<float,3> smooth(SMOOTH,1);
    rgb32f_image_t smoothed(src.width(), src.height());
    copy_and_convert_pixels(src, view(smoothed));
    convolve_rows_fixed<rgb32f_pixel_t>
      (const_view(smoothed), smooth, view(smoothed));
    convolve_cols_fixed<rgb32f_pixel_t>
      (const_view(smoothed), smooth, view(smoothed));
    blend(const_view(smoothed),
	  color_converted_view<rgb32f_pixel_t>(src),
	  amount, dst);
  }
}}

using namespace lat;
using namespace boost;
namespace ext = __gnu_cxx;
namespace py = boost::python;
typedef std::pair<std::string, std::string> StringPair;
typedef std::set<StringAtom> StringAtomSet;
typedef std::vector<StringAtom> StringAtomList;
typedef std::map<StringAtom, StringAtomList> StringAtomParentList;

static pthread_rwlock_t parentslock;
static StringAtomParentList parents;
static StringAtomTree stree(1024*1024);
static Regexp RX_THOUSANDS("(\\d)(\\d{3}($|\\D))");
static Regexp RX_OUTER_WHITE("^\\s+|\\s+$");
static Regexp RX_CMSSW_VERSION("CMSSW(?:_[0-9])+(?:_pre[0-9]+)?");

namespace __gnu_cxx
{
  template<> struct hash<StringAtom>
  {
    size_t operator()(const StringAtom &s) const
      { return s.index(); }
  };

  template<> struct hash<std::string>
  {
    size_t operator()(const std::string &s) const
      { return __stl_hash_string(s.c_str()); }
  };
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class PyReleaseInterpreterLock
{
  PyThreadState *save_;
public:
  PyReleaseInterpreterLock(void) { save_ = PyEval_SaveThread(); }
  ~PyReleaseInterpreterLock(void) { PyEval_RestoreThread(save_); }
};

class Lock
{
  pthread_mutex_t *lock_;
public:
  Lock(pthread_mutex_t *lock) { pthread_mutex_lock(lock_ = lock); }
  ~Lock(void)                 { pthread_mutex_unlock(lock_); }
};

class RDLock
{
  pthread_rwlock_t *lock_;
public:
  RDLock(pthread_rwlock_t *lock) { pthread_rwlock_rdlock(lock_ = lock); }
  ~RDLock(void)                  { pthread_rwlock_unlock(lock_); }
};

class WRLock
{
  pthread_rwlock_t *lock_;
public:
  WRLock(pthread_rwlock_t *lock) { pthread_rwlock_wrlock(lock_ = lock); }
  ~WRLock(void)                  { pthread_rwlock_unlock(lock_); }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
struct VisDQMAxisOptions;
struct VisDQMDrawOptions;
struct VisDQMLayoutItem;
struct VisDQMLayoutRow;
struct VisDQMItem;
struct VisDQMStatus;
struct VisDQMShownItem;
struct VisDQMShownRow;
struct VisDQMStatus;
struct VisDQMEventNum;
struct VisDQMSample;
struct VisDQMRegexp;
class VisDQMSource;

typedef std::vector< shared_ptr<VisDQMLayoutItem> >	     VisDQMLayoutItems;
typedef std::vector< shared_ptr<VisDQMLayoutRow> >	     VisDQMLayoutRows;
typedef std::vector< shared_ptr<VisDQMShownItem> >	     VisDQMShownItems;
typedef std::vector< shared_ptr<VisDQMShownRow> >	     VisDQMShownRows;
typedef ext::hash_map<StringAtom, shared_ptr<VisDQMItem> >   VisDQMItems;
typedef ext::hash_map<StringAtom, VisDQMStatus>		     VisDQMStatusMap;
typedef std::map<StringAtom, VisDQMDrawOptions>		     VisDQMDrawOptionMap;
typedef std::list<VisDQMEventNum>		 	     VisDQMEventNumList;
typedef std::vector<VisDQMSample>		 	     VisDQMSamples;
typedef std::map<std::string, VisDQMRegexp>                  VisDQMRegexps;

struct VisDQMAxisOptions
{
  std::string			min;
  std::string			max;
  std::string			type;
};

struct VisDQMDrawOptions
{
  VisDQMAxisOptions		xaxis;
  VisDQMAxisOptions		yaxis;
  VisDQMAxisOptions		zaxis;
  std::string			drawopts;
  std::string			withref;
};

struct VisDQMLayoutItem
{
  StringAtom			path;
  std::string			desc;
  VisDQMDrawOptions		drawopts;
};

struct VisDQMLayoutRow
{
  VisDQMLayoutItems		columns;
};

struct VisDQMItem
{
  uint32_t			flags;
  uint64_t			version;
  StringAtom			name;
  StringAtomList		parents;
  std::string			data;
  VisDQMSource		 	*plotter;
  shared_ptr<VisDQMLayoutRows>	layout;
};

struct VisDQMShownItem
{
  uint64_t			version;
  int				nalarm;
  int				nlive;
  StringAtom			name;
  std::string			desc;
  VisDQMSource			*plotter;
  VisDQMDrawOptions		drawopts;
};

struct VisDQMShownRow
{
  VisDQMShownItems		columns;
};

struct VisDQMStatus
{
  int				nleaves;
  int				nalarm;
  int				nlive;
};

struct VisDQMEventNum
{
  std::string			subsystem;
  long				runnr;
  long				luminr;
  long				eventnr;
  long				runstart;
};

enum VisDQMSampleType
{
  SAMPLE_LIVE,
  SAMPLE_ONLINE_DATA,
  SAMPLE_OFFLINE_DATA,
  SAMPLE_OFFLINE_RELVAL,
  SAMPLE_OFFLINE_MC,
  SAMPLE_ANY
};

struct VisDQMSample
{
  VisDQMSampleType		type;
  long				runnr;
  std::string			dataset;
  std::string			version;
  VisDQMSource			*origin;
  uint64_t			time;
};

static const char *sampleTypeLabel[] =
{
  "live",
  "online_data",
  "offline_data",
  "offline_relval",
  "offline_mc",
  "any"
};

struct VisDQMRegexp
{
  static const size_t REGEXPS_PER_LOCK = 1024;
  shared_ptr<Regexp>		rx;
  std::vector<bool>		matched;
  std::vector<bool>		tried;
  std::vector<pthread_rwlock_t>	locks;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
static std::ostream &
lograw(const char *label)
{
  Time now = Time::current();
  return std::cout
    << now.format(true, "[%d/%b/%Y:%H:%M:%S.")
    << now.nanoformat(3, 3) << ']'
    << label;
}

static std::ostream &
loginfo(void)
{
  return lograw("  INFO: ");
}

static std::ostream &
logerr(void)
{
  return lograw("  ERROR: ");
}

static std::ostream &
logwarn(void)
{
  return lograw("  WARNING: ");
}

static std::string
thousands(const std::string &arg)
{
  std::string result;
  std::string tmp(arg);
  result.reserve(arg.size() + arg.size()/3 + 1);

  while (true)
  {
    result = StringOps::replace(tmp, RX_THOUSANDS, "\\1'\\2");
    if (result == tmp)
      break;

    tmp = result;
  }

  return result;
}

static void
copyopts(const py::dict &opts, std::map<std::string, std::string> &options)
{
  for (py::stl_input_iterator<std::string> i(opts), e; i != e; ++i)
  {
    std::string key(*i);
    py::extract<std::string> value(opts.get(key));
    if (value.check())
      options[key] = value();
  }
}

/// Convert binary data to hex string.
template <class T>
static std::string
hexlify(const T &x)
{
  std::string result;
  result.reserve(2*x.size());
  for (size_t i = 0, e = x.size(); i != e; ++i)
  {
    char buf[3];
    sprintf(buf, "%02x", (unsigned) (unsigned char) x[i]);
    result += buf[0];
    result += buf[1];
  }
  return result;
}

/// Check whether the @a path is a subdirectory of @a ofdir.  Returns
/// true both for an exact match and any nested subdirectory.
static bool
isSubdirectory(const std::string &ofdir, const std::string &path)
{
  return (ofdir.empty()
	  || (path.size() >= ofdir.size()
	      && path.compare(0, ofdir.size(), ofdir) == 0
	      && (path.size() == ofdir.size()
		  || path[ofdir.size()] == '/')));
}

static void
splitPath(std::string &dir, std::string &name, const std::string &path)
{
  size_t slash = path.rfind('/');
  if (slash != std::string::npos)
  {
    dir.append(path, 0, slash);
    name.append(path, slash+1, std::string::npos);
  }
  else
    name = path;
}

static std::string
stringToJSON(const std::string &x, bool emptyIsNone = false)
{
  if (emptyIsNone && x.empty())
    return "None";

  char buf[8];
  std::string result;
  result.reserve(2 * x.size());
  result += '"';
  for (size_t i = 0, e = x.size(); i < e; ++i)
  {
    unsigned int ch = (unsigned char) x[i];
    if (ch == '\\')
    {
      result += '\\';
      result += '\\';
    }
    else if (ch == '"')
    {
      result += '\\';
      result += (char) ch;
    }
    else if (ch == '\n')
    {
      result += '\\';
      result += 'n';
    }
    else if (ch == '\r')
    {
      result += '\\';
      result += 'r';
    }
    else if (ch == '\t')
    {
      result += '\\';
      result += 't';
    }
    else if (! isprint(ch))
    {
      sprintf(buf, "\\u%04x", ch);
      result += buf;
    }
    else
      result += (char) ch;
  }

  result += '"';
  return result;
}

static shared_ptr<Regexp>
rx(const std::string &match, int options = 0)
{
  shared_ptr<Regexp> r;
  if (! match.empty())
  {
    r.reset(new Regexp(match, options));
    if (! r->valid())
      throw std::runtime_error(StringFormat
			       ("Invalid regular expression syntax at"
				" character %2 of '%1': %3")
			       .arg(match)
			       .arg(r->errorOffset())
			       .arg(r->errorMessage()));
    r->study();
  }
  return r;
}

static void
makerx(const std::string &rxstr,
       shared_ptr<Regexp> &rxobj,
       std::string &rxerr,
       int options = 0)
{
  try
  {
    if (! rxstr.empty())
      rxobj = rx(rxstr, options);
  }
  catch (Error &e)
  {
    rxerr = e.explainSelf();
  }
  catch (std::exception &e)
  {
    rxerr = e.what();
  }
}

static void
fastrx(VisDQMRegexp &rxobj, const std::string &rxstr)
{
  rxobj.rx = rx(rxstr);
  rxobj.tried.resize(stree.capacity(), false);
  rxobj.matched.resize(stree.capacity(), false);
  rxobj.locks.resize(stree.capacity() / VisDQMRegexp::REGEXPS_PER_LOCK);
  for (size_t i = 0, e = rxobj.locks.size(); i < e; ++i)
    pthread_rwlock_init(&rxobj.locks[i], 0);
}

static bool
fastmatch(VisDQMRegexp *rx, const StringAtom &str)
{
  if (! rx || ! rx->rx)
    return true;

  size_t nlock = str.index() / VisDQMRegexp::REGEXPS_PER_LOCK;
  pthread_rwlock_rdlock(&rx->locks[nlock]);
  assert(str.index() < rx->tried.size());
  bool tried = rx->tried[str.index()];
  bool matched = rx->matched[str.index()];
  pthread_rwlock_unlock(&rx->locks[nlock]);

  if (tried)
    return matched;

  pthread_rwlock_wrlock(&rx->locks[nlock]);
  size_t maxtree = stree.size();
  size_t minidx = nlock * VisDQMRegexp::REGEXPS_PER_LOCK;
  size_t maxidx = minidx + VisDQMRegexp::REGEXPS_PER_LOCK;
  for (size_t i = minidx; i < maxidx && i < maxtree; ++i)
    if (! rx->tried[i])
    {
      bool match = rx->rx->match(stree.key(i));
      rx->tried[i] = true;
      rx->matched[i] = match;
      if (i == str.index())
	matched = match;
    }

  pthread_rwlock_unlock(&rx->locks[nlock]);
  return matched;
}

static void
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

static uint32_t
getType(uint32_t flags)
{
  return flags & VisDQMIndex::SUMMARY_PROP_TYPE_MASK;
}

static bool
isROOTType(uint32_t flags)
{
  uint32_t type = getType(flags);
  return (type >= VisDQMIndex::SUMMARY_PROP_TYPE_TH1F
	  && type <= VisDQMIndex::SUMMARY_PROP_TYPE_TPROF2D);
}

static bool
isScalarType(uint32_t flags)
{
  uint32_t type = getType(flags);
  return (type >= VisDQMIndex::SUMMARY_PROP_TYPE_INT
	  && type <= VisDQMIndex::SUMMARY_PROP_TYPE_STRING);
}

static bool
isBLOBType(uint32_t flags)
{
  return getType(flags) == DQMNet::DQM_PROP_TYPE_DATABLOB;
}

static bool
isIntegerType(uint32_t flags)
{
  return getType(flags) == VisDQMIndex::SUMMARY_PROP_TYPE_INT;
}

static bool
isRealType(uint32_t flags)
{
  return getType(flags) == VisDQMIndex::SUMMARY_PROP_TYPE_REAL;
}

static void
translateDrawOptions(py::dict opts, VisDQMDrawOptions &o)
{
  py::stl_input_iterator<std::string> ki(opts), ke;
  for ( ; ki != ke; ++ki)
  {
    std::string key(*ki);
    std::string value(py::extract<std::string>(opts.get(key)));
    if (key == "withref")        o.withref = value;
    else if (key == "drawopts")  o.drawopts = value;
    else if (key == "xmin")      o.xaxis.min = value;
    else if (key == "xmax")      o.xaxis.max = value;
    else if (key == "xtype")     o.xaxis.type = value;
    else if (key == "ymin")      o.yaxis.min = value;
    else if (key == "ymax")      o.yaxis.max = value;
    else if (key == "ytype")     o.yaxis.type = value;
    else if (key == "zmin")      o.zaxis.min = value;
    else if (key == "zmax")      o.zaxis.max = value;
    else if (key == "ztype")     o.zaxis.type = value;
  }
}

static std::string
formatStartTime(long curStartTime)
{
  Time now = Time::current();
  Time runstart = (curStartTime <= 0 ? now : Time(curStartTime, 0));
  tm nowtm = now.utc();
  tm runtm = runstart.utc();
  bool sameyear = nowtm.tm_year == runtm.tm_year;
  bool samemonth = (sameyear && nowtm.tm_mon == runtm.tm_mon);
  bool sameday = (samemonth && nowtm.tm_mday == runtm.tm_mday);
  const char *fmt = (curStartTime <= 0 ? "(Not recorded)"
		     : sameday ? "Today %H:%M"
		     : samemonth ? "%a %d, %H:%M"
		     : sameyear ? "%a %b %d, %H:%M"
		     : "%a %b %d '%y, %H:%M");
  return runstart.format(false, fmt);
}

static void
getEventInfoNum(const std::string &name,
		const char *data,
		VisDQMEventNumList &eventnums)
{
  long value = -1;
  double real;
  size_t len;

  if ((len = 15) > 0
      && name.size() > len
      && name.compare(name.size() - len, len, "/EventInfo/iRun") == 0)
    value = strtol(data, 0, 10);
  else if ((len = 23) > 0
	   && name.size() > len
	   && name.compare(name.size() - len, len, "/EventInfo/iLumiSection") == 0)
    value = strtol(data, 0, 10);
  else if ((len = 17)
	   && name.size() > len
	   && name.compare(name.size() - len, len, "/EventInfo/iEvent") == 0)
    value = strtol(data, 0, 10);
  else if ((len = 28)
	   && name.size() > len
	   && name.compare(name.size() - len, len, "/EventInfo/runStartTimeStamp") == 0)
    if ((real = atof(data)) > 3600)
      value = long(real+0.5);

  if (value >= 0)
  {
    bool found = false;
    std::string subsys(name, 0, name.size() - len);
    VisDQMEventNumList::iterator li = eventnums.begin();
    VisDQMEventNumList::iterator le = eventnums.end();
    for ( ; li != le; ++li)
      if ((found = (li->subsystem == subsys)))
	break;

    VisDQMEventNum *evnum;
    if (! found)
    {
      eventnums.push_front(VisDQMEventNum());
      evnum = &eventnums.front();
      evnum->subsystem = subsys;
      evnum->runnr = -1;
      evnum->luminr = -1;
      evnum->eventnr = -1;
      evnum->runstart = -1;
    }
    else
      evnum = &*li;

    if (len == 15)
      evnum->runnr = value;
    else if (len == 23)
      evnum->luminr = value;
    else if (len == 17)
      evnum->eventnr = value;
    else if (len == 28)
      evnum->runstart = value;
  }
}

// Pick greatest run/lumi/event number combo seen.
static void
setEventInfoNums(const VisDQMEventNumList &eventnums, VisDQMEventNum &current)
{
  VisDQMEventNumList::const_iterator li = eventnums.begin();
  VisDQMEventNumList::const_iterator le = eventnums.end();
  for ( ; li != le; ++li)
  {
    if (li->runnr > current.runnr)
      current = *li;

    if (li->runnr == current.runnr && li->luminr > current.luminr)
      current.luminr = li->luminr;

    if (li->runnr == current.runnr && li->eventnr > current.eventnr)
      current.eventnr = li->eventnr;

    if (li->runnr == current.runnr
	&& li->runstart >= 0
	&& li->runstart < current.runstart)
      current.runstart = li->runstart;
  }
}

inline bool
orderSamplesByDataset(const VisDQMSample &a, const VisDQMSample &b)
{
  // - Order by sample type (ascending).
  if (a.type < b.type)
    return true;
  else if (a.type > b.type)
    return false;

  // - Order by dataset (descending, natural string comparison).
  if (natcmpstr(a.dataset, b.dataset))
    return true;
  else if (natcmpstr(b.dataset, a.dataset))
    return false;

  // - Order by run (descending).
  if (a.runnr > b.runnr)
    return true;
  else if (a.runnr < b.runnr)
    return false;

  // - Order by version (descending, natural string comparison).
  if (natcmpstr(b.version, a.version))
    return true;
  else if (natcmpstr(a.version, b.version))
    return false;

  // Should not come here, but give deterministic order.
  return &a < &b;
}

inline bool
orderSamplesByRun(const VisDQMSample &a, const VisDQMSample &b)
{
  // - Order by sample type (ascending).
  if (a.type < b.type)
    return true;
  else if (a.type > b.type)
    return false;

  // - Order by run (descending).
  if (a.runnr > b.runnr)
    return true;
  else if (a.runnr < b.runnr)
    return false;

  // - Order by version (descending, natural string comparison).
  if (natcmpstr(b.version, a.version))
    return true;
  else if (natcmpstr(a.version, b.version))
    return false;

  // - Order by dataset (descending, natural string comparison).
  if (natcmpstr(a.dataset, b.dataset))
    return true;
  else if (natcmpstr(b.dataset, a.dataset))
    return false;

  // Should not come here, but give deterministic order.
  return &a < &b;
}

static std::string
axisStatsToJSON(uint32_t nbins[3], double mean[3], double rms[3], double bounds[3][2], int axis)
{
  if (std::isfinite(mean[axis]) && std::isfinite(rms[axis]))
    return StringFormat("{ \"nbins\": %1, \"mean\": %2, \"rms\": %3,"
			" \"min\": %4, \"max\": %5 }")
      .arg(nbins[axis])
      .arg(mean[axis])
      .arg(rms[axis])
      .arg(bounds[axis][0])
      .arg(bounds[axis][1]);
  if (!std::isfinite(mean[axis]))
  {
    if (!std::isfinite(rms[axis]))
      return StringFormat("{ \"nbins\": %1, \"mean\": \"NaN or Inf\", \"rms\": \"Nan or Inf\","
			  " \"min\": %2, \"max\": %3 }")
	.arg(nbins[axis])
	.arg(bounds[axis][0])
	.arg(bounds[axis][1]);
    return StringFormat("{ \"nbins\": %1, \"mean\": \"NaN or Inf\", \"rms\": %2,"
		      " \"min\": %3, \"max\": %4 }")
    .arg(nbins[axis])
    .arg(rms[axis])
    .arg(bounds[axis][0])
    .arg(bounds[axis][1]);
  }
  return StringFormat("{ \"nbins\": %1, \"mean\": %2, \"rms\": \"Nan or Inf\","
		      " \"min\": %3, \"max\": %4 }")
    .arg(nbins[axis])
    .arg(mean[axis])
    .arg(bounds[axis][0])
    .arg(bounds[axis][1]);
}

// Format objects to json, with full data if requested.
static void
objectToJSON(const std::string &name,
	     const char *value,
	     const char *qdata,
	     DQMNet::DataBlob &rawdata,
	     uint64_t lumisect,
	     uint32_t flags,
	     uint32_t tag,
	     double nentries,
	     uint32_t nbins[3],
	     double mean[3],
	     double rms[3],
	     double bounds[3][2],
	     DQMNet::QReports &qreports,
	     std::string &qstr,
	     std::string &result)
{
  if (! result.empty())
    result += ", ";

  DQMNet::QReports::const_iterator qi, qe;
  uint32_t type = flags & DQMNet::DQM_PROP_TYPE_MASK;
  uint32_t report = flags & DQMNet::DQM_PROP_REPORT_MASK;
  uint32_t qflags = 0;
  const char *qsep = "";

  qreports.clear();
  qstr.clear();

  DQMNet::unpackQualityData(qreports, qflags, qdata);
  for (qi = qreports.begin(), qe = qreports.end(); qi != qe; ++qi)
  {
    char buf[64];
    sprintf(buf, "%.*g", DBL_DIG+2, qi->qtresult);
    qstr += StringFormat("%1 {\"status\": %2,\"result\": %3,"
			 " \"name\": %4, \"algorithm\": %5,"
			 " \"message\": %6}")
      .arg(qsep)
      .arg(qi->code)
      .arg(buf)
      .arg(stringToJSON(qi->qtname))
      .arg(stringToJSON(qi->algorithm))
      .arg(stringToJSON(qi->message));
    qsep = ", ";
  }

  result += StringFormat("{\"obj\": %1, \"properties\": {\"kind\": \"%2\","
			 " \"type\": \"%3\", \"lumisect\": \"%4\", \"report\": { \"alarm\": %5,"
			 " \"error\": %6, \"warn\": %7, \"other\": %8 },"
			 " \"hasref\": %9, \"tagged\": %10 },\"tag\": %11,"
			 " \"qresults\": [%12], \"nentries\": %13,"
			 " \"stats\": { \"x\": %14, \"y\": %15, \"z\": %16 },"
			 " \"%17\": \"%18\"}\n")
    .arg(stringToJSON(name))
    .arg(type == DQMNet::DQM_PROP_TYPE_INVALID ? "INVALID"
	 : type <= DQMNet::DQM_PROP_TYPE_SCALAR ? "SCALAR"
	 : "ROOT")
    .arg(type == DQMNet::DQM_PROP_TYPE_INT ? "INT"
	 : type == DQMNet::DQM_PROP_TYPE_REAL ? "REAL"
	 : type == DQMNet::DQM_PROP_TYPE_STRING ? "STRING"
	 : type == DQMNet::DQM_PROP_TYPE_TH1F ? "TH1F"
	 : type == DQMNet::DQM_PROP_TYPE_TH1S ? "TH1S"
	 : type == DQMNet::DQM_PROP_TYPE_TH1D ? "TH1D"
	 : type == DQMNet::DQM_PROP_TYPE_TH2F ? "TH2F"
	 : type == DQMNet::DQM_PROP_TYPE_TH2S ? "TH2S"
	 : type == DQMNet::DQM_PROP_TYPE_TH2D ? "TH2D"
	 : type == DQMNet::DQM_PROP_TYPE_TH3F ? "TH3F"
	 : type == DQMNet::DQM_PROP_TYPE_TH3S ? "TH3S"
	 : type == DQMNet::DQM_PROP_TYPE_TH3D ? "TH3D"
	 : type == DQMNet::DQM_PROP_TYPE_TPROF ? "TPROF"
	 : type == DQMNet::DQM_PROP_TYPE_TPROF2D ? "TPROF2D"
	 : type == DQMNet::DQM_PROP_TYPE_DATABLOB ? "DATABLOB"
	 : "OTHER")
    .arg((unsigned long)lumisect)
    .arg((report & DQMNet::DQM_PROP_REPORT_ALARM) ? 1 : 0)
    .arg((report & DQMNet::DQM_PROP_REPORT_ERROR) ? 1 : 0)
    .arg((report & DQMNet::DQM_PROP_REPORT_WARN) ? 1 : 0)
    .arg((report & DQMNet::DQM_PROP_REPORT_OTHER) ? 1 : 0)
    .arg((flags & DQMNet::DQM_PROP_HAS_REFERENCE) ? 1 : 0)
    .arg((flags & DQMNet::DQM_PROP_TAGGED) ? 1 : 0)
    .arg(tag)
    .arg(qstr)
    .arg(nentries)
    .arg(axisStatsToJSON(nbins, mean, rms, bounds, 0))
    .arg(axisStatsToJSON(nbins, mean, rms, bounds, 1))
    .arg(axisStatsToJSON(nbins, mean, rms, bounds, 2))
    .arg(isScalarType(flags) ? "value" : "rootobj")
    .arg(isScalarType(flags) ? StringOps::replace(std::string(value),"\"","\\\"") : hexlify(rawdata));
}

static void
sampleToJSON(const VisDQMSample &sample, std::string &result)
{
  result += "{\"type\":\"";
  result += sampleTypeLabel[sample.type];
  result += "\", \"run\":\"";
  result += StringFormat("%1").arg(sample.runnr);
  result += "\", \"dataset\":\"";
  result += sample.dataset;
  result += "\", \"version\":\"";
  result += sample.version;
  result += "\"}";
}

static std::string
sampleToJSON(const VisDQMSample &sample)
{
  std::string result;
  result.reserve(64 + sample.dataset.size());
  sampleToJSON(sample, result);
  return result;
}

static std::string
samplesToJSON(VisDQMSamples &samples)
{
  std::string result;

  size_t size = 0;
  VisDQMSamples::iterator i, e;
  for (i = samples.begin(), e = samples.end(); i != e; ++i)
    size += 100 + i->dataset.size();
  result.reserve(size);

  const char *comma = "";
  VisDQMSampleType last = SAMPLE_LIVE;
  for (i = samples.begin(), e = samples.end(); i != e; ++i)
  {
    if (result.empty() || i->type != last)
    {
      last = i->type;
      if (! result.empty())
	result += "]}, ";
      result += "{\"type\":\"";
      result += sampleTypeLabel[i->type];
      result += "\", \"items\":[";
      comma = "";
    }

    result += comma;
    sampleToJSON(*i, result);
    comma = ", ";
  }

  if (! result.empty())
    result += "]}";

  return result;
}

static VisDQMSample
sessionSample(const py::dict &session)
{
  VisDQMSample result;
  long type = py::extract<long>(session.get("dqm.sample.type"));
  result.runnr = py::extract<long>(session.get("dqm.sample.runnr"));
  result.dataset = py::extract<std::string>(session.get("dqm.sample.dataset"));
  result.origin = 0;

  if (type < SAMPLE_LIVE || type > SAMPLE_OFFLINE_MC)
    throw std::runtime_error(StringFormat
			     ("invalid session dqm.sample.type: %1")
			     .arg(type));

  result.type = (VisDQMSampleType) type;

  if (result.runnr < 0)
    throw std::runtime_error(StringFormat
			     ("invalid session dqm.sample.runnr: %1")
			     .arg(result.runnr));
  return result;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/** Base class for providing DQM objects. */
class VisDQMSource
{
protected:
  void
  clearobj(DQMNet::Object &obj)
    {
      obj.flags = DQMNet::DQM_PROP_TYPE_INVALID;
      obj.tag = 0;
      obj.version = 0;
      obj.dirname = 0;
      obj.hash = 0;
      obj.lastreq = 0;
      obj.objname.clear();
      obj.scalar.clear();
      obj.rawdata.clear();
      obj.qdata.clear();
    }

  void
  clearattr(VisDQMIndex::Summary &attrs)
    {
      memset(&attrs, 0, sizeof(attrs));
    }

public:
  VisDQMSource(void)
    {}

  virtual ~VisDQMSource(void)
    {}

  virtual const char *
  plotter(void) const
    {
      return "unknown";
    }

  virtual void
  prepareSession(py::dict session)
    {}

  virtual void
  getdata(const VisDQMSample &sample,
	  const std::string &path,
	  std::string &streamers,
	  DQMNet::Object &obj)
    {
      streamers.clear();
      clearobj(obj);
    }

  virtual void
  getattr(const VisDQMSample &sample,
	  const std::string &path,
	  VisDQMIndex::Summary &attrs,
	  std::string &xstreamers,
	  DQMNet::Object &xobj)
    {
      clearattr(attrs);
    }

  virtual void
  prescan(void)
    {}

  virtual void
  scan(VisDQMItems &result,
       const VisDQMSample &sample,
       VisDQMEventNum &current,
       VisDQMRegexp *rxmatch,
       Regexp *rxsearch,
       bool *alarm,
       std::string *layoutroot,
       VisDQMRegexp *rxlayout)
    {}

  virtual void
  json(const VisDQMSample &sample,
       const std::string &rootpath,
       bool fulldata,
       bool lumisect,
       std::string &result,
       double &stamp,
       const std::string &name,
       std::set<std::string> &dirs)
    {}

  virtual void
  samples(VisDQMSamples &samples)
    {}

  virtual void
  getcert(VisDQMSample &sample,
	  const std::string &path,
	  const std::string &variableName,
	  std::vector<VisDQMIndex::Summary> &attrs,
	  std::vector<double> &axisvals,
	  std::string &binlabels)
    {}

};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
struct IMGOPT { std::string label; Regexp &verify; };
static Regexp RX_OPT_INT("^[0-9]+$");
static Regexp RX_OPT_FLOAT("^([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)?$");
static Regexp RX_OPT_DRAWOPT("^[A-Za-z ]*$");
static Regexp RX_OPT_REFTYPE("^(|object|reference|overlay)$");
static Regexp RX_OPT_AXISTYPE("^(def|lin|log)$");
static Regexp RX_OPT_TREND_TYPE("^(num-(entries|bins|bytes)|value|"
				"[xyz]-(min|max|bins|mean(-rms|-min-max)?))$");
static IMGOPT STDIMGOPTS[] = {
  { "w",	RX_OPT_INT },
  { "h",	RX_OPT_INT },
  { "drawopts",	RX_OPT_DRAWOPT },
  { "ref",	RX_OPT_REFTYPE },
  { "xtype",	RX_OPT_AXISTYPE },
  { "xmin",	RX_OPT_FLOAT },
  { "xmax",	RX_OPT_FLOAT },
  { "ytype",	RX_OPT_AXISTYPE },
  { "ymin",	RX_OPT_FLOAT },
  { "ymax",	RX_OPT_FLOAT },
  { "ztype",	RX_OPT_AXISTYPE },
  { "zmin",	RX_OPT_FLOAT },
  { "zmax",	RX_OPT_FLOAT },
  { "ktest",	RX_OPT_FLOAT },
  { "",		RX_OPT_INT }

};

static IMGOPT TRENDIMGOPTS[] = {
  { "w",	RX_OPT_INT },
  { "h",	RX_OPT_INT },
  { "trend",	RX_OPT_TREND_TYPE },
  { "xmin",	RX_OPT_FLOAT },
  { "xmax",	RX_OPT_FLOAT },
  { "",		RX_OPT_INT }
};

/** Utility to fetch an image from visDQMRender process.  Validates the
    image request options and builds a spec string to send to the
    backend render process.  Waits for the backend to reply with an
    image and returns either (MimeType, ImageData) tuple on success or
    (None, None) on failure.

    The remaining arguments should be the object name, the streamers
    and the streamed object data, each a string.  The streamers should
    be serialised ROOT TStreamerInfo required to deserialise the data
    (for ROOT objects) or empty (for scalar values).  The data should
    be either serialised ROOT objects or the scalar value as a string.

    The call will wait as long as the render process takes to reply.  A
    persistent per-thread socket is used to talk to the render process.

    PIL is used to scale down images that would be very small, as ROOT
    is not good at drawing small anti-aliased images.

    A cache of recent identical images is maintained, keyed by the spec
    and serialised object data, to avoid unnecessarily re-rendering the
    frequently accessed images. */
class VisDQMRenderLink
{
  static const uint32_t DQM_MSG_GET_IMAGE_DATA = 4;
  static const uint32_t DQM_MSG_REPLY_IMAGE_DATA = 105;
  static const int MIN_WIDTH = 532;
  static const int MIN_HEIGHT = 400;
  static const int IMAGE_CACHE_SIZE = 1024;

  typedef boost::shared_ptr<SubProcess> SubProcessPtr;

  struct Image;
  struct Server
  {
    Filename		path;
    Filename		sockpath;
    LocalSocket		socket;
    SubProcessPtr	proc;
    std::list<Image *>	pending;
    std::string		lastimg;
    bool		checkme;
  };

  struct Image
  {
    int64_t		id;
    uint64_t		hash;
    uint64_t		version;
    uint32_t		flags;
    uint32_t		tag;
    std::string		pathname;
    std::string		imagespec;
    std::string		databytes;
    std::string		qdata;
    std::string		pngbytes;
    size_t		numparts;
    int			width;
    int			height;
    int			inuse;
    bool		busy;
  };

  Filename		dir_;
  bool			debug_;
  bool			quiet_;
  std::string		plugin_;
  Pipe			logpipe_;
  SubProcess		logger_;
  std::vector<Server>	servers_;
  Image			cache_[IMAGE_CACHE_SIZE];
  pthread_mutex_t	lock_;
  pthread_cond_t	imgavail_;
  pthread_cond_t	srvavail_;
  pthread_t		thread_;
  static VisDQMRenderLink *s_instance;

public:
  static VisDQMRenderLink *
  instance(void)
    {
      return s_instance;
    }

  VisDQMRenderLink(const std::string &basedir,
                   const std::string &logdir,
		   const std::string &plugin,
		   int nproc,
		   bool debug)
    {
      assert(! s_instance);
      s_instance = this;
      plugin_ = plugin;
      debug_ = debug;
      dir_ = basedir;
      quiet_ = false;

      // Initialise cache to 5000 unused images.
      for (int i = 0; i < IMAGE_CACHE_SIZE; ++i)
	lrunuke(cache_[i]);

      // Initialise requested number of sub-processes, all feeding to one logger.
      std::string logout(logdir + "/renderlog-%Y%m%d.log");
      logger_.run(Argz("rotatelogs", logout.c_str(), "86400").argz(),
		  SubProcess::Write | SubProcess::NoCloseError | SubProcess::Search,
		  &logpipe_);

      servers_.resize(nproc);
      for (int i = 0; i < nproc; ++i)
      {
        char buf[32];
	snprintf(buf, sizeof(buf), "render-%d", i);
        servers_[i].path = Filename(basedir, buf);
	servers_[i].sockpath = Filename(servers_[i].path, "socket");
	Filename::makedir(servers_[i].path, 0755, true, true);
	Filename::remove(servers_[i].sockpath, false, true);
	startsrv(servers_[i]);
      }

      // Initialise locks for image cache and server list.
      pthread_mutex_init(&lock_, 0);
      pthread_cond_init(&imgavail_, 0);
      pthread_cond_init(&srvavail_, 0);
    }

  ~VisDQMRenderLink(void)
    {
    }

  void
  start(void)
    {
      pthread_mutex_lock(&lock_);
      quiet_ = false;
      pthread_mutex_unlock(&lock_);
      logme() << "INFO: render link enabled\n";
    }

  void
  stop(void)
    {
      logme() << "INFO: render link disabled\n";
      pthread_mutex_lock(&lock_);
      quiet_ = true;
      pthread_mutex_unlock(&lock_);
    }

  bool
  render(std::string &image,
	 std::string &type,
	 const std::string &path,
         const std::map<std::string, std::string> &opts,
         const std::string *streamers,
         DQMNet::Object *obj,
	 size_t numobj,
	 IMGOPT *IMGOPTS)
    {
      // Verify incoming arguments and build perliminary specification.
      std::string finalspec;
      std::string reqspec;
      int destwidth = 0;
      int destheight = 0;
      for (int opt = 0; ! IMGOPTS[opt].label.empty(); ++opt)
      {
	std::map<std::string, std::string>::const_iterator pos
	  = opts.find(IMGOPTS[opt].label);
        if (pos != opts.end())
	{
	  if (! IMGOPTS[opt].verify.match(pos->second)
	      && !(IMGOPTS[opt].label == "drawopts" && pos->second.empty()))
	  {
	    logme() << "WARNING: option '" << pos->first << "' value '"
		    << pos->second << "' does not pass verification\n";
	    return false;
	  }

	  if (opt == 0) // w
	  {
	    destwidth = atoi(pos->second.c_str());
	    continue;
	  }

	  if (opt == 1) // h
	  {
	    destheight = atoi(pos->second.c_str());
	    continue;
	  }

	  if (! reqspec.empty())
	    reqspec += ';';
	  reqspec += pos->first;
	  reqspec += '=';
	  reqspec += pos->second;
	}
      }

      // If absurd image size was requested, return nothing.
      if (destwidth <= 0 || destheight <= 0
	  || destwidth >= 2500 || destheight >= 2500)
	return false;

      // If the requested image is small, generate a larger image in
      // the space aspect ratio and use rescale the image with high
      // quality (lanczos) anti-aliasing resizing filter. ROOT itself
      // generates hideously ugly output for small images.
      int width = destwidth;
      int height = destheight;
      char buf[64];
      sprintf(buf, "%sw=%d;h=%d",
	      reqspec.empty() ? "" : ";",
	      destwidth, destheight);
      finalspec = reqspec + buf;

      if (width < MIN_WIDTH || height < MIN_HEIGHT)
      {
        double wratio = double(MIN_WIDTH) / double(width);
        double hratio = double(MIN_HEIGHT) / double(height);
	if (wratio > hratio)
	{
	  width = MIN_WIDTH;
	  height = int(height * wratio + .5);
	}
        else
        {
	  height = MIN_HEIGHT;
	  width = int(width * hratio + .5);
	}
      }

      sprintf(buf, "%sw=%d;h=%d",
	      reqspec.empty() ? "" : ";",
	      width, height);
      reqspec += buf;

      Image protoimg;
      initimg(protoimg, path, reqspec, streamers, obj, numobj, width, height);

      // See if we have the image already cached. If we are going to
      // resize the image, we will try both image specifications, in
      // the order we need them.
      Lock gate(&lock_);
      return ! quiet_ && makepng(image, type, protoimg, finalspec, destwidth, destheight);
    }

private:
  static std::ostream &
  logme(void)
    {
      Time now = Time::current();
      return std::cerr
	<< now.format(true, "%Y-%m-%d %H:%M:%S.")
	<< now.nanoformat(3, 3)
	<< " visDQMRenderLink[" << getpid()
	<< '/' << pthread_self() << "]: ";
    }

  void
  startsrv(Server &srv)
    {
      assert(srv.pending.empty());
      IOChannel *null = 0;
      const char *serverArgz[] = {
	"visDQMRender", "--state-directory", srv.path.name(),
	"--load", plugin_.c_str(), (debug_ ? "--debug" : 0), 0
      };
      srv.proc.reset(new SubProcess(serverArgz,
				    SubProcess::First
				    | SubProcess::Search
				    | SubProcess::NoCloseOutput
				    | SubProcess::NoCloseError,
				    null, logpipe_.sink(),
				    logpipe_.sink()));
      srv.checkme = true;
    }

  static void
  initimg(Image &proto,
	  const std::string &path,
	  const std::string &imagespec,
	  const std::string *streamers,
	  const DQMNet::Object *obj,
	  size_t numobj,
	  int width,
	  int height)
    {
      assert(numobj);
      lrunuke(proto);
      proto.id = Time::current().ns();
      proto.version = obj[0].version;
      proto.flags = obj[0].flags;
      proto.tag = obj[0].tag;
      proto.pathname = path;
      proto.qdata = obj[0].qdata;
      proto.imagespec = imagespec;
      proto.numparts = numobj;
      proto.width = width;
      proto.height = height;

      size_t datalen = 0;
      for (size_t i = 0; i < numobj; ++i)
      {
 	assert(obj[i].flags == 0 // totally missing
	       || isScalarType(obj[i].flags)
	       || isROOTType(obj[i].flags)
	       || isBLOBType(obj[i].flags));
	assert(i == 0 || !isScalarType(obj[i].flags));
	assert(i == 0
	       || (isBLOBType(obj[0].flags) && numobj <= 2) // stripchart
	       || getType(obj[i].flags) == getType(obj[0].flags)); // other

        if (isROOTType(obj[i].flags) || isBLOBType(obj[i].flags))
	  datalen += 2*sizeof(uint32_t) + streamers[i].size() + obj[i].rawdata.size();
	else
	  datalen += obj[i].scalar.size();
      }

      proto.databytes.reserve(datalen);
      for (size_t i = 0; i < numobj; ++i)
      {
	if (isROOTType(obj[i].flags) || isBLOBType(obj[i].flags))
	{
	  uint32_t words[2];
	  words[0] = streamers[i].size();
	  words[1] = obj[i].rawdata.size();
	  proto.databytes.append((const char *) words, sizeof(words));
	  proto.databytes.append(streamers[i]);
	  if (obj[i].rawdata.size())
	    proto.databytes.append((const char *) &obj[i].rawdata[0],
				   obj[i].rawdata.size());
	}
	else
	  proto.databytes = obj[i].scalar;
      }

      proto.hash = (((uint64_t) DQMNet::dqmhash(&path[0], path.size()) << 32)
		    | DQMNet::dqmhash(&proto.databytes[0], datalen));
    }

  static void
  lrunuke(Image &img)
    {
      img.id = 0;
      img.hash = 0;
      img.version = 0;
      img.flags = 0;
      img.tag = 0;
      img.pathname.clear();
      std::string().swap(img.imagespec);
      std::string().swap(img.databytes);
      std::string().swap(img.qdata);
      std::string().swap(img.pngbytes);
      img.numparts = 0;
      img.width = 0;
      img.height = 0;
      img.busy = false;
      img.inuse = 0;
    }

  Image &
  lruimg(int &imgidx, Image &proto)
    {
      int lruidx = -1;
      imgidx = -1;

      for (int i = 0; i < IMAGE_CACHE_SIZE; ++i)
      {
	if (cache_[i].id
	    && cache_[i].hash == proto.hash
	    && cache_[i].width == proto.width
	    && cache_[i].height == proto.height
	    && cache_[i].databytes == proto.databytes
	    && cache_[i].qdata == proto.qdata
	    && cache_[i].pathname == proto.pathname
	    && cache_[i].imagespec == proto.imagespec)
	{
	  imgidx = i;
	  break;
	}
	else if (cache_[i].id == 0 && (lruidx < 0 || cache_[lruidx].id))
	  lruidx = i;
	else if (cache_[i].inuse)
	  continue;
	else if (lruidx < 0 || cache_[i].id < cache_[lruidx].id)
	  lruidx = i;
      }

      assert(imgidx >= 0 || lruidx >= 0);
      if (imgidx < 0)
      {
	assert(! cache_[lruidx].inuse);
	assert(! cache_[lruidx].busy);
	lrunuke(cache_[lruidx]);
	return cache_[lruidx];
      }
      else
	return cache_[imgidx];
    }

  void
  blacklistimg(Server &srv)
    {
      typedef std::map<std::string, Time> BlackList;

      if (srv.lastimg.empty())
	return;

      BlackList bad;
      Filename blacklist(dir_, "blacklist.txt");
      if (blacklist.exists() && blacklist.isReadable())
      {
	std::string line;
	std::ifstream f(blacklist);
	do
	{
	  getline(f, line);
	  if (line.empty())
	    continue;

	  StringList s = StringOps::split(line, ' ', 0, 2, 0, -1);
	  bad[s[1]] = Time::ValueType(atof(s[0].c_str())*1e9);
	} while (f);
      }

      bad[srv.lastimg] = Time::current();
      std::ofstream f(blacklist);
      for (BlackList::iterator i = bad.begin(), e = bad.end(); i != e; ++i)
	f << i->second << ' ' << i->first << std::endl;
    }

  void
  requestimg(Image &img, std::string &imgbytes)
    {
      assert(imgbytes.empty());

      // Pick the least loaded server to talk to.  While we do that,
      // check the servers are still running; restart those that are
      // not running yet or any more.  Prefer already running server
      // to one we just restarted, as the former will have a socket
      // ready.
      size_t proc = servers_.size();
      size_t maybe = servers_.size();
      for (size_t i = 0; i < servers_.size(); ++i)
      {
        Server &srv = servers_[i];
	SubProcessPtr p = srv.proc;
	if (srv.checkme && p && p->done())
	{
	  pid_t pid = p->pid();
	  int code = p->wait();
	  char buf[128];
	  if (! p->exitBySignal(code))
	    sprintf(buf, " exited with code %d", p->exitStatus(code));
	  else
	    sprintf(buf, " was killed by signal %d", p->exitSignal(code));

	  logme() << "WARNING: restarting render process #"
		  << i << ", process " << pid
		  << buf << " while rendering '"
		  << srv.lastimg << "'\n";
	  blacklistimg(srv);
	  srv.socket.abort();
	  srv.proc.reset();
	  srv.pending.clear();
	  p.reset();
	}

	if (! p)
	{
	  startsrv(srv);
	  if (maybe == servers_.size())
	    maybe = i;
	}
	else if (proc == servers_.size()
		 || srv.socket.fd() == IOFD_INVALID
		 || (srv.pending.size() < servers_[proc].pending.size()))
	  proc = i;
      }

      assert(proc < servers_.size() || maybe < servers_.size());
      Server &srv = servers_[proc < servers_.size() ? proc : maybe];
      LocalSocket &sock = srv.socket;
      if (sock.fd() == IOFD_INVALID)
      {
	// Wait up to one second to connect to the server.
	int ntry;
	for (ntry = 10; ntry >= 0; TimeInfo::msleep(100), --ntry)
	  if (srv.sockpath.exists())
	    break;

	try
	{
	  if (ntry >= 0)
	  {
	    sock.create();
	    sock.connect(srv.sockpath.name());
	  }
	}
	catch (Error &e)
	{
	  logme() << "WARNING: failed to connect to server #" << maybe
		  << ": " << e.explain () << std::endl;
	  sock.abort();
	}

	if (sock.fd() == IOFD_INVALID)
	{
	  logme() << "WARNING: no connection to server #" << maybe << '\n';
	  srv.checkme = true;
	  return;
	}
      }

      // Wait for the socket to become available, issue an image
      // request and wait for result.  While waiting, release the lock
      // so others can do work.
      assert(sock.fd() != IOFD_INVALID);
      assert(img.inuse > 0);
      assert(img.busy);
      srv.lastimg = img.pathname;
      srv.pending.push_back(&img);

      while (srv.pending.size() && srv.pending.front() != &img)
	pthread_cond_wait(&srvavail_, &lock_);

      if (srv.pending.empty())
      {
	srv.checkme = true;
	return;
      }

      pthread_mutex_unlock(&lock_);

      try
      {
	uint32_t words[11] =
	  {
	    sizeof(words) + img.pathname.size() + img.imagespec.size()
	    + img.databytes.size() + img.qdata.size(),
	    DQM_MSG_GET_IMAGE_DATA,
	    img.flags,
	    img.tag,
	    (img.version >> 0 ) & 0xffffffff,
	    (img.version >> 32) & 0xffffffff,
	    img.numparts,
	    img.pathname.size(),
	    img.imagespec.size(),
	    img.databytes.size(),
	    img.qdata.size()
	  };

	std::string message;
	message.reserve(words[0]);
	message.append((const char *)&words[0], sizeof(words));
	message.append(img.pathname);
	message.append(img.imagespec);
	message.append(img.databytes);
	message.append(img.qdata);
	sock.xwrite(&message[0], message.size());
	message.clear();

	if (sock.xread(&words[0], 2*sizeof(uint32_t)) == 2*sizeof(uint32_t)
	    && words[0] >= 2*sizeof(uint32_t)
	    && words[1] == DQM_MSG_REPLY_IMAGE_DATA)
	{
	  message.resize(words[0] - 2*sizeof(uint32_t), '\0');
	  if (sock.xread(&message[0], message.size()) == message.size())
	    imgbytes = message;
	}
      }
      catch (Error &e)
      {
	logme() << "ERROR: failed to retrieve image: " << e.explain() << std::endl;
	sock.abort();
      }

      // Reacquire locks and wake up waiters.
      pthread_mutex_lock(&lock_);
      assert(img.inuse > 0);
      assert(img.busy);
      srv.checkme = true;
      if (sock.fd() == IOFD_INVALID)
	srv.pending.clear();
      else
      {
	if (! imgbytes.empty())
	{
	  srv.checkme = false;
	  srv.lastimg.clear();
	  compress(img, imgbytes);
	}
	srv.pending.pop_front();
      }
      pthread_cond_broadcast(&srvavail_);
    }

  void
  resizeimg(std::string &imgdata, Image &destimg, const Image &srcspec)
    {
      using namespace boost::gil;
      std::string newdata;
      assert(destimg.busy);
      assert(destimg.inuse);
      assert(destimg.pngbytes.empty());
      assert(imgdata.size() == size_t(srcspec.width) * size_t(srcspec.height) * 3);
      pthread_mutex_unlock(&lock_);
      try
      {
        newdata.resize(destimg.width * destimg.height * 3, '\x00');
	rgb8c_view_t srcview
	  = interleaved_view(srcspec.width, srcspec.height,
			     (rgb8c_pixel_t *) &imgdata[0],
			     srcspec.width * 3);
	rgb8_view_t destview
	  = interleaved_view(destimg.width, destimg.height,
			     (rgb8_pixel_t *) &newdata[0],
			     destimg.width * 3);
	rescale(srcview, destview, catmull_rom_filter()); // lanczos_filter()
	sharpen(destview, destview, 1.4);
      }
      catch (std::exception &e)
      {
	// Ignore actual error, we handle the situation below.
	logme() << "WARNING: failed to scale image: "
		<< e.what() << std::endl;
	std::string().swap(newdata);
      }
      catch (...)
      {
	logme() << "WARNING: failed to scale image, unknown reason\n";
	std::string().swap(newdata);
      }

      imgdata.swap(newdata);
      pthread_mutex_lock(&lock_);
    }

  void
  compress(Image &img, const std::string &imgbytes)
    {
      assert(! imgbytes.empty());
      assert(img.pngbytes.empty());
      assert(img.inuse);
      assert(img.busy);
      FILE *png = 0;
      char *pngdata = 0;
      size_t pngsize = 0;
      std::string pngbytes;
      pthread_mutex_unlock(&lock_);

      try
      {
	if ((png = open_memstream(&pngdata, &pngsize)))
	{
	  using namespace boost::gil;
	  boost::gil::detail::png_writer writer(png);
	  writer.apply(interleaved_view
		       (img.width, img.height,
			(const rgb8_pixel_t *) &imgbytes[0],
			img.width * 3));
	  fflush(png);
	  pngbytes.append(pngdata, pngsize);
	  fclose(png);
	  png = 0;
	}
      }
      catch (std::exception &e)
      {
	logme() << "WARNING: failed to encode png image: "
		<< e.what() << std::endl;
	std::string().swap(pngbytes);
      }
      catch (...)
      {
	logme() << "WARNING: failed to encode png image, unknown reason\n";
	std::string().swap(pngbytes);
      }

      if (png)
	fclose(png);

      if (pngdata)
	free(pngdata);

      pthread_mutex_lock(&lock_);
      img.pngbytes = pngbytes;
    }

  bool
  makepng(std::string &imgdata,
	  std::string &imgtype,
	  Image &protoreq,
	  const std::string &spec,
	  int width,
	  int height)
    {
      imgdata.clear();
      imgtype.clear();
      int imgidx;
      Image proto = protoreq;
      proto.id = Time::current().ns();
      proto.imagespec = spec;
      proto.width = width;
      proto.height = height;
      Image &img = lruimg(imgidx, proto);
      if (imgidx >= 0)
	img.inuse++;
      else
      {
	std::string srcbytes;
	assert(! img.busy);
	assert(! img.inuse);
	assert(img.pngbytes.empty());
        img = proto;
	img.busy = true;
	img.inuse++;

	// If we are not rescaling, request and compress image.
	if (width == protoreq.width && height == protoreq.height)
	  requestimg(img, srcbytes);

	// Otherwise, we are rescaling. First get original bigger image,
	// then rescale and compress the image. We might either find the
	// original as-is, in which case we need to expand the PNG form,
	// or create it, in which case we get the raw bytes for free.
	else
	{
	  Image &srcimg = lruimg(imgidx, protoreq);
	  if (imgidx >= 0)
	    srcimg.inuse++;
	  else
	  {
	    assert(! srcimg.busy);
	    assert(! srcimg.inuse);
	    assert(srcimg.pngbytes.empty());
	    srcimg = protoreq;
	    srcimg.busy = true;
	    srcimg.inuse++;
	    requestimg(srcimg, srcbytes);
	    assert(srcimg.inuse > 0);
	    assert(srcimg.busy);
	    srcimg.busy = false;
	    pthread_cond_broadcast(&imgavail_);
	  }

	  while (srcimg.busy)
	    pthread_cond_wait(&imgavail_, &lock_);

	  assert(srcimg.width == protoreq.width);
	  assert(srcimg.height == protoreq.height);
	  if (srcbytes.empty() && ! srcimg.pngbytes.empty())
	    expandpng(srcbytes, srcimg);

	  assert(srcimg.inuse > 0);
	  srcimg.inuse--;
	  assert(img.inuse > 0);
	  if (! srcbytes.empty())
	    resizeimg(srcbytes, img, protoreq);

	  if (! srcbytes.empty())
	    compress(img, srcbytes);
	}

	assert(img.inuse > 0);
	assert(img.busy);
	img.busy = false;
        pthread_cond_broadcast(&imgavail_);
      }

      while (img.busy)
	pthread_cond_wait(&imgavail_, &lock_);

      assert(img.inuse > 0);
      img.inuse--;
      if (! img.pngbytes.empty())
      {
	imgdata = img.pngbytes;
	imgtype = "image/png";
	return true;
      }
      else
	return false;
    }

  void
  expandpng(std::string &imgbytes, const Image &img)
    {
      // FIXME: Lock, unlock?
      FILE *png = 0;
      try
      {
        imgbytes.resize(img.width * img.height * 3, '\0');
	if ((png = fmemopen(const_cast<char *>(&img.pngbytes[0]),
			    img.pngbytes.size(), "r")))
	{
	  using namespace boost::gil;
	  boost::gil::detail::png_reader reader(png);
	  reader.apply(interleaved_view
		       (img.width, img.height,
			(rgb8_pixel_t *) &imgbytes[0],
			img.width * 3));
	  fclose(png);
	  png = 0;
	}
      }
      catch (std::exception &e)
      {
	logme() << "WARNING: failed to decode png image: "
		<< e.what() << std::endl;
	imgbytes.clear();
      }
      catch (...)
      {
	logme() << "WARNING: failed to decode png image, unknown reason\n";
	imgbytes.clear();
      }

      if (png)
	fclose(png);
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMUnknownSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMUnknownSource(void)
    : link_(VisDQMRenderLink::instance())
    {}

  py::tuple
  plot(const std::string &path, py::dict opts)
    {
      std::map<std::string, std::string> options;
      bool imageok = false;
      std::string imagedata;
      std::string imagetype;
      std::string streamers;
      DQMNet::Object obj;
      copyopts(opts, options);

      {
        PyReleaseInterpreterLock nogil;
	streamers.clear();
	clearobj(obj);
        imageok = link_->render(imagedata, imagetype, path, options,
				&streamers, &obj, 1, STDIMGOPTS);
      }

      if (imageok)
	return py::make_tuple(imagetype, imagedata);
      else
	return py::make_tuple(py::object(), py::object());
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/** Dump catalogue and objects out in JSON. */
class VisDQMToJSON
{
public:
  VisDQMToJSON(void)
    {}

  py::tuple
  samples(py::list pysources, py::dict opts)
    {
      double stamp = 0;
      std::string result;
      std::map<std::string, std::string> options;
      std::vector<VisDQMSource *> sources;
      shared_ptr<Regexp> rx;
      copyopts(opts, options);
      sources.reserve(py::len(pysources) + 1);

      // Get all the sources which have a C++ part we can query.
      for (py::stl_input_iterator<py::object> i(pysources), e; i != e; ++i)
      {
	py::extract<VisDQMSource *> src(*i);
	if (src.check())
	  sources.push_back(src());
      }

      {
	PyReleaseInterpreterLock nogil;

	// Get all the samples from all the sources.
	VisDQMSamples samples;
	for (size_t i = 0, e = sources.size(); i != e; ++i)
	  sources[i]->samples(samples);

	// Match dataset names against dataset name regexp if one was given.
	if (options.count("match"))
	{
	  std::string rxerr;
	  makerx(options["match"], rx, rxerr, Regexp::IgnoreCase);
	  if (rx)
	  {
	    VisDQMSamples final;
	    final.reserve(samples.size());
	    for (size_t i = 0, e = samples.size(); i != e; ++i)
	      if (rx->search(samples[i].dataset) >= 0)
		final.push_back(samples[i]);
	    std::swap(final, samples);
	  }
	}

	for (size_t i = 0, e = samples.size(); i != e; ++i)
	  stamp = std::max(stamp, samples[i].time * 1e-9);

	result = StringFormat("{\"samples\": [%1]}").arg(samplesToJSON(samples));
      }

      return py::make_tuple(stamp, result);
    }

  py::tuple
  list(VisDQMSource *layoutsrc,
       VisDQMSource *src,
       int runnr,
       const std::string &dataset,
       const std::string &path,
       py::dict opts)
    {
      VisDQMSample sample = { SAMPLE_ANY, runnr, dataset };
      std::map<std::string, std::string> options;
      std::set<std::string> dirs;
      VisDQMItems    layoutResult;
      double         stamp = 0;
      std::string    result;
      VisDQMRegexp   rxmatch;
      std::string    layoutroot;
      VisDQMEventNum current = { "", -1, -1, -1, -1 };
      bool           alarm = false;
      VisDQMRegexp   rxlayout;
      Regexp         *rxsearch = NULL;

      copyopts(opts, options);
      fastrx(rxmatch, path);
      fastrx(rxlayout, "(^" + path + ").*");

      {
	PyReleaseInterpreterLock nogil;
	src->json(sample, path, options.count("rootcontent") > 0, options.count("lumisect") > 0, result, stamp, "", dirs);

	// Add layout content, in case a layout source had been
	// registered to the main server.
	if (layoutsrc)
	{
	  layoutsrc->scan(layoutResult,
			  sample,
			  current,
			  &rxmatch,
			  rxsearch,
			  &alarm,
			  &layoutroot,
			  &rxlayout);
	  json(layoutResult, sample, src, options, stamp, path, result, dirs);
	}
	result = StringFormat("{\"contents\": [%1]}").arg(result);
      }

      // Return the json we produced.
      return py::make_tuple(stamp, result);
    }

  void json(const VisDQMItems &contents,
	    VisDQMSample &sample,
	    VisDQMSource *src,
	    const std::map<std::string, std::string> &options,
	    double &stamp,
	    const std::string &rootpath,
	    std::string &result,
	    std::set<std::string> &dirs)
    {
      VisDQMItems::const_iterator ci, ce;
      std::set<std::string>::iterator di, de;
      std::set<std::string> newdirs;
      std::string name;
      std::string dir;
      bool doroot = options.count("rootcontent") > 0;
      bool dolumi = options.count("lumisect") > 0;

      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  name.clear();
	  dir.clear();
	  splitPath(dir, name, ci->second->name.string());
	  if (rootpath == dir)
	    /* Keep object directly in rootpath directory */;
	  else if (isSubdirectory(rootpath, dir))
	  {
	    // Object in subdirectory, remember subdirectory part
	    size_t begin = (rootpath.empty() ? 0 : rootpath.size()+1);
	    size_t slash = dir.find('/', begin);
	    if (dirs.find(std::string(dir, begin, slash - begin)) == dirs.end())
	      newdirs.insert(std::string(dir, begin, slash - begin));
	    continue;
	  }
	  else
	    // Object outside directory of interest to us, skip
	    continue;

	  VisDQMItem &o = *ci->second;
	  if (o.layout)
	  {
	    VisDQMLayoutRows &rows = *o.layout;
	    for (size_t nrow = 0; nrow < rows.size(); ++nrow)
	    {
	      VisDQMLayoutRow &row = *rows[nrow];
	      for (size_t ncol = 0; ncol < row.columns.size(); ++ncol)
	      {
		name.clear();
		dir.clear();
		VisDQMLayoutItem &col = *row.columns[ncol];
		const std::string path(col.path.string());
		splitPath(dir, name, path);
		// The check on the name (i.e. ME name) and directory
		// is mandatory here since we want to avoid the case
		// in which part of the layout has been defined using
		// python's None: this, in the current implementation,
		// would have the side effect of including all
		// directories, starting from the root one.
		if (!name.empty() && !dir.empty())
		  src->json(sample, dir, doroot, dolumi, result, stamp, name, dirs);
	      }
	    }
	  }
	}

      // Format sub-directories.
      for (di = newdirs.begin(), de = newdirs.end(); di != de; ++di)
	result += StringFormat(", { \"subdir\": %1 }\n").arg(stringToJSON(*di));

      // Successfully read the data, return.
      return;
    }

};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/** Overlay several ROOT objects on top of each other. */
class VisDQMOverlaySource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMOverlaySource(void)
    : link_(VisDQMRenderLink::instance())
    {}

  virtual const char *
  plotter(void) const
    {
      return "overlay";
    }

  py::tuple
  plot(py::list overlay, py::dict opts)
    {
      std::map<std::string, std::string> options;
      bool imageok = false;
      std::string imagedata;
      std::string imagetype;
      copyopts(opts, options);

      std::vector<std::string> streamers;
      std::vector<DQMNet::Object> objects;
      streamers.reserve(py::len(overlay));
      objects.reserve(py::len(overlay));
      DQMNet::Object obj;
      std::string streamer;
      std::string path;

      // Collect overlaid input objects from the specified sources.
      // The python side provides us checked tuples with the actual
      // provider source object and sample type resolved, and the run
      // number already converted to an integer type.  What we need
      // to do here is to ask for the actual object, and if we are
      // happy with it, stash into the render list.
      for (py::stl_input_iterator<py::tuple> i(overlay), e; i != e; ++i)
      {
	py::tuple item = *i;
	py::extract<VisDQMSource *> src(item[0]);
	if (! src.check())
	  continue;

	VisDQMSource *xsrc = src();
	VisDQMSample sample = { SAMPLE_ANY,
			        py::extract<int>(item[1]),
			        py::extract<std::string>(item[2]) };
	std::string opath = py::extract<std::string>(item[3]);

        {
          PyReleaseInterpreterLock nogil;
	  xsrc->getdata(sample, opath, streamer, obj);
        }

	// If it's a ROOT object, or the first scalar, stack it.
	// If we have at least one object, ensure it's the same
	// fundamental type. Skip invalid or unknown objects.
	if (! objects.empty() && getType(obj.flags) != getType(objects[0].flags))
	  continue;
        if (! objects.empty() && isScalarType(obj.flags))
	  continue;
	if (! isROOTType(obj.flags) && ! isScalarType(obj.flags))
	  continue;

	streamers.push_back(streamer);
	objects.push_back(obj);
	if (path.empty())
	  path = opath;
      }

      // If we ended up with something, issue a bulk draw.
      // Otherwise produce "missing in action" message.
      if (objects.size())
      {
        PyReleaseInterpreterLock nogil;
        imageok = link_->render(imagedata, imagetype, path, options,
				&streamers[0], &objects[0], objects.size(),
				STDIMGOPTS);
      }
      else
      {
        PyReleaseInterpreterLock nogil;
	streamer.clear();
	clearobj(obj);
        imageok = link_->render(imagedata, imagetype, path, options,
				&streamer, &obj, 1, STDIMGOPTS);
      }

      // Return the image we produced.
      if (imageok)
	return py::make_tuple(imagetype, imagedata);
      else
	return py::make_tuple(py::object(), py::object());
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/** Make strip charts of ROOT objects. */
class VisDQMStripChartSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMStripChartSource(void)
    : link_(VisDQMRenderLink::instance())
    {}

  virtual const char *
  plotter(void) const
    {
      return "stripchart";
    }

  py::tuple
  plot(py::list pysources, py::object info, const std::string &path, py::dict opts)
    {
      VisDQMSample cursample = { SAMPLE_ANY, -1 };
      std::map<std::string, std::string> options;
      bool imageok = false;
      std::string imagedata;
      std::string imagetype;
      std::string origdataset;
      std::vector<VisDQMSource *> sources;
      copyopts(opts, options);
      sources.reserve(py::len(pysources) + 1);

      // Get the current source if one was given.
      if (info.ptr() != Py_None)
      {
	py::tuple t = py::extract<py::tuple>(info);
	py::extract<VisDQMSource *> src(t[0]);
	if (src.check())
	{
	  // Extract run number and dataset name.  Remember both the
	  // mutilated and original dataset names, former for relval
	  // and the other for other sample matches.  At this stage we
	  // don't know the type of this sample.
	  cursample.runnr = py::extract<long>(t[1]);
	  origdataset = py::extract<std::string>(t[2]);
	  cursample.dataset = StringOps::remove(origdataset, RX_CMSSW_VERSION);
	}
      }

      // Get all the sources which have a C++ part we can query.
      for (py::stl_input_iterator<py::object> i(pysources), e; i != e; ++i)
      {
	py::extract<VisDQMSource *> src(*i);
	if (src.check())
	  sources.push_back(src());
      }

      // Now let python go and start doing real work.
      {
	PyReleaseInterpreterLock nogil;

	// Get all the samples from all the sources.
	VisDQMSamples samples;
	for (size_t i = 0, e = sources.size(); i != e; ++i)
	  sources[i]->samples(samples);

	// If we have a current run/dataset, filter samples to the
	// same dataset, and ignore cmssw variation in relval samples
	// (note: the same dataset requirement makes live and online
	// interchangeable, which is what we want).  Keep only samples
	// which precede the current sample (if any).
	std::vector<VisDQMSample> final;
	final.reserve(samples.size());
	for (size_t i = 0, e = samples.size(); i != e; ++i)
	{
	  VisDQMSample &s = samples[i];
	  bool include = false;
	  if (cursample.runnr == -1 && cursample.dataset.empty())
	    // accept everything if we have no starting sample
	    include = true;
	  else if (cursample.runnr > 0
		   && (s.runnr <= 0 || s.runnr > cursample.runnr))
	    // skip later runs if we have a run number specified
	    ;
	  else if (s.type != SAMPLE_OFFLINE_RELVAL)
	    // require exact dataset name match for non-relval data
	    include = (s.dataset == origdataset);
	  else if (s.type == SAMPLE_OFFLINE_RELVAL)
	    // require fuzzy dataset name match for relval data
	    include = (StringOps::remove(s.dataset, RX_CMSSW_VERSION)
		       == cursample.dataset);

	  if (include)
	    final.push_back(s);
	}

	// Sort the samples by type and run (or CMSSW version).  This
	// leaves the list sorted for most recent to oldest data.  The
	// current sample is guaranteed to be the first one (if any).
	std::sort(final.begin(), final.end(), orderSamplesByRun);

	// Determine options: number of samples to include, display
	// x-axis as run or time, and whether to exclude histograms
	// with no entries (=~ wasn't present in the run).
	std::string runopt = "n";
	std::string skipopt = "omit";
	std::string axisopt = "axis";
	size_t keep = final.size();
	bool skipempty = false;
	bool axistime = false;

	if (options.find(runopt) != options.end() && ! options[runopt].empty())
	  keep = std::min(keep, (size_t) atoi(options[runopt].c_str()));
	if (options.find(skipopt) != options.end() && options[skipopt] == "empty")
	  skipempty = true;
	if (options.find(axisopt) != options.end() && options[axisopt] == "time")
	  axistime = true;

	// Get the object summary data for the requested object for
	// each sample. Live DQM is special because it cannot provide
	// the attributes -- it provides the full ROOT object, which
	// the plotting engine will deserialise.  We assume here live
	// will only provide one object, and will always make it the
	// last sample if live is included.  Also it has to be a ROOT
	// object, not a scalar; this is handled below by checking
	// only on xobj[1].rawdata, not xobj[1].scalar.
	DQMNet::Object xobj[2];
	std::string streamers[2];
	std::string binlabels;
	std::vector<double> axisvals;
	std::vector<VisDQMIndex::Summary> attrs;
	VisDQMIndex::Summary oneattr;
	attrs.reserve(final.size());
	axisvals.reserve(final.size());
	binlabels.reserve(10 * final.size());
	clearobj(xobj[0]);
	clearobj(xobj[1]);
	for (size_t i = 0, e = final.size(); i != e; ++i)
	{
	  // If we have enough samples, give up.
	  if (attrs.size() == keep)
	    break;

	  // Get the attribute here.
	  final[i].origin->getattr(final[i], path, oneattr,
				   streamers[1], xobj[1]);

	  // Skip if empty and skipping was requested.
	  // We can't know if live is empty or not.
	  if (skipempty
	      && ! oneattr.nentries
	      && final[i].type != SAMPLE_LIVE)
	    continue;

	  // OK, save this, its axis value and label (runnr or
	  // version, or string for time label).
	  std::string binlabel;
	  if (final[i].version.size())
	    binlabel = final[i].version;
	  else if (final[i].type == SAMPLE_LIVE)
	    binlabel = "Live";
	  else
	  {
	    char tmpbuf[64];
	    sprintf(tmpbuf, "%ld", final[i].runnr);
	    binlabel = tmpbuf;
	  }

	  if (axistime)
	  {
	    binlabel += Time(final[i].time).format(false, ": %b %d %H:%M");
	    axisvals.push_back(final[i].time * 1e-9);
	  }
	  else
	    axisvals.push_back(axisvals.size());

	  binlabels.append(binlabel.c_str(), binlabel.size()+1);
	  attrs.push_back(oneattr);
	}

	// Generate the render request.  We masquerade the summary
	// data as the first object and any live object as another.
	// We use different image option set which the caller cannot
	// use otherwise, so the render server can clearly distinguish
	// the request from other requests and take special action.
	// We abuse 'tag' to let server know the attribute count.
	if (attrs.size())
	{
	  assert(attrs.size() == axisvals.size());

	  // Make sure the 'trend' parameter is defined.
	  if (options.find("trend") == options.end())
	    options["trend"] = "y-mean-rms";

	  // NOTE: This deliberately ignores xobj[1] if it's scalar.
	  // The current protocol does not work for live scalar type.
	  size_t attrsize = attrs.size() * sizeof(VisDQMIndex::Summary);
	  size_t axissize = attrs.size() * sizeof(double);
	  size_t binssize = binlabels.size();
	  xobj[0].tag = attrs.size();
	  xobj[0].flags = DQMNet::DQM_PROP_TYPE_DATABLOB
			  | (axistime ? DQM_PROP_STRIP_AXIS_TIME : 0)
			  | (skipempty ? DQM_PROP_STRIP_SKIP_EMPTY : 0);
	  xobj[0].rawdata.resize(attrsize + axissize + binlabels.size());
	  memcpy(&xobj[0].rawdata[0], &axisvals[0], axissize);
	  memcpy(&xobj[0].rawdata[axissize], &attrs[0], attrsize);
	  memcpy(&xobj[0].rawdata[axissize+attrsize], &binlabels[0], binssize);
	  imageok = link_->render(imagedata, imagetype, path, options,
				  streamers, xobj,
				  (xobj[1].rawdata.size() ? 2 : 1),
				  TRENDIMGOPTS);
	}
	else
	{
	  imageok = link_->render(imagedata, imagetype, path, options,
				  streamers, xobj, 1, STDIMGOPTS);
	}
      }

      // Return the image we produced.
      if (imageok)
	return py::make_tuple(imagetype, imagedata);
      else
	return py::make_tuple(py::object(), py::object());
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/** Make Data Certification. */
class VisDQMCertificationSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMCertificationSource(void)
    : link_(VisDQMRenderLink::instance())
    {}

  virtual const char *
  plotter(void) const
    {
      return "certification";
    }

  py::tuple
  plot(py::object info, const std::string &fullpath, const std::string &path, const std::string &variableName, py::dict opts)
    {
      VisDQMSample cursample = { SAMPLE_ANY, -1 };
      std::map<std::string, std::string> options;
      bool imageok = false;
      std::string imagedata;
      std::string imagetype;
      std::string origdataset;
      copyopts(opts, options);

      // Get the current source.
      if (info.ptr() != Py_None)
      {
	py::tuple t = py::extract<py::tuple>(info);
	py::extract<VisDQMSource *> src(t[0]);
	if (src.check())
	{
	  DQMNet::Object xobj[2];
	  std::string streamers[2];
	  std::string binlabels;
	  std::vector<double> axisvals;
	  std::vector<VisDQMIndex::Summary> attrs;
	  VisDQMIndex::Summary oneattr;
	  // Extract run number and dataset name.  Remember both the
	  // mutilated and original dataset names, former for relval
	  // and the other for other sample matches.  At this stage we
	  // don't know the type of this sample.
	  cursample.runnr = py::extract<long>(t[1]);
	  origdataset = py::extract<std::string>(t[2]);
	  cursample.dataset = StringOps::remove(origdataset, RX_CMSSW_VERSION);
	  // Get the attributes here. An accompanying vector is passed
	  // and filled with actual values of lumisection number: this
	  // is mandatory in case we miss some lumisection.
	  src()->getcert(cursample, path, variableName, attrs, axisvals, binlabels);
	  clearobj(xobj[0]);
	  clearobj(xobj[1]);
	  if (attrs.size())
	  {
	    assert(attrs.size() == axisvals.size());
	    size_t attrsize = attrs.size() * sizeof(VisDQMIndex::Summary);
	    size_t axissize = attrs.size() * sizeof(double);
	    size_t binssize = binlabels.size();
	    xobj[0].tag = attrs.size();
	    xobj[0].flags = DQMNet::DQM_PROP_TYPE_DATABLOB;
	    xobj[0].rawdata.resize(attrsize + axissize + binlabels.size());
	    memcpy(&xobj[0].rawdata[0], &axisvals[0], axissize);
	    memcpy(&xobj[0].rawdata[axissize], &attrs[0], attrsize);
	    memcpy(&xobj[0].rawdata[axissize+attrsize], &binlabels[0], binssize);
	    imageok = link_->render(imagedata, imagetype, fullpath, options,
				    streamers, xobj,
				    1,
				    TRENDIMGOPTS);
	  }
	  else
	  {
	    imageok = link_->render(imagedata, imagetype, path, options,
				    streamers, xobj, 1, STDIMGOPTS);
	  }
	}
      }
      // Return the image we produced.
      if (imageok)
	return py::make_tuple(imagetype, imagedata);
      else
	return py::make_tuple(py::object(), py::object());
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class VisDQMLayoutSource : public VisDQMSource
{
  pthread_mutex_t	lock_;
  VisDQMItems           items_;
public:

  VisDQMLayoutSource()
    {
      pthread_mutex_init(&lock_, 0);
    }

  ~VisDQMLayoutSource(void)
    {}

  virtual void
  scan(VisDQMItems &result,
       const VisDQMSample &sample,
       VisDQMEventNum &current,
       VisDQMRegexp *rxmatch,
       Regexp *rxsearch,
       bool *alarm,
       std::string *layoutroot,
       VisDQMRegexp *rxlayout)
    {
      Lock gate(&lock_);
      VisDQMItems::iterator oi, oe;
      result.resize(result.size() + items_.size());
      for (oi = items_.begin(), oe = items_.end(); oi != oe; ++oi)
      {
	VisDQMItem &o = *oi->second;

	if (layoutroot && rxlayout && rxlayout->rx)
	{
	  RegexpMatch m;
	  if (rxlayout->rx->match(o.name.string(), 0, 0, &m))
	  {
	    std::string root = m.matchString(o.name.string(), 1);
	    if (*layoutroot == root)
	      ;
	    else if (layoutroot->empty())
	      *layoutroot = root;
	    else
	      layoutroot->clear();
	  }
	}

	if (! fastmatch(rxmatch, o.name))
	  continue;

	if (rxsearch && rxsearch->search(o.name.string()) < 0)
	  continue;

	if (alarm && ((o.flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) != 0) != (*alarm == true))
	  continue;

	result[o.name] = oi->second;
      }
    }

  void pushLayouts(py::dict layouts)
    {
      Lock gate(&lock_);
      items_.clear();
      // Now process all the layouts.
      try
      {
	py::stl_input_iterator<std::string> ki(layouts), ke;
	for ( ; ki != ke; ++ki)
	{
	  shared_ptr<VisDQMItem> i(new VisDQMItem);
	  i->flags = 0;
	  i->name = StringAtom(&stree, *ki);
	  i->plotter = 0;
	  buildParentNames(i->parents, i->name);
	  items_[i->name] = i;

	  i->layout.reset(new VisDQMLayoutRows);
	  py::object rows = py::extract<py::object>(layouts.get(*ki));
	  py::stl_input_iterator<py::object> ri(rows), re;
	  for ( ; ri != re; ++ri)
	  {
	    py::stl_input_iterator<py::object> ci(*ri), ce;
	    shared_ptr<VisDQMLayoutRow> row(new VisDQMLayoutRow);
	    for ( ; ci != ce; ++ci)
	    {
	      py::object pycol = *ci;
	      py::extract<std::string> pycolstr(pycol);
	      py::extract<py::dict> pycoldict(pycol);
	      shared_ptr<VisDQMLayoutItem> col(new VisDQMLayoutItem);
	      if (pycol.ptr() == Py_None)
		// None, leave name as empty.
		col->path = StringAtom(&stree, std::string());
	      else if (pycolstr.check())
		// Basic string, use as a name.
		col->path = StringAtom(&stree, pycolstr());
	      else if (pycoldict.check())
	      {
		// Complete description, copy over.
		py::dict colobj = pycoldict();
		col->path = StringAtom(&stree, py::extract<std::string>(colobj.get("path")));
		col->desc = py::extract<std::string>(colobj.get("description", ""));
		if (colobj.has_key("draw"))
		  translateDrawOptions(py::extract<py::dict>(colobj.get("draw")), col->drawopts);
	      }
	      else
		logwarn()
		  << "layout element " << i->name.string()
		  << "[" << i->layout->size() << ", " << row->columns.size()
		  << "] is neither 'None', string nor a dict!\n";

	      row->columns.push_back(col);
	    }
	    i->layout->push_back(row);
	  }
	}
      }
      catch (const py::error_already_set &e)
      {
	logerr() << "failed to update layouts." << std::endl;
	PyErr_Print();
      }
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/** A background thread assisting #VisDQMLiveSource.  Listens to
    changes in live DQM data in upstream sources and maintains a
    up-to-date summary relevant for the web service. */
class VisDQMLiveThread : public DQMBasicNet
{
  VisDQMSource		*owner_;
  VisDQMItems		items_;
  pthread_rwlock_t	itemlock_;
  pthread_cond_t	reqrecv_;
  std::string		streamers_;

public:
  VisDQMLiveThread(VisDQMSource *owner, bool verbose, const std::string &host, int port)
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

  void
  stop(void)
    {
      shutdown();
    }

  virtual bool
  onMessage(Bucket *msg, Peer *p, unsigned char *data, size_t len)
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
	  shared_ptr<VisDQMItem> &i = items_[name];
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
  fetch(const std::string &name, std::string &streamers, Object &retobj)
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
      uint32_t header[3] = { req.size(), DQM_MSG_GET_OBJECT, name.size() };
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

  void
  scan(VisDQMItems &result, VisDQMEventNum &current,
       VisDQMRegexp *rxmatch, Regexp *rxsearch, bool *alarm)
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

  void
  json(const std::string &rootpath,
       bool fulldata,
       bool lumisect,
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

protected:
  // Release an object from wait: wake up fetch() side of this class.
  // Here we need to issue wake up only if the object is null; valid
  // objects are handled in sendObjectToPeer().
  virtual void
  releaseFromWait(Bucket *msg, WaitObject &w, Object *o)
    {
      DQMBasicNet::releaseFromWait(msg, w, o);
      if (! o) pthread_cond_broadcast(&reqrecv_);
    }

  // Handle notification to send an object to downstream client. The
  // only client we can have is actually ourselves: the fetch(). Wake
  // up the receiver side to inform it the object has arrived.
  virtual void
  sendObjectToPeer(Bucket *msg, Object &o, bool data)
    {
      DQMBasicNet::sendObjectToPeer(msg, o, data);
      pthread_cond_broadcast(&reqrecv_);
    }

  // Handle peer removal.  Wake up the receiver side since we may have
  // lost any chance of receiving anything any more.
  virtual void
  removePeer(Peer *p, Socket *s)
    {
      DQMBasicNet::removePeer(p, s);
      pthread_cond_broadcast(&reqrecv_);
    }
};

class VisDQMLiveSource : public VisDQMSource
{
  std::string mydataset_;
  VisDQMLiveThread *thread_;
  VisDQMRenderLink *link_;

public:
  VisDQMLiveSource(py::object gui, py::dict opts)
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

  virtual void
  getdata(const VisDQMSample &sample,
	  const std::string &path,
	  std::string &streamers,
	  DQMNet::Object &obj)
    {
      streamers.clear();
      clearobj(obj);
      thread_->fetch(path, streamers, obj);
    }

  virtual void
  getattr(const VisDQMSample &sample,
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
      VisDQMSample sample = { SAMPLE_ANY, -1 };
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
  StringAtomTree	objnames_; // (1500000);
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
      for (VisDQMFile::ReadHead rdhead(master, 0); ! rdhead.isdone(); rdhead.next())
      {
	void *begin;
	void *end;
	uint64_t key;
	uint64_t hipart;
	uint64_t lopart;

	rdhead.get(&key, &begin, &end);
	hipart = key & 0xffffffff00000000ull;
	lopart = key & 0x00000000ffffffffull;
	if (hipart == VisDQMIndex::MASTER_SAMPLE_RECORD)
	  samples_.push_back(*(const VisDQMIndex::Sample *)begin);
	else
	  break;
      }

      readStrings(dsnames_, master, VisDQMIndex::MASTER_DATASET_NAME);
      readStrings(vnames_, master, VisDQMIndex::MASTER_CMSSW_VERSION);
      readStrings(objnames_, master, VisDQMIndex::MASTER_OBJECT_NAME);
      readStrings(streamers_, master, VisDQMIndex::MASTER_TSTREAMERINFO);
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
  VisDQMArchiveSource(py::object gui, py::dict opts)
    : rxonline_(py::extract<std::string>(opts["rxonline"])),
      retry_(false),
      path_(Filename(py::extract<std::string>(opts["index"]))),
      watch_(path_),
      cache_(300*1024*1024),
      index_(path_, &cache_),
      vnames_(10000),
      dsnames_(100000),
      objnames_(1500000),
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

  void
  exit(void)
    {
      if (watch_.running())
	watch_.stop();
    }

  static inline void makeBinLabel(char * buff, uint64_t value)
    {
      sprintf(buff, "%ld", value);
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
	    uint64_t keyidx = (uint64_t(si - samples_.begin()) << 44) | objindex;
	    uint64_t key;
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
	  std::string &xstreamers,
	  DQMNet::Object &xobj)
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
	    uint64_t keyidx = (uint64_t(si - samples_.begin()) << 44) | objindex;
	    uint64_t key;
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

  py::tuple
  plot(int runnr,
       const std::string &dataset,
       const std::string &path,
       py::dict opts)
    {
      VisDQMSample sample = { SAMPLE_ANY, runnr, dataset };
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
	  uint64_t keyidx = (si - samples_.begin());
	  for (VisDQMFile::ReadHead rdhead(file.get(), keyidx << 44);
	       ! rdhead.isdone(); rdhead.next())
	  {
	    uint64_t key;
	    void *begin;
	    void *end;
	    rdhead.get(&key, &begin, &end);
	    // FIXME: handle keys other than run summary?
	    uint64_t keyparts[4] = { (key >> 44) & 0xfffff, (key >> 40) & 0xf,
				     (key >> 20) & 0xfffff, key & 0xfffff };
	    if (keyparts[0] != keyidx || keyparts[1] != 0)
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
	std::string dataset;
	VisDQMSampleType type = SAMPLE_ANY;

	{
	  PyReleaseInterpreterLock nogil;

	  // Make sure we are up to date.
	  VisDQMItems items;
	  VisDQMSample sample = { SAMPLE_ONLINE_DATA, 0 };
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
	  uint64_t keyidx = (si - samples_.begin());
	  VisDQMFile::ReadHead rdinfo(info.get(), keyidx << 44);
	  for ( ; ! rdinfo.isdone(); rdinfo.next())
	  {
	    uint64_t ikey;
	    uint64_t dkey;
	    void *begin;
	    void *end;
	    rdinfo.get(&ikey, &begin, &end);
	    uint64_t keyparts[4] = { (ikey >> 44) & 0xfffff, (ikey >> 40) & 0xf,
				     (ikey >> 20) & 0xfffff, ikey & 0xfffff };
	    if (keyparts[0] != keyidx || (!lumisect && (keyparts[1] != 0)))
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
	  uint64_t keyidx = (si - samples_.begin());
	  VisDQMFile::ReadHead rdinfo(info.get(), keyidx << 44);
	  for ( ; ! rdinfo.isdone(); rdinfo.next())
	  {
	    uint64_t ikey;
	    uint64_t dkey;
	    void *begin;
	    void *end;
	    rdinfo.get(&ikey, &begin, &end);
	    uint64_t keyparts[4] = { (ikey >> 44) & 0xfffff, (ikey >> 40) & 0xf,
				     (ikey >> 20) & 0xfffff, ikey & 0xfffff };
	    if (keyparts[0] != keyidx)
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
  state(py::dict session)
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
      bool show;
      int  x;
      int  y;
      int  w;
      int  h;

      show = py::extract<bool>(session.get("dqm.zoom.show", false));
      x = py::extract<int>(session.get("dqm.zoom.x", -1));
      y = py::extract<int>(session.get("dqm.zoom.y", -1));
      w = py::extract<int>(session.get("dqm.zoom.w", -1));
      h = py::extract<int>(session.get("dqm.zoom.h", -1));

      return StringFormat("'zoom':{'show':%1,'x':%2,'y':%3,'w':%4,'h':%5}")
	.arg(show).arg(x).arg(y).arg(w).arg(h);
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
      return StringFormat("{'position':'%1', 'show':'%2', 'param':[%3,%4,%5,%6]}")
        .arg(py::extract<std::string>(refdict.get("position"))) // overlay, on-side
	.arg(py::extract<std::string>(refdict.get("show")))     // all, none, custom
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
	       const std::string &reference,
	       const std::string &strip,
	       const std::string &rxstr,
	       const std::string &rxerr,
	       size_t rxmatches,
	       const std::string &toolspanel,
	       Time startTime)
    {
      StringFormat result
	= StringFormat("([{'kind':'AutoUpdate', 'interval':%1, 'stamp':%2, 'serverTime':%19},"
		       "{'kind':'DQMHeaderRow', 'run':\"%3\", 'lumi':\"%4\", 'event':\"%5\","
		       " 'runstart':\"%6\", 'service':'%7', 'services':[%8], 'workspace':'%9',"
		       " 'workspaces':[%10], 'view':{'show':'%11','sample': %12, 'filter':'%13',"
		       " 'reference':%14, 'strip':%15, 'search':%16, %17}},%18])")
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

      VisDQMStatus leaf;
      leaf.nleaves = 1;
      leaf.nalarm = 0;
      leaf.nlive = 0;

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
			    submenu, sample, filter, reference, strip,
			    rxstr, rxerr, summary.size(), toolspanel,
			    startTime);
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
			    submenu, sample, filter, reference, strip,
			    rxstr, rxerr, cert.size(), toolspanel,
			    startTime);
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
			    submenu, sample, filter, reference, strip,
			    rxstr, rxerr, qmap.size(), toolspanel,
			    startTime);
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
			  " 'drawopts':%10, 'withref':%11}")
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
	.arg(stringToJSON(x.drawopts.withref));
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
  shownDirToJSON(const VisDQMItems &contents,
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
			 const std::string &match)
    : VisDQMWorkspace(gui, name)
    {
      if (! match.empty() && match != "^")
      {
        fastrx(rxmatch_, match);
	fastrx(rxlayout_, "(" + match + "Layouts)/.*");
      }
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
			    		 " %6, %7, 'reference':%8, 'strip':%9,"
					 " 'layoutroot':%11}")
			    .arg(shownToJSON(contents, status, drawopts,
					     StringAtom(&stree, root),
					     sample, shown))
			    .arg(stringToJSON(root))
			    .arg(stringToJSON(focus, true))
			    .arg(size)
			    .arg(helppanel)
			    .arg(custompanel)
			    .arg(zoom)
			    .arg(reference)
			    .arg(strip)
			    .arg(stringToJSON(layoutroot)),
			    sample.type == SAMPLE_LIVE ? 30 : 300, current,
			    services, workspaceName_, workspaces,
			    submenu, sample, filter, reference, strip,
			    rxstr, rxerr, contents.size(), toolspanel,
			    startTime);
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
					 " 'reference':%5, 'strip':%6}")
			    .arg(shownToJSON(contents, status, drawopts,
					     StringAtom(&stree, root),
					     sample, shown))
			    .arg(playinterval)
			    .arg(playpos)
			    .arg(shown.size())
			    .arg(reference)
			    .arg(strip),
			    300, current, services, workspace, workspaces,
			    submenu, sample, filter, reference, strip,
			    rxstr, rxerr, contents.size(), toolspanel,
			    startTime);
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
    .def("_plot", &VisDQMUnknownSource::plot);

  py::class_<VisDQMOverlaySource, shared_ptr<VisDQMOverlaySource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMOverlaySource", py::init<>())
    .add_property("plothook", &VisDQMOverlaySource::plotter)
    .def("_plot", &VisDQMOverlaySource::plot);

  py::class_<VisDQMStripChartSource, shared_ptr<VisDQMStripChartSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMStripChartSource", py::init<>())
    .add_property("plothook", &VisDQMStripChartSource::plotter)
    .def("_plot", &VisDQMStripChartSource::plot);

  py::class_<VisDQMCertificationSource, shared_ptr<VisDQMCertificationSource>,
             py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMCertificationSource", py::init<>())
    .add_property("plothook", &VisDQMCertificationSource::plotter)
    .def("_plot", &VisDQMCertificationSource::plot);

  py::class_<VisDQMLiveSource, shared_ptr<VisDQMLiveSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMLiveSource", py::init<py::object, py::dict>())
    .add_property("plothook", &VisDQMLiveSource::plotter)
    .def("_plot", &VisDQMLiveSource::plot)
    .def("_exit", &VisDQMLiveSource::exit);

  py::class_<VisDQMArchiveSource, shared_ptr<VisDQMArchiveSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMArchiveSource", py::init<py::object, py::dict>())
    .add_property("plothook", &VisDQMArchiveSource::plotter)
    .def("_plot", &VisDQMArchiveSource::plot)
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
    ("DQMContentWorkspace", py::init<py::object, std::string, std::string>())
    .def("_state", &VisDQMContentWorkspace::state);

  py::class_<VisDQMPlayWorkspace, shared_ptr<VisDQMPlayWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMPlayWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMPlayWorkspace::state);

  EXC_TRANSLATE(StdError, std::exception, what());
  EXC_TRANSLATE(LATError, lat::Error, explain().c_str());
}
