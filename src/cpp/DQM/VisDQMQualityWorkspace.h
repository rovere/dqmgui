#ifndef DQM_VISDQMQUALITYWORKSPACE_H
#define DQM_VISDQMQUALITYWORKSPACE_H

#include "DQM/VisDQMWorkspace.h"

#include "classlib/utils/Regexp.h"

#include "boost/python.hpp"

#include <map>
#include <string>

class VisDQMQualityWorkspace : public VisDQMWorkspace
{
  struct QMapItem
  {
    std::string location;
    std::string name;
    std::string	reportSummary;
    std::string eventTimeStamp;
    uint64_t version;
  };

  typedef std::map<std::string, QMapItem> QMap;
public:
  VisDQMQualityWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMQualityWorkspace(void)
    {}

  virtual std::string state(py::dict session);

private:
  static std::string qmapToJSON(const QMap &qmap);
};

#endif  // DQM_VISDQMQUALITYWORKSPACE_H
