#ifndef DQM_VISDQMTOJSON_H
#define DQM_VISDQMTOJSON_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMEventNum.h"
#include "DQM/VisDQMSourceObjects.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

using namespace lat;
namespace py = boost::python;

class VisDQMSource;

/** Dump catalogue and objects out in JSON. */
class VisDQMToJSON
{
 public:
  VisDQMToJSON(void)
  {}

  py::tuple samples(py::list pysources, py::dict opts);

  py::tuple list(VisDQMSource *layoutsrc,
                 VisDQMSource *src,
                 int runnr,
                 const std::string &dataset,
                 const std::string &path,
                 py::dict opts);

  void json(const VisDQMItems &contents,
	    VisDQMSample &sample,
	    VisDQMSource *src,
	    const std::map<std::string, std::string> &options,
	    double &stamp,
	    const std::string &rootpath,
	    std::string &result,
	    std::set<std::string> &dirs);
};

#endif // DQM_VISDQMTOJSON_H
