#include "DQM/VisDQMCertificationSource.h"

#include <string>
#include <vector>
#include <map>

#include "classlib/utils/StringOps.h"

#include "DQM/DQMNet.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMLocks.h"
#include "DQM/VisDQMRenderLink.h"
#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMSample.h"

namespace py = boost::python;

/** Make Data Certification. */
VisDQMCertificationSource::VisDQMCertificationSource(void)
    : link_(VisDQMRenderLink::instance())
{}

py::tuple
VisDQMCertificationSource::plot(py::object info,
                                const std::string &fullpath,
                                const std::string &path,
                                const std::string &variableName, py::dict opts)
{
  VisDQMSample cursample(SAMPLE_ANY, -1);
  std::map<std::string, std::string> options;
  bool imageok = false;
  std::string imagedata;
  std::string imagetype;
  std::string origdataset;
  copyopts(opts, options);

  // Get the current source.
  if (info.ptr() != Py_None)
  {
    py::tuple t = py::extract<py::tuple>(info);
    py::extract<VisDQMSource *> src(t[0]);
    if (src.check())
    {
      DQMNet::Object xobj[2];
      std::string streamers[2];
      std::string binlabels;
      std::vector<double> axisvals;
      std::vector<VisDQMIndex::Summary> attrs;
      // Extract run number and dataset name.  Remember both the
      // mutilated and original dataset names, former for relval
      // and the other for other sample matches.  At this stage we
      // don't know the type of this sample.
      cursample.runnr = py::extract<long>(t[1]);
      origdataset = py::extract<std::string>(t[2]);
      cursample.dataset = StringOps::remove(origdataset, RX_CMSSW_VERSION);
      // Get the attributes here. An accompanying vector is passed
      // and filled with actual values of lumisection number: this
      // is mandatory in case we miss some lumisection.
      src()->getcert(cursample, path, variableName, attrs, axisvals, binlabels);
      clearobj(xobj[0]);
      clearobj(xobj[1]);
      if (attrs.size())
      {
        assert(attrs.size() == axisvals.size());
        size_t attrsize = attrs.size() * sizeof(VisDQMIndex::Summary);
        size_t axissize = attrs.size() * sizeof(double);
        size_t binssize = binlabels.size();
        xobj[0].tag = attrs.size();
        xobj[0].flags = DQMNet::DQM_PROP_TYPE_DATABLOB;
        xobj[0].rawdata.resize(attrsize + axissize + binlabels.size());
        memcpy(&xobj[0].rawdata[0], &axisvals[0], axissize);
        memcpy(&xobj[0].rawdata[axissize], &attrs[0], attrsize);
        memcpy(&xobj[0].rawdata[axissize+attrsize], &binlabels[0], binssize);
        imageok = link_->render(imagedata, imagetype, fullpath, options,
                                streamers, xobj,
                                1,
                                TRENDIMGOPTS);
      }
      else
      {
        imageok = link_->render(imagedata, imagetype, path, options,
                                streamers, xobj, 1, STDIMGOPTS);
      }
    }
  }
  // Return the image we produced.
  if (imageok)
    return py::make_tuple(imagetype, imagedata);
  else
    return py::make_tuple(py::object(), py::object());
}
