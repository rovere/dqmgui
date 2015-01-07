#ifndef DQM_VISDQMPLAYWORKSPACE_H
#define DQM_VISDQMPLAYWORKSPACE_H

#include "DQM/VisDQMWorkspace.h"
#include "DQM/VisDQMLayoutTools.h"

#include "classlib/utils/Regexp.h"

#include "boost/python.hpp"

#include <string>

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
  virtual std::string state(py::dict session);
};

#endif  // DQM_VISDQMPLAYWORKSPACE_H
