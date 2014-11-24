#ifndef DQM_TYPEDEFS_H
#define DQM_TYPEDEFS_H

#include <ext/hash_map>

#include <utility>
#include <set>
#include <vector>
#include <map>
#include <list>
#include <string>

#include "boost/shared_ptr.hpp"

#include "classlib/utils/Regexp.h"

#include "DQM/StringAtom.h"
#include "DQM/VisDQMIndex.h"

using namespace boost;
using namespace lat;

namespace __gnu_cxx {
  template<> struct hash<StringAtom> {
    size_t operator()(const StringAtom &s) const
      { return s.index(); }
  };

  template<> struct hash<std::string> {
    size_t operator()(const std::string &s) const
      { return __stl_hash_string(s.c_str()); }
  };
}

namespace ext = __gnu_cxx;

struct VisDQMAxisOptions;
struct VisDQMDrawOptions;
struct VisDQMLayoutItem;
struct VisDQMLayoutRow;
struct VisDQMItem;
struct VisDQMStatus;
struct VisDQMShownItem;
struct VisDQMShownRow;
struct VisDQMStatus;
struct VisDQMEventNum;
struct VisDQMSample;
struct VisDQMRegexp;
class VisDQMSource;

typedef std::pair<std::string, std::string>  StringPair;
typedef std::set<StringAtom>                 StringAtomSet;
typedef std::vector<StringAtom>              StringAtomList;
typedef std::map<StringAtom, StringAtomList> StringAtomParentList;

typedef std::vector< shared_ptr<VisDQMLayoutItem> >        VisDQMLayoutItems;
typedef std::vector< shared_ptr<VisDQMLayoutRow> >         VisDQMLayoutRows;
typedef std::vector< shared_ptr<VisDQMShownItem> >         VisDQMShownItems;
typedef std::vector< shared_ptr<VisDQMShownRow> >          VisDQMShownRows;
typedef ext::hash_map<StringAtom, shared_ptr<VisDQMItem> > VisDQMItems;
typedef ext::hash_map<StringAtom, VisDQMStatus>            VisDQMStatusMap;
typedef std::map<StringAtom, VisDQMDrawOptions>            VisDQMDrawOptionMap;
typedef std::list<VisDQMEventNum>                          VisDQMEventNumList;
typedef std::vector<VisDQMSample>                          VisDQMSamples;
typedef std::map<std::string, VisDQMRegexp>                VisDQMRegexps;

struct VisDQMAxisOptions
{
  std::string			min;
  std::string			max;
  std::string			type;
};

struct VisDQMDrawOptions
{
  VisDQMAxisOptions		xaxis;
  VisDQMAxisOptions		yaxis;
  VisDQMAxisOptions		zaxis;
  std::string			drawopts;
  std::string			withref;
};

struct VisDQMLayoutItem
{
  StringAtom			path;
  std::string			desc;
  VisDQMDrawOptions		drawopts;
  StringAtomSet                 overlays;
};

struct VisDQMLayoutRow
{
  VisDQMLayoutItems		columns;
};

struct VisDQMItem
{
  uint32_t			flags;
  uint64_t			version;
  StringAtom			name;
  StringAtomList		parents;
  std::string			data;
  VisDQMSource		 	*plotter;
  shared_ptr<VisDQMLayoutRows>	layout;
};

struct VisDQMShownItem
{
  uint64_t			version;
  int				nalarm;
  int				nlive;
  StringAtom			name;
  std::string			desc;
  VisDQMSource			*plotter;
  VisDQMDrawOptions		drawopts;
  StringAtomSet                 overlays;
};

struct VisDQMShownRow
{
  VisDQMShownItems		columns;
};

struct VisDQMStatus
{
  int				nleaves;
  int				nalarm;
  int				nlive;
};

struct VisDQMEventNum
{
  std::string			subsystem;
  long				runnr;
  long				luminr;
  long				eventnr;
  long				runstart;
};

enum VisDQMSampleType
{
  SAMPLE_LIVE,
  SAMPLE_ONLINE_DATA,
  SAMPLE_OFFLINE_DATA,
  SAMPLE_OFFLINE_RELVAL,
  SAMPLE_OFFLINE_MC,
  SAMPLE_ANY
};

struct VisDQMSample
{
  VisDQMSampleType		type;
  long				runnr;
  uint32_t			importversion;
  std::string			dataset;
  std::string			version;
  VisDQMSource			*origin;
  uint64_t			time;
  VisDQMSample(VisDQMSampleType t, long r, const std::string & d)
      : type(t),
        runnr(r),
        importversion(0),
        dataset(d),
        version(""),
        origin(0),
        time(0)
  {}
  VisDQMSample(VisDQMSampleType t, long r)
      : type(t),
        runnr(r),
        importversion(0),
        dataset(""),
        version(""),
        origin(0),
        time(0)
  {}
  VisDQMSample()
  {
    VisDQMSample(SAMPLE_ANY, 0);
  }
};

static const char *sampleTypeLabel[] =
{
  "live",
  "online_data",
  "offline_data",
  "offline_relval",
  "offline_mc",
  "any"
};

struct VisDQMRegexp
{
  static const size_t REGEXPS_PER_LOCK = 1024;
  shared_ptr<Regexp>		rx;
  std::vector<bool>		matched;
  std::vector<bool>		tried;
  std::vector<pthread_rwlock_t>	locks;
};


#endif // DQM_TYPEDEFS_H
