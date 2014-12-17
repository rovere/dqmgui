#ifndef DQM_VISDQMCERTIFICATIONSOURCE_H
#define DQM_VISDQMCERTIFICATIONSOURCE_H

#include <string>

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

#include "DQM/VisDQMSource.h"

class VisDQMRenderLink;

namespace py = boost::python;

/** Make Data Certification. */
class VisDQMCertificationSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMCertificationSource(void);

  virtual const char *
  plotter(void) const
    {
      return "certification";
    }

  virtual const char *
  jsoner(void) const
    {
      return "underconstruction";
    }

  py::tuple
  plot(py::object info, const std::string &fullpath,
       const std::string &path, const std::string &variableName,
       py::dict opts);
};

#endif // DQM_VISDQMCERTIFICATIONSOURCE_H
