#include "DQM/VisDQMContentWorkspace.h"
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

// Helper function to present session state.  Returns a JSON object
// representing current menu and canvas state.
std::string VisDQMContentWorkspace::state(py::dict session)
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
  std::string   strip        (sessionStripChart(session));
  std::string   submenu      (py::extract<std::string>(session.get("dqm.submenu")));
  std::string   size         (py::extract<std::string>(session.get("dqm.size")));
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

    VisDQMItems             contents;
    StringAtomSet           shown;
    VisDQMItems::iterator   ci, ce;
    VisDQMStatusMap         status;
    std::string             data;
    std::string             layoutroot;
    VisDQMEventNum          current = { "", -1, -1, -1, -1 };
    bool                    alarms = false;
    bool                    *qalarm = 0;

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
