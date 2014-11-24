#ifndef DQM_VISDQMCERTIFICATIONSOURCE_H
#define DQM_VISDQMCERTIFICATIONSOURCE_H

#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMRenderLink.h"

/** Make Data Certification. */
class VisDQMCertificationSource : public VisDQMSource {
  VisDQMRenderLink *link_;

 public:
  VisDQMCertificationSource(void)
      : link_(VisDQMRenderLink::instance())
  {}

  virtual const char *
  plotter(void) const {
    return "certification";
  }

  virtual const char *
  jsoner(void) const {
    return "underconstruction";
  }

  py::tuple plot(py::object info, const std::string &fullpath,
                 const std::string &path, const std::string &variableName,
                 py::dict opts);
};

#endif // DQM_VISDQMCERTIFICATIONSOURCE_H
