#ifndef DQM_VISDQMCONTENTWORKSPACE_H
#define DQM_VISDQMCONTENTWORKSPACE_H

#include "DQM/VisDQMWorkspace.h"
#include "DQM/VisDQMLayoutTools.h"
#include "DQM/VisDQMRegexp.h"

#include "classlib/utils/Regexp.h"

#include "boost/python.hpp"

#include <map>
#include <string>

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
  virtual std::string state(py::dict session);
};

#endif  // DQM_VISDQMCONTENTWORKSPACE_H
