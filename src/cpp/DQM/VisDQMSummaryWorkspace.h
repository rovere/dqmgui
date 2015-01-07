#ifndef VISDQMSUMMARYWORKSPACE_H
#define VISDQMSUMMARYWORKSPACE_H

#include "DQM/VisDQMWorkspace.h"

#include "classlib/utils/Regexp.h"

#include "boost/python.hpp"

#include <map>
#include <string>

class VisDQMSummaryWorkspace : public VisDQMWorkspace
{
  typedef std::map<std::string, std::string> KeyValueMap;
  typedef std::map<std::string, KeyValueMap> SummaryMap;

  Regexp rxevi_;
public:
  VisDQMSummaryWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name),
      rxevi_("(.*?)/EventInfo/(.*)")
    {
      rxevi_.study();
    }

  virtual ~VisDQMSummaryWorkspace(void)
    {}

  virtual std::string state(py::dict session);

private:
  static std::string summaryToJSON(const SummaryMap &summary);
};

#endif // DQM_VISDQMSUMMARYWORKSPACE_H
