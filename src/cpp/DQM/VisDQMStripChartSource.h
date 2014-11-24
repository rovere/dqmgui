#ifndef DQM_VISDQMSTRIPCHARTSOURCE_H
#define DQM_VISDQMSTRIPCHARTSOURCE_H

#include <string>

#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMRenderLink.h"

/** Make strip charts of ROOT objects. */
class VisDQMStripChartSource : public VisDQMSource {
  VisDQMRenderLink *link_;

 public:
  VisDQMStripChartSource(void)
      : link_(VisDQMRenderLink::instance())
  {}

  virtual const char *
  plotter(void) const {
    return "stripchart";
  }

  virtual const char *
  jsoner(void) const {
    return "underconstruction";
  }

  py::tuple plot(py::list pysources, py::object info,
                 const std::string &path, py::dict opts);
};
#endif // DQM_VISDQMSTRIPCHARTSOURCE_H
