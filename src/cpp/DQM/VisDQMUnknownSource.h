#ifndef DQM_VISDQMUNKNOWNSOURCE_H
#define DQM_VISDQMUNKNOWNSOURCE_H

#include <map>
#include <string>

#include "DQM/VisDQMSource.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

class VisDQMRenderLink;

namespace py = boost::python;

class VisDQMUnknownSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMUnknownSource(void);
  py::tuple plot(const std::string &path, py::dict opts);
};

#endif // DQM_VISDQMUNKNOWNSOURCE_H
