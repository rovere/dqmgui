#include "DQM/VisDQMOverlaySource.h"
#include "DQM/PyReleaseInterpreterLock.h"

py::tuple  VisDQMOverlaySource::plot(py::list overlay,
                                     py::dict opts) {
  std::map<std::string, std::string> options;
  bool imageok = false;
  std::string imagedata;
  std::string imagetype;
  copyopts(opts, options);

  std::vector<std::string> streamers;
  std::vector<DQMNet::Object> objects;
  streamers.reserve(py::len(overlay));
  objects.reserve(py::len(overlay));
  DQMNet::Object obj;
  std::string streamer;
  std::string path;

  // Collect overlaid input objects from the specified sources.
  // The python side provides us checked tuples with the actual
  // provider source object and sample type resolved, and the run
  // number already converted to an integer type.  What we need
  // to do here is to ask for the actual object, and if we are
  // happy with it, stash into the render list.
  for (py::stl_input_iterator<py::tuple> i(overlay), e; i != e; ++i) {
    py::tuple item = *i;
    py::extract<VisDQMSource *> src(item[0]);
    if ( !src.check() )
      continue;

    VisDQMSource *xsrc = src();
    VisDQMSample sample(SAMPLE_ANY,
                        py::extract<long>(item[1]),
                        py::extract<std::string>(item[2]));
    std::string opath = py::extract<std::string>(item[3]);

    {
      PyReleaseInterpreterLock nogil;
      xsrc->getdata(sample, opath, streamer, obj);
    }

    // If it's a ROOT object, or the first scalar, stack it.
    // If we have at least one object, ensure it's the same
    // fundamental type. Skip invalid or unknown objects.
    if ( !objects.empty() && getType(obj.flags) != getType(objects[0].flags) )
      continue;
    if ( !objects.empty() && isScalarType(obj.flags) )
      continue;
    if ( !isROOTType(obj.flags) && !isScalarType(obj.flags) )
      continue;

    streamers.push_back(streamer);
    objects.push_back(obj);
    if (path.empty())
      path = opath;
  }

  // If we ended up with something, issue a bulk draw.
  // Otherwise produce "missing in action" message.
  if (objects.size()) {
    PyReleaseInterpreterLock nogil;
    imageok = link_->render(imagedata, imagetype, path, options,
                            &streamers[0], &objects[0], objects.size(),
                            STDIMGOPTS);
  } else {
    PyReleaseInterpreterLock nogil;
    streamer.clear();
    clearobj(obj);
    imageok = link_->render(imagedata, imagetype, path, options,
                            &streamer, &obj, 1, STDIMGOPTS);
  }

  // Return the image we produced.
  if (imageok)
    return py::make_tuple(imagetype, imagedata);
  else
    return py::make_tuple(py::object(), py::object());
}
