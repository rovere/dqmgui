#include "DQM/VisDQMLiveSource.h"

#include <map>

#include "DQM/PyReleaseInterpreterLock.h"

void VisDQMLiveSource::getdata(const VisDQMSample & /* sample */,
                                       const std::string &path,
                                       std::string &streamers,
                                       DQMNet::Object &obj) {
  streamers.clear();
  clearobj(obj);
  thread_->fetch(path, streamers, obj);
}

void VisDQMLiveSource::getattr(const VisDQMSample & /* sample */,
                                       const std::string &path,
                                       VisDQMIndex::Summary &attrs,
                                       std::string &xstreamers,
                                       DQMNet::Object &xobj) {
  clearattr(attrs);
  thread_->fetch(path, xstreamers, xobj);
}

py::tuple VisDQMLiveSource::plot(const std::string &path, py::dict opts) {
  VisDQMSample sample(SAMPLE_ANY, -1);
  std::map<std::string, std::string> options;
  bool imageok = false;
  std::string imagedata;
  std::string imagetype;
  std::string streamers;
  DQMNet::Object obj;
  copyopts(opts, options);

  {
    PyReleaseInterpreterLock nogil;
    getdata(sample, path, streamers, obj);
    imageok = link_->render(imagedata, imagetype, path, options,
                            &streamers, &obj, 1, STDIMGOPTS);
  }

  if (imageok)
    return py::make_tuple(imagetype, imagedata);
  else
    return py::make_tuple(py::object(), py::object());
}

void VisDQMLiveSource::prepareSession(py::dict session) {
  if ( !session.has_key("dqm.sample.type")) {
    session["dqm.sample.runnr"] = 0L;
    session["dqm.sample.dataset"] = mydataset_;
    session["dqm.sample.importversion"] = 0;
    session["dqm.sample.type"] = long(SAMPLE_LIVE);
  }
}

void VisDQMLiveSource::scan(VisDQMItems &result,
                                    const VisDQMSample &sample,
                                    VisDQMEventNum &current,
                                    VisDQMRegexp *rxmatch,
                                    Regexp *rxsearch,
                                    bool *alarm,
                                    std::string *,
                                    VisDQMRegexp *) {
  if (sample.type == SAMPLE_LIVE)
    thread_->scan(result, current, rxmatch, rxsearch, alarm);
}

void VisDQMLiveSource::json(const VisDQMSample &sample,
                                    const std::string &rootpath,
                                    bool fulldata,
                                    bool lumisect,
                                    std::string &result,
                                    double &stamp,
                                    const std::string &name,
                                    std::set<std::string> &dirs) {
  if (sample.type == SAMPLE_LIVE || sample.type == SAMPLE_ANY)
    thread_->json(rootpath, fulldata, lumisect, result, stamp, name, dirs);
}

void VisDQMLiveSource::samples(VisDQMSamples &samples) {
  samples.push_back(VisDQMSample());
  VisDQMSample &s = samples.back();
  s.type = SAMPLE_LIVE;
  s.runnr = 0;
  s.dataset = mydataset_;
  s.origin = this;
  s.time = Time::current().ns();
}
