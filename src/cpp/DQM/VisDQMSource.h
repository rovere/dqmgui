#ifndef DQM_VISDQMSOURCE_H
#define DQM_VISDQMSOURCE_H

#include "DQM/DQMNet.h"
#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMSourceObjects.h"

/** Base class for providing DQM objects. */
class VisDQMSource
{
protected:
  void
  clearobj(DQMNet::Object &obj)
    {
      obj.flags = DQMNet::DQM_PROP_TYPE_INVALID;
      obj.tag = 0;
      obj.version = 0;
      obj.dirname = 0;
      obj.hash = 0;
      obj.lastreq = 0;
      obj.objname.clear();
      obj.scalar.clear();
      obj.rawdata.clear();
      obj.qdata.clear();
    }

  void
  clearattr(VisDQMIndex::Summary &attrs)
    {
      memset(&attrs, 0, sizeof(attrs));
    }

public:
  VisDQMSource(void)
    {}

  virtual ~VisDQMSource(void)
    {}

  virtual const char *
  plotter(void) const
    {
      return "unknown";
    }

  virtual const char *
  jsoner(void) const
    {
      return "unknown";
    }

  virtual void
  prepareSession(py::dict /* session */)
    {}

  virtual void
  getdata(const VisDQMSample & /* sample */,
          const std::string & /* path */,
          std::string &streamers,
          DQMNet::Object &obj)
    {
      streamers.clear();
      clearobj(obj);
    }

  virtual void
  getattr(const VisDQMSample & /* sample */,
          const std::string & /* path */,
          VisDQMIndex::Summary &attrs,
          std::string & /* xstreamers */,
          DQMNet::Object & /* xobj */)
    {
      clearattr(attrs);
    }

  virtual void
  prescan(void)
    {}

  virtual void
  scan(VisDQMItems & /* result */,
       const VisDQMSample & /* sample */,
       VisDQMEventNum & /* current */,
       VisDQMRegexp * /* rxmatch */,
       Regexp * /* rxsearch */,
       bool * /* alarm */,
       std::string * /* layoutroot */,
       VisDQMRegexp * /* rxlayout */)
    {}

  virtual void
  json(const VisDQMSample & /* sample */,
       const std::string & /* rootpath */,
       bool /* fulldata */,
       bool /* lumisect */,
       std::string & /* result */,
       double & /* stamp */,
       const std::string & /* name */,
       std::set<std::string> & /* dirs */)
    {}

  virtual void
  samples(VisDQMSamples & /* samples */)
    {}

  virtual void
  getcert(VisDQMSample & /* sample */,
          const std::string & /* path */,
          const std::string & /* variableName */,
          std::vector<VisDQMIndex::Summary> & /* attrs */,
          std::vector<double> & /* axisvals */,
          std::string & /* binlabels */)
    {}
};

#endif // DQM_VISDQMSOURCE_H
