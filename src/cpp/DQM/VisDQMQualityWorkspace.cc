#include "DQM/VisDQMQualityWorkspace.h"
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

std::string VisDQMQualityWorkspace::state(py::dict session)
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

std::string VisDQMQualityWorkspace::qmapToJSON(const QMap &qmap)
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

