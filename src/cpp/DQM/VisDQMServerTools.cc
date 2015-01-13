/** This file contains utility functions that are exclusively used by
    the serverext packages within the Accelerator shared library used
    to link the C++ back-end to the python frontend. These have not
    been included in the other utility file VisDQMTools.h since they
    do not pertaing neither to rendering, nor to indexing and would
    have therefore increased code side w/o any reason.  */
#include "DQM/VisDQMServerTools.h"

#define DEBUG(n,x)

#include <iostream>
#include <string>
#include <map>
#include <set>

#include "DQM/VisDQMIndex.h"
// We do not want to import ROOT stuff in here.
# define VISDQM_NO_ROOT 1
#include "DQM/VisDQMTools.h"
#include "DQM/VisDQMHexlify.h"
#include "DQM/StringAtom.h"

#include "classlib/utils/TimeInfo.h"
#include "classlib/utils/StringOps.h"
#include "classlib/utils/StringFormat.h"

extern StringAtomTree stree;
  // WARNING: we include hexlify in the list of externals since we rely
  // on the fact that it will be instantiated with this very same
  // signature inside serverext.cc, hence avoiding a useless
  // compilation.


using namespace lat;
using namespace boost;

// All the following functions are for internal usage within the
// serverext.cc file and hence we can hide them completely from being
// exposed via the Accelerator.so shared library, reducing its space
// and loading time.
#pragma GCC visibility push(hidden)

// Custom function to log generic events prepending a properly
// formatted time information.
std::ostream &
lograw(const char *label)
{
  Time now = Time::current();
  return std::cout
      << now.format(true, "[%d/%b/%Y:%H:%M:%S.")
      << now.nanoformat(3, 3) << ']'
      << label;
}

std::ostream &
loginfo(void)
{
  return lograw("  INFO: ");
}

std::ostream &
logerr(void)
{
  return lograw("  ERROR: ");
}

std::ostream &
logwarn(void)
{
  return lograw("  WARNING: ");
}

std::string
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

void
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

/// Check whether the @a path is a subdirectory of @a ofdir.  Returns
/// true both for an exact match and any nested subdirectory.
bool
isSubdirectory(const std::string &ofdir, const std::string &path)
{
  return (ofdir.empty()
	  || (path.size() >= ofdir.size()
	      && path.compare(0, ofdir.size(), ofdir) == 0
	      && (path.size() == ofdir.size()
		  || path[ofdir.size()] == '/')));
}

void
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

std::string
stringToJSON(const std::string &x, bool emptyIsNone /* = false */)
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

std::string
stringsToJSON(const StringAtomSet &overlays, bool /* emptyIsNone  = false */)
{
  StringAtomSet::const_iterator mi = overlays.begin();
  StringAtomSet::const_iterator me = overlays.end();
  std::string result = "[";

  for (; mi != me; ++mi) {
    if (result.length() > 1 )
      result += ", ";
    result += stringToJSON(mi->string());
  }
  result += "]";
  return result;
}


shared_ptr<Regexp>
rx(const std::string &match, int options /* = 0 */)
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

void
makerx(const std::string &rxstr,
       shared_ptr<Regexp> &rxobj,
       std::string &rxerr,
       int options /* = 0 */)
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

void
fastrx(VisDQMRegexp &rxobj, const std::string &rxstr)
{
  rxobj.rx = rx(rxstr);
  rxobj.tried.resize(stree.capacity(), false);
  rxobj.matched.resize(stree.capacity(), false);
  rxobj.locks.resize(stree.capacity() / VisDQMRegexp::REGEXPS_PER_LOCK);
  for (size_t i = 0, e = rxobj.locks.size(); i < e; ++i)
    pthread_rwlock_init(&rxobj.locks[i], 0);
}

bool
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

uint32_t
getType(uint32_t flags)
{
  return flags & VisDQMIndex::SUMMARY_PROP_TYPE_MASK;
}

bool
isROOTType(uint32_t flags)
{
  uint32_t type = getType(flags);
  return (type >= VisDQMIndex::SUMMARY_PROP_TYPE_TH1F
	  && type <= VisDQMIndex::SUMMARY_PROP_TYPE_TPROF2D);
}

bool
isScalarType(uint32_t flags)
{
  uint32_t type = getType(flags);
  return (type >= VisDQMIndex::SUMMARY_PROP_TYPE_INT
	  && type <= VisDQMIndex::SUMMARY_PROP_TYPE_STRING);
}

bool
isBLOBType(uint32_t flags)
{
  return getType(flags) == DQMNet::DQM_PROP_TYPE_DATABLOB;
}

bool
isIntegerType(uint32_t flags)
{
  return getType(flags) == VisDQMIndex::SUMMARY_PROP_TYPE_INT;
}

bool
isRealType(uint32_t flags)
{
  return getType(flags) == VisDQMIndex::SUMMARY_PROP_TYPE_REAL;
}

void
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

std::string
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

void
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
void
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

std::string
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
void
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
			 " \"hasref\": %9, \"isEff\": %10, \"tagged\": %11 },\"tag\": %12,"
			 " \"qresults\": [%13], \"nentries\": %14,"
			 " \"stats\": { \"x\": %15, \"y\": %16, \"z\": %17 },"
			 " \"%18\": \"%19\"}\n")
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
      .arg((flags & DQMNet::DQM_PROP_EFFICIENCY_PLOT) ? 1 : 0)
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

void
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
  result += "\", \"importversion\":";
  result += StringFormat("%1").arg(sample.importversion);
  result += "}";
}

std::string
sampleToJSON(const VisDQMSample &sample)
{
  std::string result;
  result.reserve(64 + sample.dataset.size());
  sampleToJSON(sample, result);
  return result;
}

std::string
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

VisDQMSample
sessionSample(const py::dict &session)
{
  VisDQMSample result;
  long type = py::extract<long>(session.get("dqm.sample.type"));
  result.runnr = py::extract<long>(session.get("dqm.sample.runnr"));
  result.dataset = py::extract<std::string>(session.get("dqm.sample.dataset"));
  result.importversion = py::extract<int>(session.get("dqm.sample.importversion"));
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

#pragma GCC visibility pop
