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
#include "DQM/VisDQMRegexp.h"
#include "DQM/VisDQMDrawOptions.h"
#include "DQM/VisDQMEventNum.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMLocks.h"

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

#include <set>
#include <cfloat>
#include <fstream>
#include <stdexcept>
#include <dlfcn.h>
#include <math.h>
#include <inttypes.h>

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



using namespace lat;
using namespace boost;
namespace ext = __gnu_cxx;
namespace py = boost::python;
typedef std::map<StringAtom, StringAtomList> StringAtomParentList;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma GCC visibility push(hidden)
pthread_rwlock_t parentslock;
StringAtomParentList parents;
StringAtomTree stree(1024*1024);

void
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
#pragma GCC visibility pop

#include "DQM/VisDQMToJSON.h"
#include "DQM/VisDQMRenderLink.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#include "DQM/VisDQMUnknownSource.h"
#include "DQM/VisDQMOverlaySource.h"
#include "DQM/VisDQMStripChartSource.h"
#include "DQM/VisDQMCertificationSource.h"
#include "DQM/VisDQMLayoutSource.h"
#include "DQM/VisDQMLiveSource.h"
#include "DQM/VisDQMArchiveWatch.h"
#include "DQM/VisDQMArchiveSource.h"
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include "DQM/VisDQMWorkspace.h"
#include "DQM/VisDQMSummaryWorkspace.h"

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
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
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
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, cert.size(),
                            toolspanel, startTime);
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
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
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
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, qmap.size(),
                            toolspanel, startTime);
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
			  " 'drawopts':%10, 'withref':%11, 'overlays':%12}")
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
          .arg(stringToJSON(x.drawopts.withref))
          .arg(stringsToJSON(x.overlays));
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
  shownDirToJSON(const VisDQMItems & /* contents */,
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
              scol->overlays = col.overlays;
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
			 const std::string &match,
			 const std::string &layouts)
    : VisDQMWorkspace(gui, name)
    {
      if (! match.empty() && match != "^")
      {
        fastrx(rxmatch_, match);
        if (layouts.empty())
          fastrx(rxlayout_, "(" + match + "Layouts)/.*");
      }
      if (! layouts.empty() && layouts !="^")
        fastrx(rxlayout_, "(" + layouts + ")/.*");

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
      std::string   showstats    (py::extract<std::string>(session.get("dqm.showstats")));
      std::string   showerrbars  (py::extract<std::string>(session.get("dqm.showerrbars")));
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
                                         " %6, 'showstats': %7, 'showerrbars': %8, %9,"
                                         " 'reference':%10, 'strip':%11,"
                                         " 'layoutroot':%12}")
                            .arg(shownToJSON(contents, status, drawopts,
                                             StringAtom(&stree, root),
                                             sample, shown))
                            .arg(stringToJSON(root))
                            .arg(stringToJSON(focus, true))
                            .arg(size)
                            .arg(helppanel)
                            .arg(custompanel)
                            .arg(showstats)
                            .arg(showerrbars)
                            .arg(zoom)
                            .arg(reference)
                            .arg(strip)
                            .arg(stringToJSON(layoutroot)),
                            sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, contents.size(),
                            toolspanel, startTime);
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
      std::string   showstats    (py::extract<std::string>(session.get("dqm.showstats")));
      std::string   showerrbars  (py::extract<std::string>(session.get("dqm.showerrbars")));
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
					 " 'reference':%5, 'strip':%6, 'showstats': %7,"
                                         " 'showerrbars': %8}")
			    .arg(shownToJSON(contents, status, drawopts,
					     StringAtom(&stree, root),
					     sample, shown))
			    .arg(playinterval)
			    .arg(playpos)
			    .arg(shown.size())
			    .arg(reference)
			    .arg(strip)
                            .arg(showstats)
                            .arg(showerrbars),
			    300, current, services, workspace, workspaces,
			    submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, contents.size(),
                            toolspanel, startTime);
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
    .add_property("jsonhook", &VisDQMUnknownSource::jsoner)
    .def("_plot", &VisDQMUnknownSource::plot);

  py::class_<VisDQMOverlaySource, shared_ptr<VisDQMOverlaySource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMOverlaySource", py::init<>())
    .add_property("plothook", &VisDQMOverlaySource::plotter)
    .add_property("jsonhook", &VisDQMOverlaySource::jsoner)
    .def("_plot", &VisDQMOverlaySource::plot);

  py::class_<VisDQMStripChartSource, shared_ptr<VisDQMStripChartSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMStripChartSource", py::init<>())
    .add_property("plothook", &VisDQMStripChartSource::plotter)
    .add_property("jsonhook", &VisDQMStripChartSource::jsoner)
    .def("_plot", &VisDQMStripChartSource::plot);

  py::class_<VisDQMCertificationSource, shared_ptr<VisDQMCertificationSource>,
             py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMCertificationSource", py::init<>())
    .add_property("plothook", &VisDQMCertificationSource::plotter)
    .add_property("jsonhook", &VisDQMCertificationSource::jsoner)
    .def("_plot", &VisDQMCertificationSource::plot);

  py::class_<VisDQMLiveSource, shared_ptr<VisDQMLiveSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMLiveSource", py::init<py::object, py::dict>())
    .add_property("plothook", &VisDQMLiveSource::plotter)
    .add_property("jsonhook", &VisDQMLiveSource::jsoner)
    .def("_plot", &VisDQMLiveSource::plot)
    .def("_exit", &VisDQMLiveSource::exit);

  py::class_<VisDQMArchiveSource, shared_ptr<VisDQMArchiveSource>,
    	     py::bases<VisDQMSource>, boost::noncopyable>
    ("DQMArchiveSource", py::init<py::object, py::dict>())
    .add_property("plothook", &VisDQMArchiveSource::plotter)
    .add_property("jsonhook", &VisDQMArchiveSource::jsoner)
    .def("_plot", &VisDQMArchiveSource::plot)
    .def("_getJson", &VisDQMArchiveSource::getJson)
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
    ("DQMContentWorkspace", py::init<py::object, std::string, std::string, std::string>())
    .def("_state", &VisDQMContentWorkspace::state);

  py::class_<VisDQMPlayWorkspace, shared_ptr<VisDQMPlayWorkspace>,
	     py::bases<VisDQMWorkspace>, boost::noncopyable>
    ("DQMPlayWorkspace", py::init<py::object, std::string>())
    .def("_state", &VisDQMPlayWorkspace::state);

  EXC_TRANSLATE(StdError, std::exception, what());
  EXC_TRANSLATE(LATError, lat::Error, explain().c_str());
}
