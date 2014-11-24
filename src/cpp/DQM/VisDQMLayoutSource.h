#ifndef DQM_VISDQMLAYOUTSOURCE_H
#define DQM_VISDQMLAYOUTSOURCE_H

#include <pthread.h>

#include "classlib/utils/Regexp.h"
#include "classlib/utils/RegexpMatch.h"

#include "boost/python.hpp"
#include "boost/python/stl_iterator.hpp"

#include "DQM/VisDQMSource.h"
#include "DQM/Locks.h"
#include "DQM/utils.h"

namespace py = boost::python;

class VisDQMLayoutSource : public VisDQMSource {
  pthread_mutex_t       lock_;
  VisDQMItems           items_;

 public:
  VisDQMLayoutSource() {
    pthread_mutex_init(&lock_, 0);
  }

  ~VisDQMLayoutSource(void)
  {}

  virtual void scan(VisDQMItems &result,
                    const VisDQMSample & /* sample */,
                    VisDQMEventNum & /* current */,
                    VisDQMRegexp * /* rxmatch */,
                    Regexp *rxsearch,
                    bool *alarm,
                    std::string *layoutroot,
                    VisDQMRegexp *rxlayout);
  void pushLayouts(py::dict layouts);
};

#endif // DQM_VISDQMLAYOUTSOURCE_H
