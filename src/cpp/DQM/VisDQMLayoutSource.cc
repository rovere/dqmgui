#include "DQM/VisDQMLayoutSource.h"

#include "classlib/utils/RegexpMatch.h"

#include "boost/shared_ptr.hpp"

#include <string>
#include <vector>
#include <map>

#include "DQM/VisDQMSource.h"
#include "DQM/DQMNet.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMLocks.h"
#include "DQM/VisDQMRenderLink.h"
#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMSourceObjects.h"
#include "DQM/StringAtom.h"

extern void buildParentNames(StringAtomList & /* to */, const StringAtom & /* from */);
extern StringAtomTree stree;


namespace py = boost::python;

void
VisDQMLayoutSource::scan(VisDQMItems &result,
                         const VisDQMSample & /* sample */,
                         VisDQMEventNum & /* current */,
                         VisDQMRegexp * /* rxmatch */,
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
      else
        continue;
    }

    if (rxsearch && rxsearch->search(o.name.string()) < 0)
      continue;

    if (alarm && ((o.flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) != 0) != (*alarm == true))
      continue;

    result[o.name] = oi->second;
  }
}

void VisDQMLayoutSource::pushLayouts(py::dict layouts)
{
  Lock gate(&lock_);
  items_.clear();
  // Now process all the layouts.
  try
  {
    py::stl_input_iterator<std::string> ki(layouts), ke;
    for ( ; ki != ke; ++ki)
    {
      boost::shared_ptr<VisDQMItem> i(new VisDQMItem);
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
            if (colobj.has_key("overlays"))
            {
              py::list overlays = py::extract<py::list>(colobj.get("overlays"));
              py::ssize_t n = py::len(overlays);
              for(py::ssize_t j = 0; j < n; j++)
              {
                py::extract<std::string> overlay_name(overlays[j]);
                if (overlay_name.check())
                  col->overlays.insert(StringAtom(&stree, overlay_name()));
              }
            }
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
