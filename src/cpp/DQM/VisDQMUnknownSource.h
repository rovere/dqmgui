#ifndef DQM_VISDQMUNKNOWNSOURCE_H
#define DQM_VISDQMUNKNOWNSOURCE_H

#include <map>
#include <string>

#include "DQM/VisDQMSource.h"
#include "DQM/DQMNet.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMLocks.h"

class VisDQMRenderLink;

namespace py = boost::python;

class VisDQMUnknownSource : public VisDQMSource
{
  VisDQMRenderLink *link_;
public:
  VisDQMUnknownSource(void)
    : link_(VisDQMRenderLink::instance())
    {}

  py::tuple
  plot(const std::string &path, py::dict opts)
    {
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
};

#endif // DQM_VISDQMUNKNOWNSOURCE_H
