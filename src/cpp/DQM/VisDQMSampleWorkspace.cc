#include "DQM/VisDQMSampleWorkspace.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMLocks.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMSampleTypes.h"

#include "classlib/utils/Time.h"
#include "classlib/utils/RegexpMatch.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/StringOps.h"

#include "boost/shared_ptr.hpp"

#include <vector>

extern StringAtomTree stree;

std::string VisDQMSampleWorkspace::state(py::dict session)
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
    VisDQMSamples           samples;
    VisDQMSamples           final;
    StringList              pats;
    StringList              runlist;
    RXList                  rxlist;
    std::string             rxerr;
    MatchSet                uqpats;
    bool                    varyAny = (vary == "any");
    bool                    varyRun = (vary == "run");
    bool                    varyDataset = (vary == "dataset");
    std::string             samplebase;

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
