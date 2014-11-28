#ifndef DQM_VISDQMSERVERTOOLS_H
#define DQM_VISDQMSERVERTOOLS_H

/** This file contains utility functions that are exclusively used by
    the serverext packages within the Accelerator shared library used
    to link the C++ back-end to the python frontend. These have not
    been included in the other utility file VisDQMTools.h since they
    do not pertaing neither to rendering, nor to indexing and would
    have therefore increased code side w/o any reason.  */

// Shut up warning about _POSIX_C_SOURCE mismatch, caused by a
// harmless conflict between _GNU_SOURCE and python header files.
//#define NDEBUG 1
#include <unistd.h>
#if _POSIX_C_SOURCE != 200112L
# undef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
#endif
#define DEBUG(n,x)

#include <iostream>
#include <string>
#include <map>
#include <set>

#include "DQM/NatSort.h"
#include "DQM/StringAtom.h"
#include "DQM/DQMNet.h"
#include "DQM/VisDQMIndex.h"
#include "DQM/VisDQMRegexp.h"
#include "DQM/VisDQMDrawOptions.h"
#include "DQM/VisDQMEventNum.h"
#include "DQM/VisDQMSample.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

struct VisDQMRegexp;
struct VisDQMDrawOptions;
struct VisDQMEventNum;
struct VisDQMSample;

using namespace lat;
using namespace boost;



namespace py = boost::python;
// WARNING: these typedef has to be centralized between this file and
// serverext.cc
typedef std::set<StringAtom> StringAtomSet;
typedef std::list<VisDQMEventNum> VisDQMEventNumList;

static Regexp RX_THOUSANDS("(\\d)(\\d{3}($|\\D))");

// Custom function to log generic events prepending a properly
// formatted time information.
std::ostream & lograw(const char *label);
std::ostream & loginfo(void);
std::ostream & logerr(void);
std::ostream & logwarn(void);
std::string thousands(const std::string &arg);
void copyopts(const py::dict &opts,
              std::map<std::string, std::string> &options);
bool isSubdirectory(const std::string &ofdir, const std::string &path);
void splitPath(std::string &dir, std::string &name, const std::string &path);

std::string stringToJSON(const std::string &x,
                         bool emptyIsNone = false);
std::string stringsToJSON(const StringAtomSet &overlays,
                          bool emptyIsNone = false);
shared_ptr<Regexp> rx(const std::string &match, int options = 0);
void makerx(const std::string &rxstr,
            shared_ptr<Regexp> &rxobj,
            std::string &rxerr,
            int options = 0);
void fastrx(VisDQMRegexp &rxobj, const std::string &rxstr);
bool fastmatch(VisDQMRegexp *rx, const StringAtom &str);

uint32_t getType(uint32_t flags);
bool isROOTType(uint32_t flags);
bool isScalarType(uint32_t flags);
bool isBLOBType(uint32_t flags);
bool isIntegerType(uint32_t flags);
bool isRealType(uint32_t flags);

void translateDrawOptions(py::dict opts, VisDQMDrawOptions &o);
std::string formatStartTime(long curStartTime);
void getEventInfoNum(const std::string &name,
                     const char *data,
                     VisDQMEventNumList &eventnums);

// Pick greatest run/lumi/event number combo seen.
void setEventInfoNums(const VisDQMEventNumList &eventnums,
                      VisDQMEventNum &current);

std::string axisStatsToJSON(uint32_t nbins[3],
                            double mean[3],
                            double rms[3],
                            double bounds[3][2],
                            int axis);
void objectToJSON(const std::string &name,
                  const char *value,
                  const char *qdata,
                  DQMNet::DataBlob &rawdata,
                  uint64_t lumisect,
                  uint32_t flags,
                  uint32_t tag,
                  double nentries,
                  uint32_t nbins[3],
                  double mean[3],
                  double rms[3],
                  double bounds[3][2],
                  DQMNet::QReports &qreports,
                  std::string &qstr,
                  std::string &result);
void sampleToJSON(const VisDQMSample &sample, std::string &result);
std::string sampleToJSON(const VisDQMSample &sample);
std::string samplesToJSON(VisDQMSamples &samples);
VisDQMSample sessionSample(const py::dict &session);

inline
bool orderSamplesByDataset(const VisDQMSample &a,
                           const VisDQMSample &b)
{
  // - Order by sample type (ascending).
  if (a.type < b.type)
    return true;
  else if (a.type > b.type)
    return false;

  // - Order by dataset (descending, natural string comparison).
  if (natcmpstr(a.dataset, b.dataset))
    return true;
  else if (natcmpstr(b.dataset, a.dataset))
    return false;

  // - Order by run (descending).
  if (a.runnr > b.runnr)
    return true;
  else if (a.runnr < b.runnr)
    return false;

  // - Order by version (descending, natural string comparison).
  if (natcmpstr(b.version, a.version))
    return true;
  else if (natcmpstr(a.version, b.version))
    return false;

  // Should not come here, unless the samples are indeed identical, in
  // which case we return false
  return false;
}

inline
bool orderSamplesByRun(const VisDQMSample &a,
                       const VisDQMSample &b)
{
  // - Order by sample type (ascending).
  if (a.type < b.type)
    return true;
  else if (a.type > b.type)
    return false;

  // - Order by run (descending).
  if (a.runnr > b.runnr)
    return true;
  else if (a.runnr < b.runnr)
    return false;

  // - Order by version (descending, natural string comparison).
  if (natcmpstr(b.version, a.version))
    return true;
  else if (natcmpstr(a.version, b.version))
    return false;

  // - Order by dataset (descending, natural string comparison).
  if (natcmpstr(a.dataset, b.dataset))
    return true;
  else if (natcmpstr(b.dataset, a.dataset))
    return false;

  // Should not come here, unless the samples are indeed identical, in
  // which case we return false
  return false;
}

#endif // DQM_VISDQMSERVERTOOLS_H
