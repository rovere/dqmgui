#ifndef DQM_UTILS_H
#define DQM_UTILS_H

#define VISDQM_NO_ROOT 1
#define DEBUG(n,x)

#include <pthread.h>

#include <iostream>
#include <ostream>
#include <map>
#include <string>

#include "classlib/utils/TimeInfo.h"
#include "classlib/utils/Regexp.h"
#include "classlib/utils/RegexpMatch.h"
#include "classlib/utils/StringOps.h"
#include "classlib/utils/StringFormat.h"

#include "boost/shared_ptr.hpp"
#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

#include "DQM/typedefs.h"
#include "DQM/Objects.h"
#include "DQM/NatSort.h"

using namespace lat;
namespace py = boost::python;

std::ostream & lograw(const char *label);
std::ostream & loginfo(void);
std::ostream & logerr(void);
std::ostream & logwarn(void);
std::string thousands(const std::string &arg);

void copyopts(const py::dict &opts,
                     std::map<std::string, std::string> &options);


/// Check whether the @a path is a subdirectory of @a ofdir.  Returns
/// true both for an exact match and any nested subdirectory.
bool isSubdirectory(const std::string &ofdir,
                           const std::string &path);

void splitPath(std::string &dir, std::string &name,
                      const std::string &path);

std::string stringToJSON(const std::string &x,
                                bool emptyIsNone = false);
std::string stringsToJSON(const StringAtomSet &overlays,
                                 bool emptyIsNone = false);

shared_ptr<Regexp> rx(const std::string &match,
                             int options = 0);
void makerx(const std::string &rxstr,
                   shared_ptr<Regexp> &rxobj,
                   std::string &rxerr,
                   int options = 0);

uint32_t getType(uint32_t flags);
bool isROOTType(uint32_t flags);
bool isScalarType(uint32_t flags);
bool isBLOBType(uint32_t flags);
bool isIntegerType(uint32_t flags);
bool isRealType(uint32_t flags);

void translateDrawOptions(py::dict opts,
                                 VisDQMDrawOptions &o);
std::string formatStartTime(long curStartTime);

void getEventInfoNum(const std::string &name,
                            const char *data,
                            VisDQMEventNumList &eventnums);
// Pick greatest run/lumi/event number combo seen.
void setEventInfoNums(const VisDQMEventNumList &eventnums,
                             VisDQMEventNum &current);

inline bool orderSamplesByDataset(const VisDQMSample &a, const VisDQMSample &b) {
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

inline bool orderSamplesByRun(const VisDQMSample &a, const VisDQMSample &b) {
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

std::string axisStatsToJSON(uint32_t nbins[3], double mean[3],
                                   double rms[3], double bounds[3][2], int axis);

// Format objects to json, with full data if requested.
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

void buildParentNames(StringAtomList &to, const StringAtom &from);

void fastrx(VisDQMRegexp &rxobj, const std::string &rxstr);
bool fastmatch(VisDQMRegexp *rx, const StringAtom &str);



#endif // DQM_UTILS_H
