#include "DQM/VisDQMSummaryWorkspace.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMLocks.h"
#include "DQM/VisDQMServerTools.h"

#include "classlib/utils/Time.h"
#include "classlib/utils/RegexpMatch.h"
#include "classlib/utils/StringFormat.h"

#include "boost/shared_ptr.hpp"

#include <vector>

extern StringAtomTree stree;

std::string VisDQMSummaryWorkspace::state(py::dict session)
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

    VisDQMItems         contents;
    StringAtomSet       myobjs; // deliberately empty, not needed
    VisDQMItems::iterator ci, ce;
    VisDQMStatusMap     status;
    RegexpMatch         m;
    std::string         plotter;
    SummaryMap          summary;
    VisDQMEventNum      current = { "", -1, -1, -1, -1 };
    bool                alarms = false;
    bool                *qalarm = 0;

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
                        submenu, sample, filter, showstats, showerrbars,
                        reference, strip, rxstr, rxerr, summary.size(),
                        toolspanel, startTime);
  }
}

std::string
VisDQMSummaryWorkspace::summaryToJSON(const VisDQMSummaryWorkspace::SummaryMap &summary)
{
  size_t len = 4 * summary.size();
  VisDQMSummaryWorkspace::SummaryMap::const_iterator si, se;
  VisDQMSummaryWorkspace::KeyValueMap::const_iterator ki, ke;
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
