#ifndef VISDQMCERTIFICATIONWORKSPACE_H
#define VISDQMCERTIFICATIONWORKSPACE_H

#include "DQM/VisDQMWorkspace.h"

#include "classlib/utils/Regexp.h"

#include "boost/python.hpp"

#include <map>
#include <string>

class VisDQMCertificationWorkspace : public VisDQMWorkspace
{
  typedef std::map<std::string, std::vector<std::string> > KeyVectorMap;

  Regexp rxevi_;
public:
  VisDQMCertificationWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name),
      rxevi_("(.*?)/EventInfo/(CertificationContents|DAQContents|DCSContents|reportSummaryContents)/(.*)")
    {
      rxevi_.study();
    }

  virtual ~VisDQMCertificationWorkspace(void)
    {}

  virtual std::string state(py::dict session);

private:
  static std::string certificationToJSON(const KeyVectorMap &cert);
};

#endif
