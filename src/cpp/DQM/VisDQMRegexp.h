#ifndef DQM_VISDQMREGEXP_H
#define DQM_VISDQMREGEXP_H

#include <pthread.h>

#include <vector>
#include <map>
#include <string>

#include "classlib/utils/Regexp.h"

#include "boost/shared_ptr.hpp"

struct VisDQMRegexp;
typedef std::map<std::string, VisDQMRegexp> VisDQMRegexps;

struct VisDQMRegexp
{
  static const size_t REGEXPS_PER_LOCK = 1024;
  boost::shared_ptr<lat::Regexp> rx;
  std::vector<bool>              matched;
  std::vector<bool>              tried;
  std::vector<pthread_rwlock_t>  locks;
};

#endif // DQM_VISDQMREGEXP_H
