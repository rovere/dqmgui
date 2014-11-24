#ifndef DQM_VISDQMUNKNOWNSOURCE_H
#define DQM_VISDQMUNKNOWNSOURCE_H

#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

#include "DQM/DQMNet.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMRenderLink.h"

namespace py = boost::python;

class VisDQMUnknownSource : public VisDQMSource {
  VisDQMRenderLink *link_;

 public:
  VisDQMUnknownSource(void)
      : link_(VisDQMRenderLink::instance())
  {}
  py::tuple plot(const std::string &path, py::dict opts);
};

#endif // DQM_VISDQMUNKNOWNSOURCE_H
