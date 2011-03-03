#ifndef DQM_OBJECTS_H
# define DQM_OBJECTS_H

# include "DQM/DQMNet.h"

class TObject;

static const uint32_t DQM_PROP_STRIP_AXIS_TIME  = 0x10000000;
static const uint32_t DQM_PROP_STRIP_SKIP_EMPTY = 0x20000000;

struct VisDQMRenderInfo
{
  std::string		drawOptions;
  bool			blacklisted;
};

struct VisDQMAxisInfo
{
  std::string		type;
  double		min;
  double		max;
};

enum VisDQMReference
{
  DQM_REF_OBJECT,
  DQM_REF_OVERLAY,
  DQM_REF_REFERENCE
};

enum VisDQMStripTrend
{
  DQM_TREND_OBJECT,
  DQM_TREND_NUM_ENTRIES,
  DQM_TREND_NUM_BINS,
  DQM_TREND_NUM_BYTES,
  DQM_TREND_X_MEAN,
  DQM_TREND_X_MIN,
  DQM_TREND_X_MAX,
  DQM_TREND_X_MEAN_RMS,
  DQM_TREND_X_MEAN_MIN_MAX,
  DQM_TREND_X_NUM_BINS,
  DQM_TREND_Y_MEAN,
  DQM_TREND_Y_MIN,
  DQM_TREND_Y_MAX,
  DQM_TREND_Y_MEAN_RMS,
  DQM_TREND_Y_MEAN_MIN_MAX,
  DQM_TREND_Y_NUM_BINS,
  DQM_TREND_Z_MEAN,
  DQM_TREND_Z_MIN,
  DQM_TREND_Z_MAX,
  DQM_TREND_Z_MEAN_RMS,
  DQM_TREND_Z_MEAN_MIN_MAX,
  DQM_TREND_Z_NUM_BINS
};

struct VisDQMImgInfo
{
  std::string		imgspec;
  std::string		drawOptions;
  VisDQMReference	reference;
  VisDQMStripTrend	trend;
  int			width;
  int			height;
  VisDQMAxisInfo	xaxis;
  VisDQMAxisInfo	yaxis;
  VisDQMAxisInfo	zaxis;
};

struct VisDQMObject : DQMNet::Object
{
  std::string		name;
  TObject		*object;
  TObject		*reference;
  VisDQMImgInfo		image;
};

#endif // DQM_OBJECTS_H
