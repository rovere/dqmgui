#include "DQM/VisDQMWorkspace.h"
#include "DQM/VisDQMLocks.h"

#include <stdio.h>

#include <set>

#include "classlib/utils/TimeInfo.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/StringOps.h"

extern StringAtomTree stree;
extern void buildParentNames(StringAtomList & /* to */, const StringAtom & /* from */);

// Helper function to compute next available run in current dataset
int VisDQMWorkspace::changeRun(py::dict session, bool forward)
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

// Produce panel configuration format.
std::string
VisDQMWorkspace::sessionPanelConfig(const py::dict &session, const char *panel)
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
std::string
VisDQMWorkspace::sessionZoomConfig(const py::dict &session)
{
  bool show, jsonmode;
  int  x, jx;
  int  y, jy;
  int  w, jw;
  int  h, jh;

  show = py::extract<bool>(session.get("dqm.zoom.show", false));
  x = py::extract<int>(session.get("dqm.zoom.x", -1));
  y = py::extract<int>(session.get("dqm.zoom.y", -1));
  w = py::extract<int>(session.get("dqm.zoom.w", -1));
  h = py::extract<int>(session.get("dqm.zoom.h", -1));
  jsonmode = py::extract<bool>(session.get("dqm.zoom.jsonmode", false));
  jx = py::extract<int>(session.get("dqm.zoom.jx", -1));
  jy = py::extract<int>(session.get("dqm.zoom.jy", -1));
  jw = py::extract<int>(session.get("dqm.zoom.jw", -1));
  jh = py::extract<int>(session.get("dqm.zoom.jh", -1));

  return StringFormat("'zoom':{'show':%1,'x':%2,'y':%3,'w':%4,'h':%5,\
                           'jsonmode':%6,'jx':%7,'jy':%8,'jw':%9,'jh':%10}")
      .arg(show).arg(x).arg(y).arg(w).arg(h)
      .arg(jsonmode).arg(jx).arg(jy).arg(jw).arg(jh);
}

// Produce Certification zoom configuration format.
std::string
VisDQMWorkspace::sessionCertZoomConfig(const py::dict &session)
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

std::string
VisDQMWorkspace::sessionReferenceOne(const py::dict &ref)
{
  return StringFormat("{'type':'%1','run':%2,'dataset':%3, 'ktest':%4}")
      .arg(py::extract<std::string>(ref.get("type"))) // refobj, other, none
      .arg(stringToJSON(py::extract<std::string>(ref.get("run"))))
      .arg(stringToJSON(py::extract<std::string>(ref.get("dataset"))))
      .arg(stringToJSON(py::extract<std::string>(ref.get("ktest"))));
}

// Produce a reference description.
std::string
VisDQMWorkspace::sessionReference(const py::dict &session)
{
  py::dict refdict = py::extract<py::dict>(session.get("dqm.reference"));
  py::list refspec = py::extract<py::list>(refdict.get("param"));
  std::string ref1(sessionReferenceOne(py::extract<py::dict>(refspec[0])));
  std::string ref2(sessionReferenceOne(py::extract<py::dict>(refspec[1])));
  std::string ref3(sessionReferenceOne(py::extract<py::dict>(refspec[2])));
  std::string ref4(sessionReferenceOne(py::extract<py::dict>(refspec[3])));
  return StringFormat("{'position':'%1', 'show':'%2', \
                            'param':[%3,%4,%5,%6]}")
      .arg(py::extract<std::string>(refdict.get("position")))    // overlay, on-side
      .arg(py::extract<std::string>(refdict.get("show")))        // all, none, custom
      .arg(ref1).arg(ref2).arg(ref3).arg(ref4);
}

std::string
VisDQMWorkspace::sessionStripChart(const py::dict &session)
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
VisDQMWorkspace::makeResponse(const std::string &state,
                              int interval,
                              VisDQMEventNum &current,
                              const std::string &services,
                              const std::string &workspace,
                              const std::string &workspaces,
                              const std::string &submenu,
                              const VisDQMSample &sample,
                              const std::string &filter,
                              const std::string &showstats,
                              const std::string &showerrbars,
                              const std::string &reference,
                              const std::string &strip,
                              const std::string &rxstr,
                              const std::string &rxerr,
                              size_t rxmatches,
                              const std::string &toolspanel,
                              Time startTime)
{
  StringFormat result
      = StringFormat("([{'kind':'AutoUpdate', 'interval':%1, 'stamp':%2, 'serverTime':%21},"
                     "{'kind':'DQMHeaderRow', 'run':\"%3\", 'lumi':\"%4\", 'event':\"%5\","
                     " 'runstart':\"%6\", 'service':'%7', 'services':[%8], 'workspace':'%9',"
                     " 'workspaces':[%10], 'view':{'show':'%11','sample': %12, 'filter':'%13',"
                     " 'showstats': %14, 'showerrbars': %15, 'reference':%16, 'strip':%17,"
                     " 'search':%18, %19}},%20])")
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
      .arg(showstats)
      .arg(showerrbars)
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
VisDQMWorkspace::sources(std::vector<VisDQMSource *> &into)
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
VisDQMWorkspace::serviceListJSON(void)
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
VisDQMWorkspace::workspaceListJSON(void)
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
VisDQMWorkspace::buildContents(const std::vector<VisDQMSource *> &sources,
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
VisDQMWorkspace::buildShown(const VisDQMItems &contents,
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
VisDQMWorkspace::updateStatus(const VisDQMItems &contents,
                              VisDQMStatusMap &status)
{
  VisDQMStatus empty;
  empty.nleaves = 0;
  empty.nalarm = 0;
  empty.nlive = 0;
  
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
