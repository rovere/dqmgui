#ifndef DQM_VISDQMSAMPLE_H
#define DQM_VISDQMSAMPLE_H

#include <stdint.h>

#include <vector>

class VisDQMSource;

struct VisDQMSample;
typedef std::vector<VisDQMSample> VisDQMSamples;

enum VisDQMSampleType
{
  SAMPLE_LIVE,
  SAMPLE_ONLINE_DATA,
  SAMPLE_OFFLINE_DATA,
  SAMPLE_OFFLINE_RELVAL,
  SAMPLE_OFFLINE_MC,
  SAMPLE_ANY
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

struct VisDQMSample
{
  VisDQMSampleType              type;
  long                          runnr;
  uint32_t                      importversion;
  std::string                   dataset;
  std::string                   version;
  VisDQMSource                  *origin;
  uint64_t                      time;
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


#endif // DQM_VISDQMSAMPLE_H
