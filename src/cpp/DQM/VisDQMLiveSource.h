#ifndef DQM_VISDQMLIVESOURCE_H
#define DQM_VISDQMLIVESOURCE_H

#include <string>

#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMRenderLink.h"
#include "DQM/VisDQMLiveThread.h"

class VisDQMLiveSource : public VisDQMSource {
  std::string mydataset_;
  VisDQMLiveThread *thread_;
  VisDQMRenderLink *link_;

 public:
  VisDQMLiveSource(py::object /* gui */, py::dict opts)
      : mydataset_(py::extract<std::string>(opts["dataset"])),
        thread_(new VisDQMLiveThread(this,
                                     py::extract<bool>(opts["verbose"]),
                                     py::extract<std::string>(opts["dqmhost"]),
                                     py::extract<int>(opts["dqmport"]))),
        link_(VisDQMRenderLink::instance())
  {}

  ~VisDQMLiveSource(void) {
    if (thread_)
      exit();
  }

  virtual const char *
  plotter(void) const {
    return "live";
  }

  virtual const char *
  jsoner(void) const {
    return "underconstruction";
  }

  void
  exit(void) {
    if (thread_) {
      thread_->stop();
      delete thread_;
      thread_ = 0;
    }
  }

  virtual void getdata(const VisDQMSample &, const std::string &path,
                       std::string &streamers, DQMNet::Object &obj);
  virtual void getattr(const VisDQMSample &, const std::string &path,
          VisDQMIndex::Summary &attrs, std::string &xstreamers,
          DQMNet::Object &xobj);
  py::tuple plot(const std::string &path, py::dict opts);

  virtual void scan(VisDQMItems &result, const VisDQMSample &sample,
                    VisDQMEventNum &current, VisDQMRegexp *rxmatch,
                    Regexp *rxsearch, bool *alarm, std::string *,
                    VisDQMRegexp *);
  virtual void prepareSession(py::dict session);
  virtual void json(const VisDQMSample &sample, const std::string &rootpath,
                    bool fulldata, bool lumisect, std::string &result,
                    double &stamp, const std::string &name,
                    std::set<std::string> &dirs);
  virtual void samples(VisDQMSamples &samples);
};

#endif // DQM_VISDQMLIVESOURCE_H
