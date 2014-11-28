#ifndef DQM_VISDQMSOURCEOBJECTS_H
#define DQM_VISDQMSOURCEOBJECTS_H

#include <vector>
#include <set>
#include <map>
#include <ext/hash_map>

#include "DQM/StringAtom.h"
#include "DQM/VisDQMDrawOptions.h"

#include "boost/shared_ptr.hpp"

typedef std::set<StringAtom> StringAtomSet;
typedef std::vector<StringAtom> StringAtomList;

namespace ext = __gnu_cxx;
namespace __gnu_cxx
{
  template<> struct hash<StringAtom>
  {
    size_t operator()(const StringAtom &s) const
      { return s.index(); }
  };

  template<> struct hash<std::string>
  {
    size_t operator()(const std::string &s) const
      { return __stl_hash_string(s.c_str()); }
  };
}

struct VisDQMLayoutItem;
struct VisDQMLayoutRow;
struct VisDQMItem;
struct VisDQMStatus;
struct VisDQMShownItem;
struct VisDQMShownRow;
struct VisDQMStatus;
class VisDQMSource;

typedef std::vector< boost::shared_ptr<VisDQMLayoutItem> >          VisDQMLayoutItems;
typedef std::vector< boost::shared_ptr<VisDQMLayoutRow> >           VisDQMLayoutRows;
typedef std::vector< boost::shared_ptr<VisDQMShownItem> >           VisDQMShownItems;
typedef std::vector< boost::shared_ptr<VisDQMShownRow> >            VisDQMShownRows;
typedef ext::hash_map<StringAtom, boost::shared_ptr<VisDQMItem> >   VisDQMItems;
typedef ext::hash_map<StringAtom, VisDQMStatus>                     VisDQMStatusMap;

struct VisDQMLayoutItem
{
  StringAtom                    path;
  std::string                   desc;
  VisDQMDrawOptions             drawopts;
  StringAtomSet                 overlays;
};

struct VisDQMLayoutRow
{
  VisDQMLayoutItems             columns;
};

struct VisDQMItem
{
  uint32_t                      flags;
  uint64_t                      version;
  StringAtom                    name;
  StringAtomList                parents;
  std::string                   data;
  VisDQMSource                  *plotter;
  boost::shared_ptr<VisDQMLayoutRows>  layout;
};

struct VisDQMShownItem
{
  uint64_t                      version;
  int                           nalarm;
  int                           nlive;
  StringAtom                    name;
  std::string                   desc;
  VisDQMSource                  *plotter;
  VisDQMDrawOptions             drawopts;
  StringAtomSet                 overlays;
};

struct VisDQMShownRow
{
  VisDQMShownItems              columns;
};

struct VisDQMStatus
{
  int                           nleaves;
  int                           nalarm;
  int                           nlive;
};



#endif // DQM_VISDQMSOURCEOBJECTS_H
