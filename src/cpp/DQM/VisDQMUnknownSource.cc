#include "DQM/VisDQMUnknownSource.h"

#include <string>
#include <map>

#include "DQM/PyReleaseInterpreterLock.h"

namespace py = boost::python;

py::tuple VisDQMUnknownSource::plot(const std::string &path,
                                    py::dict opts) {
  std::map<std::string, std::string> options;
  bool imageok = false;
  std::string imagedata;
  std::string imagetype;
  std::string streamers;
  DQMNet::Object obj;
  copyopts(opts, options);

  {
    PyReleaseInterpreterLock nogil;
    streamers.clear();
    clearobj(obj);
    imageok = link_->render(imagedata, imagetype, path, options,
                            &streamers, &obj, 1, STDIMGOPTS);
  }

  if (imageok)
    return py::make_tuple(imagetype, imagedata);
  else
    return py::make_tuple(py::object(), py::object());
}
