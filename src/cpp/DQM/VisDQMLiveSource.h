#ifndef DQM_VISDQMLIVESOURCE_H
#define DQM_VISDQMLIVESOURCE_H

#include <string>

#include "DQM/DQMNet.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMLiveThread.h"

#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

class VisDQMRenderLink;
class VisDQMLiveThread;
struct VisDQMSample;

namespace py = boost::python;

class VisDQMLiveSource : public VisDQMSource
{
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

  ~VisDQMLiveSource(void)
    {
      if (thread_)
	exit();
    }

  virtual const char *
  plotter(void) const
    {
      return "live";
    }

  virtual const char *
  jsoner(void) const
    {
      return "underconstruction";
    }

  virtual void
  getdata(const VisDQMSample & /* sample */,
	  const std::string &path,
	  std::string &streamers,
	  DQMNet::Object &obj)
    {
      streamers.clear();
      clearobj(obj);
      thread_->fetch(path, streamers, obj);
    }

  virtual void
  getattr(const VisDQMSample & /* sample */,
	  const std::string &path,
	  VisDQMIndex::Summary &attrs,
	  std::string &xstreamers,
	  DQMNet::Object &xobj)
    {
      clearattr(attrs);
      thread_->fetch(path, xstreamers, xobj);
    }

  py::tuple
      plot(const std::string &path, py::dict opts)
  {
    VisDQMSample sample(SAMPLE_ANY, -1);
    std::map<std::string,std::string> options;
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

  void
  exit(void)
    {
      if (thread_)
      {
        thread_->stop();
        delete thread_;
        thread_ = 0;
      }
    }

  virtual void
  prepareSession(py::dict session)
    {
      if (! session.has_key("dqm.sample.type"))
      {
	session["dqm.sample.runnr"] = 0L;
	session["dqm.sample.dataset"] = mydataset_;
	session["dqm.sample.importversion"] = 0;
	session["dqm.sample.type"] = long(SAMPLE_LIVE);
      }
    }

  virtual void
  scan(VisDQMItems &result,
       const VisDQMSample &sample,
       VisDQMEventNum &current,
       VisDQMRegexp *rxmatch,
       Regexp *rxsearch,
       bool *alarm,
       std::string *,
       VisDQMRegexp *)
    {
      if (sample.type == SAMPLE_LIVE)
	thread_->scan(result, current, rxmatch, rxsearch, alarm);
    }

  virtual void
  json(const VisDQMSample &sample,
       const std::string &rootpath,
       bool fulldata,
       bool lumisect,
       std::string &result,
       double &stamp,
       const std::string &name,
       std::set<std::string> &dirs)
    {
      if (sample.type == SAMPLE_LIVE || sample.type == SAMPLE_ANY)
	thread_->json(rootpath, fulldata, lumisect, result, stamp, name, dirs);
    }

  virtual void
  samples(VisDQMSamples &samples)
    {
      samples.push_back(VisDQMSample());
      VisDQMSample &s = samples.back();
      s.type = SAMPLE_LIVE;
      s.runnr = 0;
      s.dataset = mydataset_;
      s.origin = this;
      s.time = Time::current().ns();
    }
};

#endif  // DQM_VISDQMLIVESOURCE_H
