#include "DQM/VisDQMStripChartSource.h"

#include "classlib/utils/StringOps.h"

#include <string>
#include <vector>
#include <map>

#include "DQM/VisDQMSource.h"
#include "DQM/DQMNet.h"
#include "DQM/Objects.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMLocks.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMRenderLink.h"

namespace py = boost::python;

VisDQMStripChartSource::VisDQMStripChartSource(void)
    : link_(VisDQMRenderLink::instance())
{}

py::tuple
VisDQMStripChartSource::plot(py::list pysources, py::object info,
                             const std::string &path, py::dict opts)
{
  VisDQMSample cursample(SAMPLE_ANY, -1);
  std::map<std::string, std::string> options;
  bool imageok = false;
  std::string imagedata;
  std::string imagetype;
  std::string origdataset;
  std::vector<VisDQMSource *> sources;
  copyopts(opts, options);
  sources.reserve(py::len(pysources) + 1);

  // Get the current source if one was given.
  if (info.ptr() != Py_None)
  {
    py::tuple t = py::extract<py::tuple>(info);
    py::extract<VisDQMSource *> src(t[0]);
    if (src.check())
    {
      // Extract run number and dataset name.  Remember both the
      // mutilated and original dataset names, former for relval
      // and the other for other sample matches.  At this stage we
      // don't know the type of this sample.
      cursample.runnr = py::extract<long>(t[1]);
      origdataset = py::extract<std::string>(t[2]);
      cursample.dataset = StringOps::remove(origdataset, RX_CMSSW_VERSION);
    }
  }

  // Get all the sources which have a C++ part we can query.
  for (py::stl_input_iterator<py::object> i(pysources), e; i != e; ++i)
  {
    py::extract<VisDQMSource *> src(*i);
    if (src.check())
      sources.push_back(src());
  }

  // Now let python go and start doing real work.
  {
    PyReleaseInterpreterLock nogil;

    // Get all the samples from all the sources.
    VisDQMSamples samples;
    for (size_t i = 0, e = sources.size(); i != e; ++i)
      sources[i]->samples(samples);

    // If we have a current run/dataset, filter samples to the
    // same dataset, and ignore cmssw variation in relval samples
    // (note: the same dataset requirement makes live and online
    // interchangeable, which is what we want).  Keep only samples
    // which precede the current sample (if any).
    std::vector<VisDQMSample> final;
    final.reserve(samples.size());
    for (size_t i = 0, e = samples.size(); i != e; ++i)
    {
      VisDQMSample &s = samples[i];
      bool include = false;
      if (cursample.runnr == -1 && cursample.dataset.empty())
        // accept everything if we have no starting sample
        include = true;
      else if (cursample.runnr > 0
               && (s.runnr <= 0 || s.runnr > cursample.runnr))
        // skip later runs if we have a run number specified
        ;
      else if (s.type != SAMPLE_OFFLINE_RELVAL)
        // require exact dataset name match for non-relval data
        include = (s.dataset == origdataset);
      else if (s.type == SAMPLE_OFFLINE_RELVAL)
        // require fuzzy dataset name match for relval data
        include = (StringOps::remove(s.dataset, RX_CMSSW_VERSION)
                   == cursample.dataset);

      if (include)
        final.push_back(s);
    }

    // Sort the samples by type and run (or CMSSW version).  This
    // leaves the list sorted for most recent to oldest data.  The
    // current sample is guaranteed to be the first one (if any).
    std::sort(final.begin(), final.end(), orderSamplesByRun);

    // Determine options: number of samples to include, display
    // x-axis as run or time, and whether to exclude histograms
    // with no entries (=~ wasn't present in the run).
    std::string runopt = "n";
    std::string skipopt = "omit";
    std::string axisopt = "axis";
    size_t keep = final.size();
    bool skipempty = false;
    bool axistime = false;

    if (options.find(runopt) != options.end() && ! options[runopt].empty())
      keep = std::min(keep, (size_t) atoi(options[runopt].c_str()));
    if (options.find(skipopt) != options.end() && options[skipopt] == "empty")
      skipempty = true;
    if (options.find(axisopt) != options.end() && options[axisopt] == "time")
      axistime = true;

    // Get the object summary data for the requested object for
    // each sample. Live DQM is special because it cannot provide
    // the attributes -- it provides the full ROOT object, which
    // the plotting engine will deserialise.  We assume here live
    // will only provide one object, and will always make it the
    // last sample if live is included.  Also it has to be a ROOT
    // object, not a scalar; this is handled below by checking
    // only on xobj[1].rawdata, not xobj[1].scalar.
    DQMNet::Object xobj[2];
    std::string streamers[2];
    std::string binlabels;
    std::vector<double> axisvals;
    std::vector<VisDQMIndex::Summary> attrs;
    VisDQMIndex::Summary oneattr;
    attrs.reserve(final.size());
    axisvals.reserve(final.size());
    binlabels.reserve(10 * final.size());
    clearobj(xobj[0]);
    clearobj(xobj[1]);
    for (size_t i = 0, e = final.size(); i != e; ++i)
    {
      // If we have enough samples, give up.
      if (attrs.size() == keep)
        break;

      // Get the attribute here.
      final[i].origin->getattr(final[i], path, oneattr,
                               streamers[1], xobj[1]);

      // Skip if empty and skipping was requested.
      // We can't know if live is empty or not.
      if (skipempty
          && ! oneattr.nentries
          && final[i].type != SAMPLE_LIVE)
        continue;

      // OK, save this, its axis value and label (runnr or
      // version, or string for time label).
      std::string binlabel;
      if (final[i].version.size())
        binlabel = final[i].version;
      else if (final[i].type == SAMPLE_LIVE)
        binlabel = "Live";
      else
      {
        char tmpbuf[64];
        sprintf(tmpbuf, "%ld", final[i].runnr);
        binlabel = tmpbuf;
      }

      if (axistime)
      {
        binlabel += Time(final[i].time).format(false, ": %b %d %H:%M");
        axisvals.push_back(final[i].time * 1e-9);
      }
      else
        axisvals.push_back(axisvals.size());

      binlabels.append(binlabel.c_str(), binlabel.size()+1);
      attrs.push_back(oneattr);
    }

    // Generate the render request.  We masquerade the summary
    // data as the first object and any live object as another.
    // We use different image option set which the caller cannot
    // use otherwise, so the render server can clearly distinguish
    // the request from other requests and take special action.
    // We abuse 'tag' to let server know the attribute count.
    if (attrs.size())
    {
      assert(attrs.size() == axisvals.size());

      // Make sure the 'trend' parameter is defined.
      if (options.find("trend") == options.end())
        options["trend"] = "y-mean-rms";

      // NOTE: This deliberately ignores xobj[1] if it's scalar.
      // The current protocol does not work for live scalar type.
      size_t attrsize = attrs.size() * sizeof(VisDQMIndex::Summary);
      size_t axissize = attrs.size() * sizeof(double);
      size_t binssize = binlabels.size();
      xobj[0].tag = attrs.size();
      xobj[0].flags = DQMNet::DQM_PROP_TYPE_DATABLOB
          | (axistime ? DQM_PROP_STRIP_AXIS_TIME : 0)
          | (skipempty ? DQM_PROP_STRIP_SKIP_EMPTY : 0);
      xobj[0].rawdata.resize(attrsize + axissize + binlabels.size());
      memcpy(&xobj[0].rawdata[0], &axisvals[0], axissize);
      memcpy(&xobj[0].rawdata[axissize], &attrs[0], attrsize);
      memcpy(&xobj[0].rawdata[axissize+attrsize], &binlabels[0], binssize);
      imageok = link_->render(imagedata, imagetype, path, options,
                              streamers, xobj,
                              (xobj[1].rawdata.size() ? 2 : 1),
                              TRENDIMGOPTS);
    }
    else
    {
      imageok = link_->render(imagedata, imagetype, path, options,
                              streamers, xobj, 1, STDIMGOPTS);
    }
  }

  // Return the image we produced.
  if (imageok)
    return py::make_tuple(imagetype, imagedata);
  else
    return py::make_tuple(py::object(), py::object());
}
