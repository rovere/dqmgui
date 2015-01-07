#include "DQM/VisDQMCertificationWorkspace.h"
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

std::string VisDQMCertificationWorkspace::state(py::dict session)
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

    VisDQMItems         contents;
    StringAtomSet               myobjs; // deliberately empty, not needed
    VisDQMItems::iterator       ci, ce;
    VisDQMStatusMap             status;
    RegexpMatch         m;
    std::string         plotter;
    KeyVectorMap                cert;
    VisDQMEventNum              current = { "", -1, -1, -1, -1 };

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

std::string VisDQMCertificationWorkspace::certificationToJSON(const KeyVectorMap &cert)
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
