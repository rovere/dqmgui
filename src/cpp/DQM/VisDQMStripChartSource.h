#ifndef DQM_VISDQMSTRIPCHARTSOURCE_H
#define DQM_VISDQMSTRIPCHARTSOURCE_H

#include "DQM/VisDQMSource.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

class VisDQMRenderLink;

namespace py = boost::python;

/** Make strip charts of ROOT objects. */
class VisDQMStripChartSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMStripChartSource(void);

  virtual const char *
  plotter(void) const
    {
      return "stripchart";
    }

  virtual const char *
  jsoner(void) const
    {
      return "underconstruction";
    }

  py::tuple plot(py::list pysources, py::object info,
                 const std::string &path, py::dict opts);
};

#endif // DQM_VISDQMSTRIPCHARTSOURCE_H
