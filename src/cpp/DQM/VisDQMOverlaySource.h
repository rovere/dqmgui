#ifndef DQM_VISDQMOVERLAYSOURCE_H
#define DQM_VISDQMOVERLAYSOURCE_H

#include "DQM/VisDQMSource.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

class VisDQMRenderLink;

namespace py = boost::python;

/** Overlay several ROOT objects on top of each other. */
class VisDQMOverlaySource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMOverlaySource(void);

  virtual const char *
  plotter(void) const
    {
      return "overlay";
    }

  virtual const char *
  jsoner(void) const
    {
      return "underconstruction";
    }

  py::tuple plot(py::list overlay, py::dict opts);
};

#endif // DQM_VISDQMOVERLAYSOURCE_H
