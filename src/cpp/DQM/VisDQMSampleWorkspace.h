#ifndef DQM_VISDQMSAMPLEWORKSPACE_H
#define DQM_VISDQMSAMPLEWORKSPACE_H

#include "DQM/VisDQMWorkspace.h"

#include "boost/python.hpp"

#include <string>

class VisDQMSampleWorkspace : public VisDQMWorkspace
{
public:
  VisDQMSampleWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMSampleWorkspace(void)
    {}

  virtual std::string state(py::dict session);
private:
};

#endif  // DQM_VISDQMSAMPLEWORKSPACE_H
