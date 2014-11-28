#ifndef DQM_VISDQMDRAWOPTIONS_H
#define DQM_VISDQMDRAWOPTIONS_H

#include <map>
#include <string>

#include "DQM/StringAtom.h"

struct VisDQMAxisOptions;
struct VisDQMDrawOptions;
typedef std::map<StringAtom, VisDQMDrawOptions> VisDQMDrawOptionMap;

struct VisDQMAxisOptions
{
  std::string min;
  std::string max;
  std::string type;
};

struct VisDQMDrawOptions
{
  VisDQMAxisOptions xaxis;
  VisDQMAxisOptions yaxis;
  VisDQMAxisOptions zaxis;
  std::string       drawopts;
  std::string       withref;
};


#endif // DQM_VISDQMDRAWOPTIONS_H
