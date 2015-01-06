#ifndef DQM_VISDQMWORKSPACE_H
#define DQM_VISDQMWORKSPACE_H

#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMSource.h"

#include <dlfcn.h>
#include <sys/time.h>

#include <string>
#include <vector>

#include "boost/python.hpp"

class VisDQMSource;

namespace py = boost::python;

class VisDQMWorkspace
{
protected:
  py::object	gui_;
  double	guiTimeStamp_;
  std::string	guiServiceName_;
  std::string	guiServiceListJSON_;
  std::string	guiWorkspaceListJSON_;
  std::string	workspaceName_;
  void		(*snapdump_)(const char *);

public:
  VisDQMWorkspace(py::object gui, const std::string &name)
    : gui_(gui),
      guiTimeStamp_(py::extract<double>(gui.attr("stamp"))),
      guiServiceName_(py::extract<std::string>(gui.attr("serviceName"))),
      workspaceName_(name),
      snapdump_(0)
    {
      if (void *sym = dlsym(0, "igprof_dump_now"))
        snapdump_ = (void(*)(const char *)) sym;
    }

  virtual ~VisDQMWorkspace(void)
    {}

  virtual std::string
  state(py::dict /* session */)
    {
      return "{}";
    }

  virtual void
  profilesnap(void)
    {
      if (snapdump_)
      {
        char tofile[256];
        struct timeval tv;
        gettimeofday(&tv, 0);
        if (snprintf(tofile, sizeof(tofile), "|gzip -c>igprof.dqmgui.%ld.%f.gz",
	             (long) getpid(), tv.tv_sec + 1e-6*tv.tv_usec) < int(sizeof(tofile)))
	  snapdump_(tofile);
      }
    }

  // Helper function to compute next available run in current dataset
  int changeRun(py::dict session, bool forward);

protected:
  template <class T> T
  workspaceParam(const py::dict &session, const char *key)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(this->workspaceName_));
    }

  template <class T> T
  workspaceParam(const py::dict &session, const char *key, const T &def)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(this->workspaceName_, def));
    }

  template <class T> T
  workspaceParamOther(const py::dict &session, const char *key, const std::string &wspace)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(wspace));
    }

  template <class T> T
  workspaceParamOther(const py::dict &session, const char *key, const T &def, const std::string &wspace)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(wspace, def));
    }

  // Produce panel configuration format.
  static std::string sessionPanelConfig(const py::dict &session, const char *panel);

  // Produce zoom configuration format.
  static std::string sessionZoomConfig(const py::dict &session);

  // Produce Certification zoom configuration format.
  static std::string sessionCertZoomConfig(const py::dict &session);

  static std::string sessionReferenceOne(const py::dict &ref);

  // Produce a reference description.
  static std::string sessionReference(const py::dict &session);

  static std::string sessionStripChart(const py::dict &session);

  // Produce a standard response.
  std::string makeResponse(const std::string &state,
                           int interval,
                           VisDQMEventNum &current,
                           const std::string &services,
                           const std::string &workspace,
                           const std::string &workspaces,
                           const std::string &submenu,
                           const VisDQMSample &sample,
                           const std::string &filter,
                           const std::string &showstats,
                           const std::string &showerrbars,
                           const std::string &reference,
                           const std::string &strip,
                           const std::string &rxstr,
                           const std::string &rxerr,
                           size_t rxmatches,
                           const std::string &toolspanel,
                           Time startTime);

  // Get the list of native VisDQMSources configured in the GUI.
  void sources(std::vector<VisDQMSource *> &into);

  // Build and return a cached JSON representation of the DQM GUI
  // service list.  This must be called when it's safe to access
  // python.  Will build the list just once.
  const std::string & serviceListJSON(void);

  // Build and return a cached JSON representation of the DQM GUI
  // workspace object list.  This must be called when it's safe to
  // access python.  Will build the list just once.
  const std::string & workspaceListJSON(void);

  // Get the contents currently shown in this session.
  void buildContents(const std::vector<VisDQMSource *> &sources,
                     VisDQMItems &contents,
                     StringAtomSet &myobjs,
                     const VisDQMSample &sample,
                     VisDQMEventNum &current,
                     VisDQMRegexp *rxmatch,
                     Regexp *rxsearch,
                     bool *alarm,
                     std::string *layoutroot,
                     VisDQMRegexp *rxlayout);

  // Utility method to build a look-up table of shown objects.
  void buildShown(const VisDQMItems &contents,
                  StringAtomSet &shown,
                  const std::string &root);

  // Update the object metrics for the items in @a contents.
  // "Shown" means the object is included in canvas listing.
  void updateStatus(const VisDQMItems &contents,
                    VisDQMStatusMap &status);

private:
  VisDQMWorkspace(VisDQMWorkspace &);
  VisDQMWorkspace &operator=(VisDQMWorkspace &);
};


#endif  // DQM_VISDQMWORKSPACE_H
