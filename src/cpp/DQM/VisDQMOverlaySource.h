#ifndef DQM_VISDQMOVERLAYSOURCE_H
#define DQM_VISDQMOVERLAYSOURCE_H

#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMRenderLink.h"

/** Overlay several ROOT objects on top of each other. */
class VisDQMOverlaySource : public VisDQMSource {
  VisDQMRenderLink *link_;

 public:
  VisDQMOverlaySource(void)
      : link_(VisDQMRenderLink::instance())
  {}

  virtual const char *
  plotter(void) const {
    return "overlay";
  }

  virtual const char *
  jsoner(void) const {
    return "underconstruction";
  }

  py::tuple plot(py::list overlay, py::dict opts);
};

#endif // DQM_VISDQMOVERLAYSOURCE_H
