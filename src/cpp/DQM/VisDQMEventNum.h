#ifndef DQM_VISDQMEVENTNUM_H
#define DQM_VISDQMEVENTNUM_H

#include <list>

struct VisDQMEventNum;
typedef std::list<VisDQMEventNum> VisDQMEventNumList;

struct VisDQMEventNum
{
  std::string subsystem;
  long        runnr;
  long        luminr;
  long        eventnr;
  long        runstart;
};


#endif // DQM_VISDQMEVENTNUM_H
