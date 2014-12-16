#include "DQM/VisDQMToJSON.h"

#include <string>
#include <vector>
#include <set>
#include <map>

#include "DQM/VisDQMLocks.h"
#include "DQM/VisDQMSample.h"
#include "DQM/VisDQMServerTools.h"
#include "DQM/VisDQMEventNum.h"
#include "DQM/VisDQMSourceObjects.h"
#include "DQM/VisDQMSource.h"

#include "classlib/utils/StringFormat.h"
#include "classlib/utils/Regexp.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

using namespace lat;
namespace py = boost::python;

py::tuple VisDQMToJSON::samples(py::list pysources, py::dict opts)
{
  double stamp = 0;
  std::string result;
  std::map<std::string, std::string> options;
  std::vector<VisDQMSource *> sources;
  shared_ptr<Regexp> rx;
  copyopts(opts, options);
  sources.reserve(py::len(pysources) + 1);

  // Get all the sources which have a C++ part we can query.
  for (py::stl_input_iterator<py::object> i(pysources), e; i != e; ++i)
  {
    py::extract<VisDQMSource *> src(*i);
    if (src.check())
      sources.push_back(src());
  }

  {
    PyReleaseInterpreterLock nogil;

    // Get all the samples from all the sources.
    VisDQMSamples samples;
    for (size_t i = 0, e = sources.size(); i != e; ++i)
      sources[i]->samples(samples);

    // Match dataset names against dataset name regexp if one was given.
    if (options.count("match"))
    {
      std::string rxerr;
      makerx(options["match"], rx, rxerr, Regexp::IgnoreCase);
      if (rx)
      {
        VisDQMSamples final;
        final.reserve(samples.size());
        for (size_t i = 0, e = samples.size(); i != e; ++i)
          if (rx->search(samples[i].dataset) >= 0)
            final.push_back(samples[i]);
        std::swap(final, samples);
      }
    }

    for (size_t i = 0, e = samples.size(); i != e; ++i)
      stamp = std::max(stamp, samples[i].time * 1e-9);

    result = StringFormat("{\"samples\": [%1]}").arg(samplesToJSON(samples));
  }

  return py::make_tuple(stamp, result);
}

py::tuple
VisDQMToJSON::list(VisDQMSource *layoutsrc,
                   VisDQMSource *src,
                   int runnr,
                   const std::string &dataset,
                   const std::string &path,
                   py::dict opts)
{
  VisDQMSample sample(SAMPLE_ANY, runnr, dataset);
  std::map<std::string, std::string> options;
  std::set<std::string> dirs;
  VisDQMItems    layoutResult;
  double         stamp = 0;
  std::string    result;
  VisDQMRegexp   rxmatch;
  std::string    layoutroot;
  VisDQMEventNum current = { "", -1, -1, -1, -1 };
  bool           alarm = false;
  VisDQMRegexp   rxlayout;
  Regexp         *rxsearch = NULL;

  copyopts(opts, options);
  fastrx(rxmatch, path);
  fastrx(rxlayout, "(^" + path + ").*");

  {
    PyReleaseInterpreterLock nogil;
    src->json(sample, path, options.count("rootcontent") > 0, options.count("lumisect") > 0, result, stamp, "", dirs);

    // Add layout content, in case a layout source had been
    // registered to the main server.
    if (layoutsrc)
    {
      layoutsrc->scan(layoutResult,
                      sample,
                      current,
                      &rxmatch,
                      rxsearch,
                      &alarm,
                      &layoutroot,
                      &rxlayout);
      json(layoutResult, sample, src, options, stamp, path, result, dirs);
    }
    result = StringFormat("{\"contents\": [%1]}").arg(result);
  }

  // Return the json we produced.
  return py::make_tuple(stamp, result);
}

void VisDQMToJSON::json(const VisDQMItems &contents,
                        VisDQMSample &sample,
                        VisDQMSource *src,
                        const std::map<std::string, std::string> &options,
                        double &stamp,
                        const std::string &rootpath,
                        std::string &result,
                        std::set<std::string> &dirs)
{
  VisDQMItems::const_iterator ci, ce;
  std::set<std::string>::iterator di, de;
  std::set<std::string> newdirs;
  std::string name;
  std::string dir;
  bool doroot = options.count("rootcontent") > 0;
  bool dolumi = options.count("lumisect") > 0;

  for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
  {
    name.clear();
    dir.clear();
    splitPath(dir, name, ci->second->name.string());
    if (rootpath == dir)
      /* Keep object directly in rootpath directory */;
    else if (isSubdirectory(rootpath, dir))
    {
      // Object in subdirectory, remember subdirectory part
      size_t begin = (rootpath.empty() ? 0 : rootpath.size()+1);
      size_t slash = dir.find('/', begin);
      if (dirs.find(std::string(dir, begin, slash - begin)) == dirs.end())
        newdirs.insert(std::string(dir, begin, slash - begin));
      continue;
    }
    else
      // Object outside directory of interest to us, skip
      continue;

    VisDQMItem &o = *ci->second;
    if (o.layout)
    {
      VisDQMLayoutRows &rows = *o.layout;
      for (size_t nrow = 0; nrow < rows.size(); ++nrow)
      {
        VisDQMLayoutRow &row = *rows[nrow];
        for (size_t ncol = 0; ncol < row.columns.size(); ++ncol)
        {
          name.clear();
          dir.clear();
          VisDQMLayoutItem &col = *row.columns[ncol];
          const std::string path(col.path.string());
          splitPath(dir, name, path);
          // The check on the name (i.e. ME name) and directory
          // is mandatory here since we want to avoid the case
          // in which part of the layout has been defined using
          // python's None: this, in the current implementation,
          // would have the side effect of including all
          // directories, starting from the root one.
          if (!name.empty() && !dir.empty())
            src->json(sample, dir, doroot, dolumi, result, stamp, name, dirs);
        }
      }
    }
  }

  // Format sub-directories.
  for (di = newdirs.begin(), de = newdirs.end(); di != de; ++di)
    result += StringFormat(", { \"subdir\": %1 }\n").arg(stringToJSON(*di));

  // Successfully read the data, return.
  return;
}

