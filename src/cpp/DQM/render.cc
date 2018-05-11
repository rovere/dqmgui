#define DEBUG(n,x)

#include "DQM/DQMRenderPlugin.h"
#include "DQM/VisDQMRenderTools.h"
#include "DQM/VisDQMTools.h"
#include "DQM/VisDQMIndex.h"
#include "DQM/Objects.h"
#include "DQM/DQMNet.h"
#include "classlib/iobase/Filename.h"
#include "classlib/utils/Regexp.h"
#include "classlib/utils/Signal.h"
#include "classlib/utils/Time.h"
#include "classlib/utils/TimeInfo.h"
#include "classlib/utils/SystemError.h"
#include "classlib/utils/SharedLibrary.h"
#include "classlib/utils/StringFormat.h"
#include "classlib/utils/StringList.h"
#include "classlib/utils/StringOps.h"
#include "classlib/utils/DebugAids.h"
#include "classlib/utils/Error.h"
#include "boost/algorithm/string.hpp"
#include "TObjString.h"
#include "TDirectory.h"
#include "TImageDump.h"
#include "TASImage.h"
#include "TCanvas.h"
#include "TText.h"
#include "TH1D.h"
#include "TH2.h"
#include "TH3.h"
#include "TProfile.h"
#include "TColor.h"
#include "TAxis.h"
#include "TList.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TPaveStats.h"
#include "TProfile2D.h"
#include "THStack.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <cerrno>
#include <map>
#include <algorithm>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <math.h>
#include <string>
#include <limits>

using namespace lat;

static int colors[] = {kGray, kAzure, kOrange+7, kRed+1,
                       kMagenta+2, kGreen-3};

// ----------------------------------------------------------------------
static void
stripNameToLabel(std::string &label, const std::string &name)
{
  size_t slash = name.rfind('/');
  size_t namepart = (slash == std::string::npos ? 0 : slash+1);
  size_t len = name.size() - namepart;
  label.reserve(len + 1);
  if (len < 20)
    label.append(name, namepart, len);
  else
  {
    label.append(name, namepart, 12);
    label.append("...");
    label.append(name, name.size()-5, 5);
  }
}

// ----------------------------------------------------------------------
// Convert the canvas into a PNG image.
class DQMImageDump : public TImageDump
{
public:
  DQMImageDump(TVirtualPad *pad, DQMNet::DataBlob &imgdata)
    {
      fImage = new TASImage;
      fType = 114;
      SetName(pad->GetName());
      SetBit(BIT(11));

      pad->Update();
      pad->Paint();

      UInt_t width = fImage->GetWidth();
      UInt_t height = fImage->GetHeight();
      imgdata.resize(width * height * sizeof(uint8_t) * 3);
      uint32_t *argb = (uint32_t *) fImage->GetArgbArray();
      uint8_t *rgb = (uint8_t *) &imgdata[0];
      for (UInt_t row = 0, idx = 0; row < height; ++row)
	for (UInt_t col = 0; col < width; ++col, ++idx)
	{
	  uint32_t val = argb[idx];
	  rgb[idx*3+0] = (val >> 16) & 0xff;
	  rgb[idx*3+1] = (val >>  8) & 0xff;
	  rgb[idx*3+2] = (val >>  0) & 0xff;
	}
    }
};

// ----------------------------------------------------------------------
// Parse an integer parameter in an image spec.  If @ap starts with
// the parameter prefix @a name of length @a len, extracts the number
// that follows the prefix into @a value and advances @a p to the next
// character after the extracted value.  Returns true if the parameter
// was parsed, false otherwise.
static bool
parseInt(const char *&p, const char *name, size_t len, int &value)
{
  if (! strncmp(p, name, len))
  {
    value = strtol(p+len, (char **) &p, 10);
    return true;
  }
  return false;
}

// Parse a double parameter in an image spec, like parseInt().
static bool
parseDouble(const char *&p, const char *name, size_t len, double &value)
{
  if (! strncmp(p, name, len))
  {
    value = strtod(p+len, (char **) &p);
    return true;
  }
  return false;
}

// Parse reference type parameter in an image spec, like parseInt().
static bool
parseReference(const char *&p, const char *name, size_t len, VisDQMReference &value)
{
  if (! strncmp(p, name, len))
  {
    p += len;
    if (! strncmp(p, "object", 6))
    {
      value = DQM_REF_OBJECT;
      p += 6;
      return true;
    }
    else if (! strncmp(p, "reference", 9))
    {
      value = DQM_REF_REFERENCE;
      p += 9;
      return true;
    }
    else if (! strncmp(p, "overlay", 7))
    {
      value = DQM_REF_OVERLAY;
      p += 7;
      return true;
    }
    else if (! strncmp(p, "ratiooverlay", 12))
    {
      value = DQM_REF_OVERLAY_RATIO;
      p += 12;
      return true;
    }
    else if (! strncmp(p, "samesample", 10))
    {
      value = DQM_REF_SAMESAMPLE;
      p += 10;
      return true;
    }
    else if (! strncmp(p, "stacked", 7))
    {
      value = DQM_REF_STACKED;
      p += 7;
      return true;
    }
  }
  return false;
}

// Parse strip chart trend type parameter in an image spec, like parseInt().
static bool
parseStripChart(const char *&p, const char *name, size_t len, VisDQMStripTrend &value)
{
  if (! strncmp(p, name, len))
  {
    p += len;
    if (! strncmp(p, "object", 6))
    {
      value = DQM_TREND_OBJECT;
      p += 6;
      return true;
    }
    else if (! strncmp(p, "num-entries", 11))
    {
      value = DQM_TREND_NUM_ENTRIES;
      p += 11;
      return true;
    }
    else if (! strncmp(p, "num-bins", 8))
    {
      value = DQM_TREND_NUM_BINS;
      p += 8;
      return true;
    }
    else if (! strncmp(p, "num-bytes", 9))
    {
      value = DQM_TREND_NUM_BYTES;
      p += 9;
      return true;
    }
    else if (! strncmp(p, "value", 5))
    {
      value = DQM_TREND_LS_VALUE;
      p += 5;
      return true;
    }
    else if ((p[0] == 'x' || *p == 'y' || *p == 'z') && p[1] == '-')
    {
      int offset = (p[0] == 'z' ? DQM_TREND_Z_MEAN - DQM_TREND_X_MEAN
		    : p[0] == 'y' ? DQM_TREND_Y_MEAN - DQM_TREND_X_MEAN
		    : 0);

      if (! strncmp(p+2, "mean-rms", 8))
      {
	value = (VisDQMStripTrend) (offset + DQM_TREND_X_MEAN_RMS);
	p += 10;
	return true;
      }
      else if (! strncmp(p+2, "mean-min-max", 12))
      {
	value = (VisDQMStripTrend) (offset + DQM_TREND_X_MEAN_MIN_MAX);
	p += 14;
	return true;
      }
      else if (! strncmp(p+2, "mean", 4))
      {
	value = (VisDQMStripTrend) (offset + DQM_TREND_X_MEAN);
	p += 6;
	return true;
      }
      else if (! strncmp(p+2, "min", 3))
      {
	value = (VisDQMStripTrend) (offset + DQM_TREND_X_MIN);
	p += 5;
	return true;
      }
      else if (! strncmp(p+2, "max", 3))
      {
	value = (VisDQMStripTrend) (offset + DQM_TREND_X_MAX);
	p += 5;
	return true;
      }
      else if (! strncmp(p+2, "bins", 4))
      {
	value = (VisDQMStripTrend) (offset + DQM_TREND_X_NUM_BINS);
	p += 6;
	return true;
      }
    }
  }
  return false;
}

// Parse an axis type parameter in an image spec, like parseInt().
static bool
parseAxisType(const char *&p, const char *name, size_t len, std::string &value)
{
  if (! strncmp(p, name, len))
  {
    p += len;
    if (! strncmp(p, "def", 3)
	|| ! strncmp(p, "lin", 3)
	|| ! strncmp(p, "log", 3))
    {
      value.clear();
      value.insert(value.end(), p, p+3);
      p += 3;
      return true;
    }
  }
  return false;
}

// Parse ROOT draw options parameter in an image spec, like parseInt().
static bool
parseOption(const char *&p, const char *name, size_t len, std::string &value)
{
  static const char SAFE [] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"01234567890 "  };
  if (! strncmp(p, name, len))
  {
    p += len;
    len = 0;
    while (p[len] && strchr(SAFE, p[len]))
      ++len;
    value.insert(value.end(), p, p+len);
    p += len;
    return true;
  }
  return false;
}

// Build a parsed image specification for an object image.  Returns
// already parsed image data if it exists, otherwise breaks the data
// into its components and adds a new empty image into the object.
// The image specification is expected to be a series of "key=value"
// pairs separated by semicolons.
static bool
parseImageSpec(VisDQMImgInfo &i, const std::string &spec, const char *&error)
{
  error = 0;
  i.imgspec = spec;
  i.width = -1;
  i.height = -1;
  i.reference = DQM_REF_OBJECT;
  i.showstats = 1;
  i.showerrbars = 0;
  i.trend = DQM_TREND_OBJECT;
  i.ktest = NAN;
  i.xaxis.type = i.yaxis.type = i.zaxis.type = "def";
  i.xaxis.min  = i.yaxis.min  = i.zaxis.min  = NAN;
  i.xaxis.max  = i.yaxis.max  = i.zaxis.max  = NAN;

  const char *p = spec.c_str();
  while (*p)
  {
    // Skip separators.
    while (*p == ';')
      ++p;

    // Handle various keywords.
    error = "unexpected image parameter";
    if (!    parseInt       (p, "w=",            2, i.width)
	&& ! parseInt       (p, "h=",            2, i.height)
	&& ! parseReference (p, "ref=",          4, i.reference)
	&& ! parseInt       (p, "showstats=",   10, i.showstats)
	&& ! parseInt       (p, "showerrbars=", 12, i.showerrbars)
	&& ! parseStripChart(p, "trend=",        6, i.trend)
	&& ! parseDouble    (p, "xmin=",         5, i.xaxis.min)
	&& ! parseDouble    (p, "xmax=",         5, i.xaxis.max)
	&& ! parseAxisType  (p, "xtype=",        6, i.xaxis.type)
	&& ! parseDouble    (p, "ymin=",         5, i.yaxis.min)
	&& ! parseDouble    (p, "ymax=",         5, i.yaxis.max)
	&& ! parseAxisType  (p, "ytype=",        6, i.yaxis.type)
	&& ! parseDouble    (p, "zmin=",         5, i.zaxis.min)
	&& ! parseDouble    (p, "zmax=",         5, i.zaxis.max)
	&& ! parseDouble    (p, "ktest=",        6, i.ktest)
	&& ! parseAxisType  (p, "ztype=",        6, i.zaxis.type)
	&& ! parseOption    (p, "drawopts=",     9, i.drawOptions)
	&& ! parseOption    (p, "norm=",         5, i.refnorm)
        && ! parseOption    (p, "reflabel1=",   10, i.reflabel1)
        && ! parseOption    (p, "reflabel2=",   10, i.reflabel2)
        && ! parseOption    (p, "reflabel3=",   10, i.reflabel3)
        && ! parseOption    (p, "reflabel4=",   10, i.reflabel4)
        )
      return false;

    if (*p && *p != ';')
    {
      error = "unexpected characters in parameter value";
      return false;
    }
  }

  if (i.width < 0 || i.height < 0)
  {
    error = "image dimensions missing, zero or negative";
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------
class TStripChart : public TObject
{
  TH1D *bins_;
  TGraph *graph_;
  uint32_t flags_;
public:
  TStripChart(TH1D *bins, TGraph *graph, uint32_t flags)
    : bins_(bins), graph_(graph), flags_(flags)
    {}

  ~TStripChart(void)
    {
      delete bins_;
      delete graph_;
    }

  virtual void
  Draw(const char *opt)
    {
      if (bins_)
	bins_->Draw(opt);
      if (graph_)
	graph_->Draw("P|>");

      if (flags_ & DQM_PROP_STRIP_AXIS_TIME)
	gPad->SetBottomMargin(0.25);
    }
};

TObject *
makeStripChart(VisDQMImgInfo &info,
	       uint32_t flags,
	       const std::string &title,
	       std::vector<double> &axisvals,
	       std::vector<VisDQMIndex::Summary> &attrs,
	       std::vector<std::string> &labels,
	       VisDQMObject *objs,
	       uint32_t numobjs)
{
  std::string titlepfx;
  const char *axispfx[] = { "X ", "Y ", "Z ", "" };
  size_t axis = (info.trend >= DQM_TREND_X_MEAN && info.trend <= DQM_TREND_X_NUM_BINS ? 0
		 : info.trend >= DQM_TREND_Y_MEAN && info.trend <= DQM_TREND_Y_NUM_BINS ? 1
		 : info.trend >= DQM_TREND_Z_MEAN && info.trend <= DQM_TREND_Z_NUM_BINS ? 2
		 : 3);
  TH1 *extra = (numobjs > 1 ? dynamic_cast<TH1 *>(objs[1].object) : 0);
  size_t nextra = extra ? 1 : 0;
  size_t nattrs = attrs.size();
  std::vector<double> vals(nattrs);
  std::vector<double> yerr;
  std::vector<double> ylo;
  std::vector<double> yhi;
  size_t nattrsmin = 0;
  size_t realsize = nattrs;

  switch (info.trend)
  {
  case DQM_TREND_OBJECT:
    assert(false);
    break;

  case DQM_TREND_LS_VALUE:
    titlepfx = "Values for ";
    for (size_t i = nextra; i != nattrs; ++i)
    {
      if (! (isnan(info.xaxis.max)) && (attrs[i].mean[0] > info.xaxis.max))
      {
	realsize--;
	continue;
      }
      if (! (isnan(info.xaxis.min)) && (attrs[i].mean[0] < info.xaxis.min))
      {
	nattrsmin++;
	continue;
      }
      vals[i] = attrs[i].mean[1];
    }
    realsize -= nattrsmin;
    break;

  case DQM_TREND_NUM_ENTRIES:
    titlepfx = "Entries in ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].nentries;
    if (extra)
      vals[nattrs-1] = extra->GetEntries();
    break;

  case DQM_TREND_NUM_BINS:
    titlepfx = "Number of Bins in ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].nbins[0] * attrs[i].nbins[1] * attrs[i].nbins[2];
    if (extra)
      vals[nattrs-1] = extra->GetNbinsX() * extra->GetNbinsY() * extra->GetNbinsZ();
    break;

  case DQM_TREND_NUM_BYTES:
    titlepfx = "Number of Bytes in ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].objectLength;
    if (extra)
      vals[nattrs-1] = objs[1].rawdata.size();
    break;

  case DQM_TREND_X_MEAN:
  case DQM_TREND_Y_MEAN:
  case DQM_TREND_Z_MEAN:
    titlepfx = "Mean of ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].mean[axis];
    if (extra)
      vals[nattrs-1] = extra->GetMean(axis+1);
    break;

  case DQM_TREND_X_MIN:
  case DQM_TREND_Y_MIN:
  case DQM_TREND_Z_MIN:
    titlepfx = "Minimum of ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].bounds[axis][0];
    if (extra)
      if (TAxis *a = (axis == 0 ? extra->GetXaxis()
		      : axis == 1 ? extra->GetYaxis()
		      : axis == 2 ? extra->GetZaxis() : 0))
	vals[nattrs-1] = a->GetXmin();
    break;

  case DQM_TREND_X_MAX:
  case DQM_TREND_Y_MAX:
  case DQM_TREND_Z_MAX:
    titlepfx = "Maximum of ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].bounds[axis][1];
    if (extra)
      if (TAxis *a = (axis == 0 ? extra->GetXaxis()
		      : axis == 1 ? extra->GetYaxis()
		      : axis == 2 ? extra->GetZaxis() : 0))
	vals[nattrs-1] = a->GetXmax();
    break;

  case DQM_TREND_X_MEAN_RMS:
  case DQM_TREND_Y_MEAN_RMS:
  case DQM_TREND_Z_MEAN_RMS:
    titlepfx = "Mean and RMS of ";
    yerr.resize(attrs.size());
    for (size_t i = nextra; i != nattrs; ++i)
    {
      vals[nattrs-i-1] = attrs[i].mean[axis];
      yerr[nattrs-i-1] = attrs[i].rms[axis];
    }
    if (extra)
    {
      vals[nattrs-1] = extra->GetMean(axis+1);
      yerr[nattrs-1] = extra->GetRMS(axis+1);
    }
    break;

  case DQM_TREND_X_MEAN_MIN_MAX:
  case DQM_TREND_Y_MEAN_MIN_MAX:
  case DQM_TREND_Z_MEAN_MIN_MAX:
    titlepfx = "Mean and Min-Max of ";
    ylo.resize(attrs.size());
    yhi.resize(attrs.size());
    for (size_t i = nextra; i != nattrs; ++i)
    {
      vals[nattrs-i-1] = attrs[i].mean[axis];
      ylo[nattrs-i-1] = vals[nattrs-i-1] - attrs[i].bounds[axis][0];
      yhi[nattrs-i-1] = attrs[i].bounds[axis][1] - vals[nattrs-i-1];
    }
    if (extra)
    {
      vals[nattrs-1] = extra->GetMean(axis+1);
      if (TAxis *a = (axis == 0 ? extra->GetXaxis()
		      : axis == 1 ? extra->GetYaxis()
		      : axis == 2 ? extra->GetZaxis() : 0))
      {
	ylo[nattrs-1] = vals[nattrs-1] - a->GetXmin();
	yhi[nattrs-1] = a->GetXmax() - vals[nattrs-1];
      }
    }
    break;

  case DQM_TREND_X_NUM_BINS:
  case DQM_TREND_Y_NUM_BINS:
  case DQM_TREND_Z_NUM_BINS:
    titlepfx = "Number of Bins in ";
    for (size_t i = nextra; i != nattrs; ++i)
      vals[nattrs-i-1] = attrs[i].nbins[axis];
    if (extra)
      vals[nattrs-1] = (axis == 0 ? extra->GetNbinsX()
			: axis == 1 ? extra->GetNbinsY()
			: axis == 2 ? extra->GetNbinsZ() : 0);
    break;
  }

  // Produce the full title.
  std::string fulltitle = axispfx[axis] + titlepfx + title;
  if (flags & DQM_PROP_STRIP_SKIP_EMPTY)
    fulltitle = "#splitline{" + fulltitle
		+ "}{(statistics for empty histograms omitted)}";

  // ROOT needs one more bin than there are axis value. For time
  // add one hour, for binning add just one entry.
  if (flags & DQM_PROP_STRIP_AXIS_TIME)
  {
    std::reverse(axisvals.begin(), axisvals.end());
    axisvals.push_back(axisvals.back() + 3600);
  }
  else
    axisvals.push_back(axisvals.back() + 1);

  // Construct the histogram. We use it only for the bins.
  double vmax = vals.size() > 0 ? -DBL_MAX : 0;
  double vmin = vals.size() > 0 ? DBL_MAX : 0;
  size_t skip = (info.trend != DQM_TREND_LS_VALUE) ? vals.size() / 30 + 1 : 1;
  TH1D *bins = new TH1D("Trend", fulltitle.c_str(), realsize, &axisvals[nattrsmin]);
  TAxis *xaxis = bins->GetXaxis();
  for (size_t i = 0, e = vals.size(); i != e; ++i)
  {
    vmax = std::max(vmax, vals[i]);
    vmin = std::min(vmin, vals[i]);
    if (! yerr.empty())
    {
      vmax = std::max(vmax, vals[i] + yerr[i]);
      vmin = std::min(vmin, vals[i] - yerr[i]);
    }
    if (! yhi.empty())
    {
      vmax = std::max(vmax, vals[i] + yhi[i]);
      vmin = std::min(vmin, vals[i] - ylo[i]);
     }

    // swap label ordering, since DQM_TREND_LS_VALUE is increasing, while
    // in all other cases the order is reversed (older runs comes
    // first).
    size_t labelIndex = (info.trend == DQM_TREND_LS_VALUE) ? nattrsmin+i : (nattrs-i-1);
    if (i % skip == 0)
      if (i < realsize)
	xaxis->SetBinLabel(i+1, labels[labelIndex].c_str());

    axisvals[i] = (axisvals[i] + axisvals[i+1])/2;
  }
  bins->SetStats(kFALSE);
  bins->LabelsOption("v");
  bins->SetMaximum(vmax >= 0 ? 1.1*vmax : 0.9*vmax);
  bins->SetMinimum(vmin >= 0 ? 0.9*vmin : 1.1*vmin);
  //xaxis->CenterLabels(kFALSE);
  //xaxis->SetNdivisions(530);

  // Construct a graph of data points.
  TGraph *graph = 0;
  if (yerr.empty() && yhi.empty())
    graph = new TGraph(realsize, &axisvals[nattrsmin], &vals[nattrsmin]);
  else if (! yerr.empty())
    graph = new TGraphErrors(realsize, &axisvals[nattrsmin], &vals[nattrsmin], 0, &yerr[nattrsmin]);
  else if (! yhi.empty())
    graph = new TGraphAsymmErrors(realsize, &axisvals[nattrsmin], &vals[nattrsmin],
				  0, 0, &ylo[nattrsmin], &yhi[nattrsmin]);
  graph->SetMarkerColor(46);
  graph->SetMarkerStyle(20);
  graph->SetMarkerSize(1.1);
  return new TStripChart(bins, graph, flags);
}

// ----------------------------------------------------------------------
class VisDQMImageServer : public DQMImplNet<VisDQMObject>
{
public:
  static const uint32_t DQM_MSG_GET_IMAGE_DATA	= 4;
  static const uint32_t DQM_MSG_DUMP_PROFILE	= 5;
  static const uint32_t DQM_MSG_GET_JSON_DATA   = 6;
  static const uint32_t DQM_MSG_GET_JSROOT_DATA	= 7;
  static const uint32_t DQM_REPLY_IMAGE_DATA	= 105;
  static const uint32_t DQM_REPLY_JSON_DATA 	= 106;
  static const uint32_t DQM_REPLY_JSROOT_DATA   = 107;

  typedef std::vector<DQMRenderPlugin *> RenderPlugins;
  typedef std::map<std::string, lat::Time> BlackList;
  struct ServerInfo
  {
    int			argc;
    char		**argv;
    bool		debug;
    lat::Filename	dynobj;
    lat::Filename	statedir;
    lat::Filename	sockpath;
    lat::Filename	stopfile;
    BlackList		blacklist;
  };

private:
  void			(*profdump_)(const char *path);
  RenderPlugins		plugins_;
  std::vector<bool>	applied_;
  TStyle		*standardStyle_;
  ServerInfo		info_;

public:
  VisDQMImageServer(const ServerInfo &info)
    : DQMImplNet<VisDQMObject>(info.argv[0]),
      profdump_((void(*)(const char *)) dlsym(0, "igprof_dump_now")),
      standardStyle_(0),
      info_(info)
    {
      // Prevent histograms from getting attached to the global directory.
      TH1::AddDirectory(kFALSE);

      // Capture unchangeable standard ROOT style.
      gStyle->Reset();
      gStyle->SetFillColor(10);
      gStyle->SetCanvasColor(10);
      gStyle->SetPadColor(10);
      gStyle->SetTitleFillColor(10);
      gStyle->SetTitleBorderSize(0);
      gStyle->SetOptStat(1111);
      gStyle->SetStatColor(10);
      gStyle->SetStatBorderSize(1);
      gStyle->SetNumberContours(100);
      gStyle->SetFrameFillColor(10);
      gStyle->SetLabelSize(0.04);
      standardStyle_ = (TStyle *) gStyle->Clone();
      gROOT->ForceStyle();

      // Indicate we've started.
      logme()
	<< "INFO: DQM image server started in path "
	<< info_.statedir << std::endl;

      for (BlackList::const_iterator
	     i = info.blacklist.begin(),
	     e = info.blacklist.end();
	   i != e; ++i)
        logme()
	  << "INFO: black-listed '" << i->first << "' since "
	  << i->second.format(true, "%Y-%m-%d %H:%M:%S.")
	  << i->second.nanoformat(3, 3)
	  << " (unix time " << i->second << ")\n";

      // Initialise plugins now.
      initPlugins();

      // Establish the network server side.
      debug(info_.debug);
      delay(100);
      startLocalServer(info_.sockpath);
    }

  ~VisDQMImageServer(void)
    {
      shutPlugins();
    }

protected:
  // Check if the server should stop.
  virtual bool
  shouldStop(void)
    {
      return info_.stopfile.exists();
    }

  // Release objects from wait.  This cannot ever happen because this
  // server is never connected to a network data source which it would
  // have to wait for.
  virtual void
  releaseFromWait(Bucket * /* msg */, WaitObject & /* w */, Object * /* o */)
    {
      ASSERT(false);
    }

  // Web server facing message handling.  Respond to image requests.
  virtual bool
  onMessage(Bucket *msg, Peer *p, unsigned char *data, size_t len)
    {
      // Decode and process this message.
      uint32_t type;
      uint32_t nwords = 2;
      uint32_t flags;
      uint32_t tag;
      uint32_t vlow;
      uint32_t vhigh;
      uint32_t numobjs;
      uint32_t namelen;
      uint32_t speclen;
      uint32_t datalen;
      uint32_t qlen;
      uint32_t gotlen;

      memcpy (&type, data + sizeof(uint32_t), sizeof (type));
      if (type == DQM_MSG_DUMP_PROFILE)
      {
	if (profdump_)
	{
          char fname[128];
	  sprintf(fname, "|gzip -9c>igprof.visDQMRender.%d.%u.gz",
		  (int) getpid(), (int) time(0));
	  profdump_(fname);
	}
	return true;
      }
      else if (type == DQM_MSG_GET_IMAGE_DATA || type == DQM_MSG_GET_JSON_DATA || type == DQM_MSG_GET_JSROOT_DATA)
      {
        Time start = Time::current();
	if (debug_)
	  logme()
	    << "DEBUG: received message 'GET IMAGE DATA' from peer "
	    << p->peeraddr << std::endl;

	if (len < 9*sizeof(uint32_t))
	{
	  logme()
	    << "ERROR: corrupt 'GET IMAGE DATA' message of length " << len
	    << " from peer " << p->peeraddr << std::endl;
	  return false;
	}

	memcpy(&flags,   data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&tag,     data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&vlow,    data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&vhigh,   data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&numobjs, data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&namelen, data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&speclen, data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&datalen, data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&qlen,    data + nwords++ * sizeof(uint32_t), sizeof(uint32_t));
        gotlen = nwords*sizeof(uint32_t) + namelen + speclen + datalen + qlen;

	if (len != gotlen)
	{
	  logme()
	    << "ERROR: corrupt 'GET IMAGE DATA' message of length " << len
	    << " from peer " << p->peeraddr
	    << ", expected length " << (nwords*sizeof(uint32_t))
	    << " + " << namelen << " + " << speclen
	    << " + " << datalen << " + " << qlen
	    << " = " << gotlen << std::endl;
	  return false;
	}

        if (numobjs < 1 || numobjs > 10)
	{
	  logme() << "ERROR: 'GET IMAGE DATA' with <1 or >10 parts\n";
	  return false;
	}

        const char *part = (const char *) data + nwords*sizeof(uint32_t);
	std::string name (part, namelen); part += namelen;
	std::string spec (part, speclen); part += speclen;
	std::string odata(part, datalen); part += datalen;
	std::string qdata(part, qlen);    part += qlen;

        size_t slash = name.rfind('/');
        size_t dirpos = (slash == std::string::npos ? 0 : slash);
        size_t namepos = (slash == std::string::npos ? 0 : slash+1);
        std::string dirpart(name, 0, dirpos);

	// Validate the image request.
	VisDQMImgInfo info;
	const char *error = 0;

	if (type == DQM_MSG_GET_IMAGE_DATA && ! parseImageSpec(info, spec, error))
	{
	  logme()
	    << "ERROR: invalid image specification '"
	    << spec << "' for object '" << name
	    << "': " << error << std::endl;
	  return false;
	}

        // Reconstruct the object, defaulting to missing in action.
	std::vector<VisDQMObject> objs(numobjs);
        for (uint32_t i = 0; i < numobjs; ++i)
        {
	  objs[i].flags = flags;
          objs[i].tag = tag;
	  objs[i].version = (((uint64_t) vhigh) << 32) | vlow;
          objs[i].dirname = &dirpart;
	  objs[i].objname.append(name, namepos, std::string::npos);
          unpackQualityData(objs[i].qreports, objs[i].flags, qdata.c_str());
          objs[i].name = name;
	  objs[i].object = 0;
	  objs[i].reference = 0;
        }

        if (type == DQM_MSG_GET_IMAGE_DATA)
          if(! readRequest(info, odata, objs, numobjs))
            return false;

        if (type == DQM_MSG_GET_JSON_DATA || type == DQM_MSG_GET_JSROOT_DATA)
          if(! readJsonRequest(odata, objs, numobjs))
            return false;

        // Now render and build response.
	DataBlob imgdata;
        bool blacklisted = false;
        uint32_t words[2] = { sizeof(words), DQM_REPLY_IMAGE_DATA };
        if(type == DQM_MSG_GET_IMAGE_DATA)
          blacklisted = doRender(info, &objs[0], numobjs, imgdata);
        else if(type == DQM_MSG_GET_JSON_DATA)
        {
          getJson(&objs[0], numobjs, imgdata);
          words[1] = DQM_REPLY_JSON_DATA;
        }
        else if (type == DQM_MSG_GET_JSROOT_DATA)
        {
          getJsRoot(&objs[0], numobjs, imgdata);
          words[1] = DQM_REPLY_JSROOT_DATA;
        }
	words[0] += imgdata.size();
	msg->data.reserve(msg->data.size() + words[0]);
	copydata(msg, &words[0], sizeof(words));
	if (imgdata.size())
	  copydata(msg, &imgdata[0], imgdata.size());

	// Release the objects.
	bool hadref = objs[0].reference;
	for (uint32_t i = 0; i < numobjs; ++i)
        {
	  delete objs[i].object;
	  delete objs[i].reference;
        }

        // Report how long it took.
        Time end = Time::current();
        logme()
          << "INFO: rendered '" << objs[0].name
	  << "' version " << objs[0].version
          << " as '" << spec << (blacklisted ? "', black-listed" : "'")
          << ", " << numobjs << " objects, main object had"
	  << (hadref ? "" : " no") << " reference, in "
	  << ((end - start).ns() * 1e-3) << " us\n";

	return true;
      }
      else
      {
	logme()
	  << "ERROR: unrecognised message of length " << len
	  << " and type " << type << " from peer " << p->peeraddr
	  << std::endl;
	return false;
      }
    }

private:
  void
  initPlugins(void)
    {
      // Scan the render plug-in catalogue and set up plug-ins.
      if (! info_.dynobj.empty())
      {
        DQMRenderPlugin::master(&plugins_);
        SharedLibrary::load(info_.dynobj.name());
        for (size_t i = 0, e = plugins_.size(); i != e; ++i)
        {
          if (info_.debug)
            logme() << "initialising plugin #" << i << ": " << typeid(*plugins_[i]).name() << "\n";

          plugins_[i]->initialise(info_.argc, info_.argv);
        }

        applied_.resize (plugins_.size(), false);
      }
    }

  void
  shutPlugins(void)
    {
      // There's nothing to do here, plug-ins are statically allocated.
      // We just detach ourselves.
      plugins_.clear();
    }

  // ----------------------------------------------------------------------
  bool
  readRequest(VisDQMImgInfo &info,
	      std::string &odata,
	      std::vector<VisDQMObject> &objs,
	      uint32_t &numobjs)
    {
      std::vector<VisDQMIndex::Summary> attrs;
      std::vector<std::string> binlabels;
      std::vector<double> axisvals;
      uint32_t lenbits[2];
      uint32_t firstobj = 0;
      uint32_t pos = 0;

      // If strip chart trend data is involved, extract it first.  The
      // first object is a raw datablob array of xaxis bin boundaries,
      // summaries (VisDQMIndex::Summary), and bin labels; 'tag' gives
      // the count.  There may be an optional second ROOT object -
      // never a scalar - which we read in like other ROOT objects.
      if (info.trend != DQM_TREND_OBJECT)
      {
	firstobj = 1;

	// Extract the summaries.
	if (! (objs[0].flags & DQM_PROP_TYPE_DATABLOB))
	{
	  logme()
	    << "ERROR: invalid data type for a trend plot, expected"
	    << " a data blob, got flags 0x"
	    << std::hex << objs[0].flags << std::dec
	    << " and payload of size " << odata.size() << " for "
	    << numobjs << " objects\n";
	  return false;
	}

	if (odata.size() - pos < sizeof(lenbits))
	{
	  logme() << "ERROR: 'GET IMAGE DATA' missing trend data length"
		  << ", expected " << sizeof(lenbits) << " but "
		  << (odata.size() - pos) << " bytes left of "
		  << odata.size() << " for trend attributes\n";
	  return false;
	}

	memcpy(&lenbits[0], &odata[pos], sizeof(lenbits));
	pos += sizeof(lenbits);

	if (odata.size() - pos < lenbits[0] + lenbits[1])
	{
	  logme() << "ERROR: 'GET IMAGE DATA' missing trend data"
		  << ", expected " << (lenbits[0]+lenbits[1]) << " but "
		  << (odata.size() - pos) << " bytes left of "
		  << odata.size() << " for trend attributes\n";
	  return false;
	}

	if (lenbits[0])
	{
	  logme() << "ERROR: 'GET IMAGE DATA' trend data cannot have"
		  << " streamers, got " << lenbits[0] << " bytes\n";
	  return false;
	}

	// Read the x coordinates.
	size_t lenused = 0;
	size_t axissize = objs[0].tag * sizeof(double);
	if (lenbits[1] - lenused <= axissize)
	{
	  logme() << "ERROR: 'GET IMAGE DATA' mismatch in trend data size"
		  << ", expected " << objs[0].tag << " axis doubles for size "
		  << axissize << " but " << (lenbits[1]-lenused) << " left\n";
	  return false;
	}

	axisvals.resize(objs[0].tag);
	memcpy(&axisvals[0], &odata[pos], axissize);
	pos += axissize;
	lenused += axissize;

	// Read the attributes.
	size_t attrsize = objs[0].tag * sizeof(VisDQMIndex::Summary);
	if (lenbits[1] - lenused <= attrsize)
	{
	  logme() << "ERROR: 'GET IMAGE DATA' mismatch in trend data size"
		  << ", expected " << objs[0].tag << " summaries for size "
		  << axissize << " but " << (lenbits[1]-lenused) << " left\n";
	  return false;
	}

	attrs.resize(objs[0].tag);
	memcpy(&attrs[0], &odata[pos], attrsize);
	pos += attrsize;
	lenused += attrsize;

	// Read the bin labels.
	binlabels.reserve(objs[0].tag);
	for (size_t i = 0, e = objs[0].tag; i != e && pos < odata.size(); ++i)
	{
	  const char *text = &odata[pos];
	  const char *end = (const char *) memchr(text, 0, odata.size()-pos);
	  binlabels.push_back(std::string(text, end));
	  pos += end - text + 1;
	}

	if (binlabels.size() != objs[0].tag)
	{
	  logme() << "ERROR: 'GET IMAGE DATA' mismatch in trend data size"
		  << ", expected " << objs[0].tag << " bin labels but "
		  << binlabels.size() << " found\n";
	  return false;
	}
      }

      // Read remaining objects, either scalar or real ROOT objects.
      // Draw a scalar only if the string is non-empty; empty means
      // "missing in action" and is handled in render code.  Real ROOT
      // objects come as (streamer-info, object+reference) pair for as
      // many objects as we were sent.  For strip charts that is at
      // most one additional object.
      if ((objs[0].flags & DQM_PROP_TYPE_MASK) <= DQM_PROP_TYPE_SCALAR)
      {
	if (odata.size())
	  objs[0].object = new TObjString(odata.c_str());
      }
      else
      {
	for (uint32_t i = firstobj; i < numobjs; ++i)
	{
	  if (odata.size() - pos < sizeof(lenbits))
	  {
	    logme() << "ERROR: 'GET IMAGE DATA' missing object length"
		    << ", expected " << sizeof(lenbits) << " but "
		    << (odata.size() - pos) << " bytes left of "
		    << odata.size() << " for image #" << i
		    << " of " << numobjs << std::endl;
	    return false;
	  }

	  memcpy(&lenbits[0], &odata[pos], sizeof(lenbits));
	  pos += sizeof(lenbits);

	  if (odata.size() - pos < lenbits[0] + lenbits[1])
	  {
	    logme() << "ERROR: 'GET IMAGE DATA' missing object data"
		    << ", expected " << (lenbits[0]+lenbits[1]) << " but "
		    << (odata.size() - pos) << " bytes left of "
		    << odata.size() << " for image #" << i
		    << " of " << numobjs << std::endl;
	    return false;
	  }

	  loadStreamerInfo(&odata[pos], lenbits[0]);
	  pos += lenbits[0];

	  TBufferFile buf(TBufferFile::kRead, lenbits[1], &odata[pos], kFALSE);
	  buf.Reset();
	  objs[i].object = extractNextObject(buf);
	  objs[i].reference = extractNextObject(buf);
	  pos += lenbits[1];
	}

	if (pos != odata.size())
	{
	  logme() << "ERROR: 'GET IMAGE DATA' request with excess"
		  << " object data, expected " << pos
		  << " bytes but found " << odata.size() << std::endl;
	  return false;
	}
      }

      // Prepare strip chart if necessary.
      if (info.trend != DQM_TREND_OBJECT)
      {
	objs[0].object = makeStripChart(info, objs[0].flags, objs[0].objname,
					axisvals, attrs, binlabels,
					&objs[0], numobjs);
	if (numobjs > 1)
	{
	  delete objs[1].object;
	  delete objs[1].reference;
	  objs[1].object = 0;
	  objs[1].reference = 0;
	  numobjs = 1;
	}
      }

      return true;
    }

  // ----------------------------------------------------------------------
  bool
  readJsonRequest(
	      std::string &odata,
	      std::vector<VisDQMObject> &objs,
	      uint32_t &numobjs)
    {
      std::vector<VisDQMIndex::Summary> attrs;
      std::vector<std::string> binlabels;
      std::vector<double> axisvals;
      uint32_t lenbits[2];
      uint32_t firstobj = 0;
      uint32_t pos = 0;

      if ((objs[0].flags & DQM_PROP_TYPE_MASK) <= DQM_PROP_TYPE_SCALAR)
      {
	if (odata.size())
	  objs[0].object = new TObjString(odata.c_str());
      }
      else
      {
	for (uint32_t i = firstobj; i < numobjs; ++i)
	{
	  if (odata.size() - pos < sizeof(lenbits))
	  {
	    logme() << "ERROR: 'GET JSON DATA' missing object length"
		    << ", expected " << sizeof(lenbits) << " but "
		    << (odata.size() - pos) << " bytes left of "
		    << odata.size() << " for image #" << i
		    << " of " << numobjs << std::endl;
	    return false;
	  }

	  memcpy(&lenbits[0], &odata[pos], sizeof(lenbits));
	  pos += sizeof(lenbits);

	  if (odata.size() - pos < lenbits[0] + lenbits[1])
	  {
	    logme() << "ERROR: 'GET JSON DATA' missing object data"
		    << ", expected " << (lenbits[0]+lenbits[1]) << " but "
		    << (odata.size() - pos) << " bytes left of "
		    << odata.size() << " for image #" << i
		    << " of " << numobjs << std::endl;
	    return false;
	  }

	  loadStreamerInfo(&odata[pos], lenbits[0]);
	  pos += lenbits[0];

	  TBufferFile buf(TBufferFile::kRead, lenbits[1], &odata[pos], kFALSE);
	  buf.Reset();
	  objs[i].object = extractNextObject(buf);
	  objs[i].reference = extractNextObject(buf);
	  pos += lenbits[1];
	}

	if (pos != odata.size())
	{
	  logme() << "ERROR: 'GET JSON DATA' request with excess"
		  << " object data, expected " << pos
		  << " bytes but found " << odata.size() << std::endl;
	  return false;
	}
      }
      return true;
    }

  void getJsRoot(VisDQMObject *objs, size_t /*numobjs*/, DataBlob &jsondata)
  {
    std::string json(std::move(rootObjectToJson(objs[0].object)));
    DataBlob tmp(json.begin(), json.end());
    jsondata = tmp;
    return;
  }

  void getJson(VisDQMObject *objs, size_t /*numobjs*/, DataBlob &jsondata)
  {
    TObject *ob = objs[0].object;
    std::string json ="";
    if (const TH1* const h = dynamic_cast<const TH1* const>(ob))
    {
      std::string jsonTemplate =
          "{'hist':"
          "{"
          "'type':'%1'"
          "%2"          //'title'
          ",'stats':{%3}"
          "%4"          //Xaxis
          "%5"          //Yaxis
          "%6"          //Zaxis for TH2 and TProfile2D, empty for TH1 and TProfile
          ",'values':{"
          "'min':%7"
          ",'max':%8"
          "}"
          ",'bins':{"
          "%9"
          "}"
          "%10"         // yMin for TProfile, zMin for TProfile2D, empty for the rest
          "%11"         // yMax for TProfile, zMax for TProfile2D, empty for the rest
          "%12"         // errorStyle for TProfile and TProfile2D, empty for the rest
          "} "
          "}";
      if (const TH2* const h2 = dynamic_cast<const TH2* const>(ob))
      {
        if (const TProfile2D* const h2d = dynamic_cast<const TProfile2D* const>(ob))
        {
          json += StringFormat(jsonTemplate)
              .arg(h2d->Class_Name())
              .arg(optionalTextValueToJson<const char* const>("title", h2d->GetTitle()))
              .arg(statsToJson<TH2>(h2d))
              .arg(axisDataToJson(h2d->GetXaxis()))
              .arg(axisDataToJson(h2d->GetYaxis()))
              .arg(axisDataToJson(h2d->GetZaxis()))
              .arg(h2d->GetMinimum())
              .arg(h2d->GetMaximum())
              .arg(binsToArray(h2d))
              .arg(optionalNumericValueToJson<Double_t>("zMin", h2d->GetZmin()))
              .arg(optionalNumericValueToJson<Double_t>("zMax", h2d->GetZmax()))
              .arg(errorCodeToJson<TProfile2D>(h2d));
        }
        else
        { /*just TH2*/
          json += StringFormat(jsonTemplate)
              .arg(h2->Class_Name())
              .arg(optionalTextValueToJson<const char* const>("title", h2->GetTitle()))
              .arg(statsToJson<TH2>(h2))
              .arg(axisDataToJson(h2->GetXaxis()))
              .arg(axisDataToJson(h2->GetYaxis()))
              .arg(axisDataToJson(h2->GetZaxis()))
              .arg(h2->GetMinimum())
              .arg(h2->GetMaximum())
              .arg(binsToArray(h2))
              .arg("")
              .arg("")
              .arg("");
        }
      }
      else
      { /* not TH2 */
        if (const TProfile* const   tprof = dynamic_cast<const TProfile* const >(ob))
        {
          json += StringFormat(jsonTemplate)
              .arg(tprof->Class_Name())
              .arg(optionalTextValueToJson<const char* const>("title", tprof->GetTitle()))
              .arg(statsToJson<TH1>(tprof))
              .arg(axisDataToJson(tprof->GetXaxis()))
              .arg(axisDataToJson(tprof->GetYaxis()))
              .arg("")
              .arg(tprof->GetMinimum())
              .arg(tprof->GetMaximum())
              .arg(binsToArray(tprof))
              .arg(optionalNumericValueToJson<Double_t>("yMax", tprof->GetYmax()))
              .arg(optionalNumericValueToJson<Double_t>("yMin", tprof->GetYmin()))
              .arg(errorCodeToJson<TProfile>(tprof));
        }
        else
        { /*just TH1*/
          json += StringFormat(jsonTemplate)
              .arg(h->Class_Name())
              .arg(optionalTextValueToJson<const char* const>("title", h->GetTitle()))
              .arg(statsToJson<TH1>(h))
              .arg(axisDataToJson(h->GetXaxis()))
              .arg(axisDataToJson(h->GetYaxis()))
              .arg("")
              .arg(h->GetMinimum())
              .arg(h->GetMaximum())
              .arg(binsToArray(h))
              .arg("")
              .arg("")
              .arg("");
        }
      }
    }
    else
    {
      json = "{'hist': 'unsupported type'}";
    }
    replacePseudoNumericValues(json);
    boost::replace_all(json, "'","\"");

    DataBlob tmp(json.begin(), json.end());
    jsondata = tmp;

    return;
  }

  void applyTPadCustomizations(TPad * p,
                               VisDQMImgInfo &i)
  {
    if (i.xaxis.type == "lin")
      p->SetLogx(0);
    else if (i.xaxis.type == "log")
      p->SetLogx(1);

    if (i.yaxis.type == "lin")
      p->SetLogy(0);
    else if (i.yaxis.type == "log")
      p->SetLogy(1);

    if (i.zaxis.type == "lin")
      p->SetLogz(0);
    else if (i.zaxis.type == "log")
      p->SetLogz(1);

    if (gStyle)
      gStyle->SetOptStat(i.showstats);
  }

  void drawReferenceStatBox(VisDQMImgInfo &i,
                            size_t order,
                            TH1 * ref,
                            int color,
                            const std::string &name,
                            std::vector<TObject *> &nukem)
  {
    // Draw stats box for every additional ovelayed reference
    // object, appending latest at the bottom of the stats box
    // drawn last. FIXME: stats' coordinates are fixed, which
    // is ugly, but apparently we cannot have them back from
    // ROOT unless we use some public variable (gPad) which I
    // do not know if it is thread safe but which I know is
    // causing us problems.
    TPaveStats * currentStat = new TPaveStats(0.78, 0.835 - (order + 1) * 0.16,
                                              0.98, 0.835 - order * 0.16, "brNDC");
    if (currentStat)
    {
      currentStat->SetBorderSize(1);
      nukem.push_back(currentStat);
      std::stringstream ss;
      if (order == 0)
        currentStat->AddText("StandardRef");
      else
      {
        if (i.reference == DQM_REF_SAMESAMPLE)
          ss << name;
        else
          // The numbering logic here is misleading. Numbers do **not** refer
          // directly to the text-field numbering used in the Javascript,
          // rather they refer to the actual objects that have been asked and
          // found in the index. If an object/sample has been asked but not
          // found, it is skipped but the counter is not incremented. All the
          // correct handling is done on the server side while searching for
          // samples/objects and packing information. At this stage we are mere
          // clients that rely on that information.
          switch(order) {
            case 1:
              ss << i.reflabel1;
              break;
            case 2:
              ss << i.reflabel2;
              break;
            case 3:
              ss << i.reflabel3;
              break;
            case 4:
              ss << i.reflabel4;
              break;
            default:
              assert(0);
              break;
          }
        currentStat->AddText(ss.str().c_str())->SetTextColor(color); ss.str("");
      }
      ss << "Entries = " << ref->GetEntries();
      currentStat->AddText(ss.str().c_str())->SetTextColor(color); ss.str("");
      ss << "Mean  = " << ref->GetMean();
      currentStat->AddText(ss.str().c_str())->SetTextColor(color); ss.str("");
      ss << "RMS   = " << ref->GetRMS();
      currentStat->AddText(ss.str().c_str())->SetTextColor(color); ss.str("");
      currentStat->SetOptStat(1111);
      currentStat->SetOptFit(0);
      currentStat->Draw();
    }
  }

  double calculateScalingFactor(VisDQMObject *objs, size_t numobjs)
  {
    double data_area = 0;
    double total_stack_area = 0;

    if (TH1 * h = dynamic_cast<TH1*>(objs[0].object))
      data_area = h->Integral();

    for (size_t n = 1; n < numobjs; n++)
    {
      TH1 *histogram = dynamic_cast<TH1*>(objs[n].object);
      assert(histogram);
      double histogram_area = histogram->Integral();
      assert(histogram_area >= 0);
      total_stack_area += histogram_area;
    }

    return data_area > 0 ?
        (total_stack_area > 0.0 ? data_area/total_stack_area : 1.0) : 1.0;
  }

  // ----------------------------------------------------------------------
  // Render arbitraty text message into the TCanvas.
  void doRenderMsg(std::string histogram_name,
                   const char * msg,
                   const Color_t &c,
                   std::vector<TObject *> &nukem)
  {
    std::string label;
    stripNameToLabel(label, histogram_name);
    TText *t1 = new TText(.5, .58, label.c_str());
    TText *t2 = new TText(.5, .42, msg);
    t1->SetNDC(kTRUE);     t2->SetNDC(kTRUE);
    t1->SetTextSize(0.10); t2->SetTextSize(0.10);
    t1->SetTextAlign(22);  t2->SetTextAlign(22);
    t1->SetTextColor(c);   t2->SetTextColor(c);
    t1->Draw();            t2->Draw();
    nukem.push_back(t1);   nukem.push_back(t2);
  }

  // ----------------------------------------------------------------------
  // Extract the minimum and maximum range of a histogram, excluding
  // bin with 0 content or with 0 value.

  void computeMinAndMaxForEfficiency(const TH1* h,
                                     double ratio_min, double ratio_max,
                                     double &minimum, double &maximum) {
    int bin, binx, biny, binz;
    int xfirst  = h->GetXaxis()->GetFirst();
    int xlast   = h->GetXaxis()->GetLast();
    int yfirst  = h->GetYaxis()->GetFirst();
    int ylast   = h->GetYaxis()->GetLast();
    int zfirst  = h->GetZaxis()->GetFirst();
    int zlast   = h->GetZaxis()->GetLast();
    double value = 0;
    for (binz = zfirst; binz <= zlast; binz++) {
      for (biny = yfirst; biny <= ylast; biny++) {
        for (binx = xfirst; binx <= xlast; binx++) {
          bin = h->GetBin(binx, biny, binz);
          value = h->GetBinContent(bin);
          if (value == 0)
            continue;
          if (value < minimum && value > ratio_min) {
            minimum = value;
          }
          if (value > maximum && value < ratio_max) {
            maximum = value;
          }
        }
      }
    }
  }

  // ----------------------------------------------------------------------
  // Set user-defined ranges for histograms
  void applyUserRange(TH1* obj, VisDQMImgInfo &i) {
    double xmin = NAN, xmax = NAN;
    double ymin = NAN, ymax = NAN;
    double zmin = NAN, zmax = NAN;
    // If we have an object that has axes, apply the requested params.
    // Set only the bounds that were specified, and leave the rest to
    // the natural range.  Unset bounds have NaN as their value.
    if (TAxis *a = obj->GetXaxis())
    {
      if (! (isnan(i.xaxis.min) && isnan(i.xaxis.max)))
      {
        xmin = a->GetXmin();
        xmax = a->GetXmax();
        a->SetRangeUser(isnan(i.xaxis.min) ? xmin : i.xaxis.min,
                        isnan(i.xaxis.max) ? xmax : i.xaxis.max);
      }
    }

    if (TAxis *a = obj->GetYaxis())
    {
      if (! (isnan(i.yaxis.min) && isnan(i.yaxis.max)))
      {
        ymin = a->GetXmin();
        ymax = a->GetXmax();
        a->SetRangeUser(isnan(i.yaxis.min) ? ymin : i.yaxis.min,
                        isnan(i.yaxis.max) ? ymax : i.yaxis.max);
      }
    }

    if (TAxis *a = obj->GetZaxis())
    {
      if (! (isnan(i.zaxis.min) && isnan(i.zaxis.max)))
      {
        zmin = a->GetXmin();
        zmax = a->GetXmax();
        a->SetRangeUser(isnan(i.zaxis.min) ? zmin : i.zaxis.min,
                        isnan(i.zaxis.max) ? zmax : i.zaxis.max);
      }
    }
  }

  // ----------------------------------------------------------------------
  // Render ovelaid ROOT objects and their ratio at the bottom.
  void
  doRenderOverlayAndRatio(TCanvas &c,
                          VisDQMImgInfo &i,
                          VisDQMObject *objs,
                          size_t numobjs,
                          const VisDQMRenderInfo &ri,
                          std::vector<TObject *> &nukem)
  {
    TObject *ob = objs[0].object;
    //Do normal rendering for everything but TH1*
    if (dynamic_cast<TH2 *>(ob) ||
        dynamic_cast<TProfile *>(ob) ||
        dynamic_cast<TObjString *>(ob))
    {
      doRenderOrdinary(c, i, objs, numobjs, ri, nukem);
      return;
    }

    int colorIndex = sizeof(colors)/sizeof(int);
    float ratio_label_font_size = 0.12;
    // Prepare the TPads.  We do not locally delete the TPad nor mark
    // them to be deleted since they will be take care of by the
    // TCanvas in which they are rendered.
    TPad *pad1 = new TPad("main", "main", 0, 0.3, 1, 1);
    pad1->SetBottomMargin(0.05);
    applyTPadCustomizations(pad1, i);
    pad1->Draw();
    pad1->cd();

    doRenderOrdinary(c, i, objs, numobjs, ri, nukem);
    c.cd();
    TPad *pad2 = new TPad("ratio", "ratio", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0.05);
    if (i.xaxis.type == "lin")
      pad2->SetLogx(0);
    else if (i.xaxis.type == "log")
      pad2->SetLogx(1);
    pad2->Draw();
    pad2->cd();

    TH1 * h = nullptr;
    if ( (h = dynamic_cast<TH1 *>(ob)) && (h->GetEntries() != 0))
    {
      double norm = h->GetSumOfWeights();
      static const double RATIO_MIN = 0.001;
      static const double RATIO_MAX = 10.0;
      double ratio_min = RATIO_MAX;
      double ratio_max = RATIO_MIN;
      size_t histograms_to_be_drawn = nukem.size();
      for (size_t n = 0; n < numobjs; ++n)
      {
        // Skip the embedded reference histograms, in case it is
        // there.
        if (n == 0)
          continue;

        if (TH1 *  den = dynamic_cast<TH1 *>(objs[n].object))
        {
          // No point in making the ratio if we do have an empty
          // denominator. The numerator has been already checked.
          if (den->GetEntries() == 0 or den->GetSumOfWeights() == 0)
            continue;

          TH1 * num = dynamic_cast<TH1 *>(h->Clone());
          nukem.push_back(num);
          double sum = den->GetSumOfWeights();
          if (sum)
          {
            if (objs[0].flags & VisDQMIndex::SUMMARY_PROP_EFFICIENCY_PLOT)
              norm = sum = 1.;
            den->Scale(norm/sum);
            num->SetStats(0);
            if (!num->GetSumw2N())
              num->Sumw2();
            num->Divide(den);
            num->SetLineColor(colors[n%colorIndex]);
            num->SetTitle("");

            num->GetXaxis()->SetLabelFont(42);
            num->GetXaxis()->SetLabelSize(ratio_label_font_size);
            num->GetXaxis()->SetTitleFont(42);
            num->GetXaxis()->SetTitleSize(ratio_label_font_size);
            num->GetXaxis()->SetTitleOffset(-1.2);

            computeMinAndMaxForEfficiency(num, RATIO_MIN, RATIO_MAX,
                                          ratio_min, ratio_max);
            num->GetYaxis()->SetTitle("");
            num->GetYaxis()->SetLabelFont(42);
            num->GetYaxis()->SetLabelSize(ratio_label_font_size);

            TH1 * ratio_shadow_error_range = dynamic_cast<TH1 *>(num->Clone());
            nukem.push_back(ratio_shadow_error_range);
            ratio_shadow_error_range->SetFillStyle(3001);
            ratio_shadow_error_range->SetFillColor(TColor::GetColorTransparent(colors[n%colorIndex], 0.3));
            ratio_shadow_error_range->SetLineColor(TColor::GetColorTransparent(colors[n%colorIndex], 0.3));
            ratio_shadow_error_range->SetMarkerColor(TColor::GetColorTransparent(colors[n%colorIndex], 0.3));
            ratio_shadow_error_range->SetMarkerSize(0);
            for (int bin = 1; bin < ratio_shadow_error_range->GetNbinsX(); ++bin)
              ratio_shadow_error_range->SetBinContent(bin, 1.0);
          }
        }
      }

      // Finally do the actual drawing, not that all limits have been
      // globally computed across all references.

      // We need some further checking on the limits here. If the
      // ratio-plot is empty, the minimum value is set to RATIO_MAX
      // and the maximum value is correctly set to 0. We arbirarily
      // set the minimum to RATIO_MIN and the maximum to be
      // RATIO_MAX. We generically check this condition by requiring
      // that the maximum found is lower than the minimum.
      if (ratio_max <= ratio_min) {
        if (ratio_min == 1) {
          ratio_min = 0.2;
          ratio_max = 1.8;
        } else {
          ratio_min = RATIO_MIN;
          ratio_max = RATIO_MAX;
        }
      }
      for (size_t histo = histograms_to_be_drawn; histo < nukem.size(); ++histo) {
        TH1 * to_draw = dynamic_cast<TH1 *>(nukem[histo]);
        to_draw->GetYaxis()->SetRangeUser(ratio_min, ratio_max);
        if (histo == histograms_to_be_drawn) {
          to_draw->Draw("epl");
        } else {
          if (to_draw->GetMarkerSize() == 0) {
            to_draw->Draw("same e2");
          } else {
            to_draw->Draw("same epl");
          }
        }
      }
    }
    c.cd();
  }

  // ----------------------------------------------------------------------
  // Render stacked ROOT objects.
  void
  doRenderStacked(TCanvas &c,
                  VisDQMImgInfo &i,
                  VisDQMObject *objs,
                  size_t numobjs,
                  const VisDQMRenderInfo & /* ri */,
                  std::vector<TObject *> &nukem)
  {
    int colorIndex = sizeof(colors)/sizeof(int);
    uint32_t draw_me_first_index = 0;
    double absolute_y_max = 0;
    double y_max = 0;

    applyTPadCustomizations(dynamic_cast<TPad*>(&c), i);

    TObject *ob = objs[0].object;
    // Do not even try if we are offered TH2 or TProfile objects
    if (dynamic_cast<TH2 *>(ob) ||
        dynamic_cast<TProfile *>(ob) ||
        dynamic_cast<TObjString *>(ob))
    {
      Color_t c = TColor::GetColor(64, 64, 64);
      doRenderMsg(objs[0].name, "Stacking not supported", c, nukem);
      return;
    }

    if (TH1 * dataHistogram = dynamic_cast<TH1 *>(objs[0].object))
    {
      absolute_y_max = dataHistogram->GetMaximum();
      // Calculate what each the histograms that are to be stacked should be scaled by
      // to match the data histogram
      double scalingFactor = calculateScalingFactor(objs, numobjs);
      // Build the histogram stack object (THStack)
      THStack *histogramStack = new THStack();
      nukem.push_back(histogramStack);
      for (size_t n = 1; n < numobjs; ++n)
      {
        if (TH1 * h = dynamic_cast<TH1 *>(objs[n].object))
        {
          histogramStack->Add(h);
          h->Scale(scalingFactor);
          h->SetLineColor(colors[n%colorIndex]);
          h->SetFillColor(colors[n%colorIndex]);
          h->SetFillStyle(3002);
          h->SetLineWidth(1);
          h->GetListOfFunctions()->Delete();
          y_max = h->GetMaximum();
          if ( y_max > absolute_y_max)
          {
            absolute_y_max = y_max;
            draw_me_first_index = n;
          }
        }
      }

      // Draw the histogram stack and then the histogram on top
      dataHistogram->SetLineColor(kBlack);
      dataHistogram->SetLineWidth(2);
      if (draw_me_first_index == 0)
        dataHistogram->Draw();
      histogramStack->Draw((draw_me_first_index == 0 ? "same" : ""));
      dataHistogram->Draw("same");
    }

    if (i.showstats)
      for (size_t n = 1; n < numobjs; ++n)
        drawReferenceStatBox(i, n,
                             dynamic_cast<TH1 *>(objs[n].object),
                             colors[n%colorIndex],
                             objs[n].name, nukem);
  }

  // ----------------------------------------------------------------------
  // Ordinary rendering of a ROOT object.
  void doRenderOrdinary(TCanvas &c,
                        VisDQMImgInfo &i,
                        VisDQMObject * objs,
                        size_t numobjs,
                        const VisDQMRenderInfo &ri,
                        std::vector<TObject *> &nukem)
  {
    std::string samePlotOptions("same p");
    TObject *ob = objs[0].object;
    TH1 * h = dynamic_cast<TH1 *>(ob);
    bool isMultiDimensional = dynamic_cast<TH2*>(ob) ? 1 : 0;
    isMultiDimensional |= dynamic_cast<TH3 *>(ob) ? 1 : 0;
    float max_value_in_Y = std::numeric_limits<float>::lowest();

    // Handle text.
    if (TObjString *os = dynamic_cast<TObjString *>(ob))
    {
      TText *t = new TText(.5, .5, os->GetName());
      t->SetTextFont (63);
      t->SetTextAlign (22);
      t->SetTextSize (16);
      nukem.push_back(t);
      ob = t;
    }

    // The logic to account for possible user-supplied ranges
    // (especially in Y) and the automatic selection of the best Y
    // range in case of overlaid histograms is rather tricky. As a
    // final solution, the values supplied by the user (if any) are
    // applied *last*, so that they will prevail over the automatic
    // algorithm. Plots that have been labelled as efficiency plots
    // do not even enter this rescaling algorithm

    // Maybe draw overlay from reference and other objects.
    if (h)
    {
      max_value_in_Y = h->GetMaximum();
      float norm = h->GetSumOfWeights();
      if (i.refnorm != "False"
          && norm > 0
          && !(objs[0].flags & VisDQMIndex::SUMMARY_PROP_EFFICIENCY_PLOT)
          && !isMultiDimensional)
      {
        for (size_t n = 0; n < numobjs; ++n)
        {
          TObject *refobj = 0;
          if (n == 0)
            refobj = objs[0].reference;
          else if (n > 0)
            refobj = objs[n].object;
          TH1F *ref1 = dynamic_cast<TH1F *>(refobj);
          if (ref1) {
            float den = ref1->GetSumOfWeights();
            if (den > 0)
              max_value_in_Y = max_value_in_Y > ref1->GetMaximum()*norm/den
                  ? max_value_in_Y : ref1->GetMaximum()*norm/den;
          }
        }
        if (max_value_in_Y > 0 && numobjs > 1) {
          h->SetMaximum(max_value_in_Y*1.05);
        }
      }
      applyUserRange(h, i);
      // Increase lineWidth in case there are other objects to
      // draw on top of the main one.
      if (numobjs > 1)
        h->SetLineWidth(2);
    }

    applyTPadCustomizations(dynamic_cast<TPad*>(&c), i);

    // Draw the main object on top.
    ob->Draw(ri.drawOptions.c_str());

    // Maybe draw overlay from reference and other objects.
    for (size_t n = 0; n < numobjs; ++n)
    {
      TObject *refobj = 0;
      // Compute colors array size on the fly and use it to loop
      // over defined colors in case the number of objects to
      // overlay is greater than the available colors
      // (n%colorIndex).
      int colorIndex = sizeof(colors)/sizeof(int);
      if (n == 0 && (i.reference == DQM_REF_OVERLAY
                     || i.reference == DQM_REF_OVERLAY_RATIO))
        refobj = objs[0].reference;
      else if (n > 0)
        refobj = objs[n].object;

      TH1 *ref1 = dynamic_cast<TH1 *>(refobj);
      TProfile *refp = dynamic_cast<TProfile *>(refobj);
      TH2 *ref2d = dynamic_cast<TH2 *>(refobj);
      TH3 *ref3d = dynamic_cast<TH3 *>(refobj);
      if (refp)
      {
        refp->SetLineColor(colors[n%colorIndex]);
        refp->SetLineWidth(2);
        refp->GetListOfFunctions()->Delete();
        refp->Draw("same hist");
      }
      // No point in even trying to overlay 2D or 3D histograms in the
      // same canvas. Bail out w/o complaints.
      else if (ref2d || ref3d) {
        return;
      }
      else if (ref1)
      {
        // Perform KS statistical test only on the first available
        // reference, excluding the (possible) default one
        // injected during the harvesting step.
        int color = colors[n%colorIndex];
        double norm = 1.;
        if (h)
          norm = h->GetSumOfWeights();

        if (n==1 && (! isnan(i.ktest))
            && h && norm
            && ref1 && ref1->GetSumOfWeights())
        {
          double prob = h->KolmogorovTest(ref1);
          color = prob < i.ktest ? kRed-4 : kGreen-3;
          char buffer[14];
          snprintf(buffer, 14, "%6.5f", prob);
          TText t;
          t.SetTextColor(color);
          t.DrawTextNDC(0.45, 0.9, buffer);
        }

        ref1->SetLineColor(color); ref1->SetMarkerColor(color);
        ref1->SetMarkerStyle(kStar); ref1->SetMarkerSize(0.85);
        ref1->GetListOfFunctions()->Delete();
        if (i.showerrbars)
          samePlotOptions += " e1 x0";

        // Check if the original plot has been flagged as an
        // efficieny plot at production time: if this is the case,
        // then avoid any kind of normalization that introduces
        // fake effects.
        // Also, if the option norm=False has been supplied, avoid
        // doing any normalization.
        if (norm && !(objs[0].flags & VisDQMIndex::SUMMARY_PROP_EFFICIENCY_PLOT) &&
            i.refnorm != "False") {
          if (ref1->GetSumOfWeights())
            nukem.push_back(ref1->DrawNormalized(samePlotOptions.c_str(), norm));
        } else {
          ref1->Draw(samePlotOptions.c_str());
        }

        if (i.showstats) {
          // Apply user-defined ranges also to reference histograms, so
          // that the shown statistics is consistent with the one of the
          // main plot.
          applyUserRange(ref1, i);
          drawReferenceStatBox(i, n, ref1, color, objs[n].name, nukem);
        }
      }
    } // End of loop over all reference sample
  }

  // ----------------------------------------------------------------------
  // Render a ROOT object.
  bool
  doRender(VisDQMImgInfo &i, VisDQMObject *objs, size_t numobjs, DataBlob &imgdata)
    {
      VisDQMObject &o = objs[0];
      VisDQMRenderInfo ri = { i.drawOptions, (info_.blacklist.count(o.name) > 0) };

      // Give me 30 seconds to render the histogram, then die.
      struct itimerval timer;
      timer.it_value.tv_sec = 30;
      timer.it_value.tv_usec = 0;
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;
      setitimer(ITIMER_VIRTUAL, &timer, 0);

      // Prepare render defaults and the canvas.
      delete gStyle;
      gStyle = (TStyle *) standardStyle_->Clone();
      gROOT->ForceStyle();
      for (size_t n = 0; n < numobjs; ++n)
      {
        if (objs[n].object)
          objs[n].object->UseCurrentStyle();
        if (objs[n].reference)
          objs[n].reference->UseCurrentStyle();
      }

      // If we were requested to draw only the reference, use the
      // reference in place of the object but otherwise applying all the
      // same logic.
      TObject *ob = o.object;
      if (i.reference == DQM_REF_REFERENCE)
      {
        ob = o.object = o.reference;
        o.reference = 0;
      }

      TCanvas c("", "", i.width+4, i.height+28);
      c.SetFillColor(TColor::GetColor(255, 255, 255));
      c.SetBorderMode(0);

      // Record list of temporary objects we should clear out.
      std::vector<TObject *> nukem;

      // Force histogram bounds reset now.  Plug-ins can change them.
      // We need this for the axis bounds code to work.
      bool unsafe = false;
      if (TH1 *h = dynamic_cast<TH1 *>(ob))
      {
        h->SetMinimum();
        h->SetMaximum();
	if (isnan(h->Integral())
	    || isnan(h->GetRMS())
	    || isnan(h->GetSumOfWeights()))
	  unsafe = true;
      }

      // Invoke pre-draw hook on plug-ins that want to apply.
      for (size_t p = 0; p < plugins_.size(); ++p)
      {
        applied_[p] = false;
        if (! ri.blacklisted)
        {
          if (info_.debug)
            logme() << "offering to plugin #" << p
		    << ": " << typeid(*plugins_[p]).name() << "\n";

          if (plugins_[p]->applies(o, i))
          {
            if (info_.debug)
              logme() << "start predraw on plugin #" << p
		      << ": " << typeid(*plugins_[p]).name() << "\n";

            plugins_[p]->preDraw(&c, o, i, ri);
            applied_[p] = true;

            if (info_.debug)
              logme() << "end predraw on plugin #" << p
		      << ": " << typeid(*plugins_[p]).name() << "\n";
          }
        }
      }

      // Actually draw something.  If there's no object, inform user.
      if (! ob)
      {
        Color_t c = TColor::GetColor(64, 64, 64);
        doRenderMsg(o.name,
                    i.reference != DQM_REF_REFERENCE
                    ? "is not available now"
                    : "has no reference",
                    c, nukem);
      }

      // If it exists but was black-listed, inform user.
      else if (ri.blacklisted)
      {
        Color_t c = TColor::GetColor(178, 32, 32);
        doRenderMsg(o.name, "blacklisted for crashing ROOT", c, nukem);
      }

      // It's there and wasn't black-listed, paint it.
      else
      {
        // Real drawing
        switch (i.reference)
        {
          case DQM_REF_STACKED:
            doRenderStacked(c, i, objs, numobjs, ri, nukem);
            break;
          case DQM_REF_OVERLAY_RATIO:
            doRenderOverlayAndRatio(c, i, objs, numobjs, ri, nukem);
            break;
          default:
            doRenderOrdinary(c, i, objs, numobjs, ri, nukem);
        }

        // If the stats are bad draw alert on top.
        if (unsafe)
        {
          Color_t c = TColor::GetColor(178, 32, 32);
          doRenderMsg(o.name, "NaNs", c, nukem);
        }
      }

      // Invoke post-draw hook on plug-ins that applied.
      for (size_t p = 0; p < plugins_.size(); ++p)
        if (applied_[p])
        {
          if (info_.debug)
            logme() << "start postdraw on plugin #" << p
		    << ": " << typeid(*plugins_[p]).name() << "\n";

          plugins_[p]->postDraw(&c, o, i);

          if (info_.debug)
            logme() << "end postdraw on plugin #" << p
		    << ": " << typeid(*plugins_[p]).name() << "\n";
        }

      // Grab image data and remember what version this object was.
      DQMImageDump idump (&c, imgdata);

      // Clear out temporary objects we created to avoid leaks.
      for (size_t n = 0, e = nukem.size(); n < e; ++n)
        delete nukem[n];

      // Let timer go now, we made it.
      timer.it_value.tv_sec = 0;
      timer.it_value.tv_usec = 0;
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;
      setitimer(ITIMER_VIRTUAL, &timer, 0);

      // Indicate whether it was blacklisted.
      return ri.blacklisted;
    }
};

// ----------------------------------------------------------------------
int
main(int argc, char **argv)
{
  static const int FATAL_OPTS = (lat::Signal::FATAL_DEFAULT
				 & ~(lat::Signal::FATAL_ON_INT
				     | lat::Signal::FATAL_ON_QUIT
				     | lat::Signal::FATAL_DUMP_CORE));

  // Handle fatal signals.
  DebugAids::failHook(&onAssertFail);
  Signal::handleFatal(argv[0], IOFD_INVALID, 0, 0, FATAL_OPTS);

  // Make all output line buffered. We need this because multiple
  // concurrent processes are feeding output to a single log daemon.
  std::cout.flush();
  std::cerr.flush();
  std::ios_base::sync_with_stdio(true);
  setvbuf(stdout, 0, _IOLBF, 1024);
  setvbuf(stderr, 0, _IOLBF, 1024);

  // Check and process arguments.
  VisDQMImageServer::ServerInfo params;
  params.argc = argc;
  params.argv = argv;
  params.debug = false;
  for (int i = 1; i < argc; ++i)
    if (i < argc-1 && ! strcmp(argv[i], "--state-directory"))
      params.statedir = argv[++i];
    else if (i < argc-1 && ! strcmp(argv[i], "--load"))
      params.dynobj = argv[++i];
    else if (! strcmp(argv[i], "--no-debug"))
      params.debug = false;
    else if (! strcmp(argv[i], "--debug"))
      params.debug = true;
    else
      break;

  if (! params.statedir || ! *params.statedir)
  {
    std::cerr << "Usage: " << argv[0]
	      << " --state-directory STATE-DIRECTORY"
	      << " [--load DYNAMIC-OBJECT]"
	      << " [--[no-]debug]"
	      << " [render-plugin-options]\n";
    return 1;
  }

  if (! params.statedir.isDirectory ())
  {
    std::cerr << params.statedir << ": not a directory\n";
    return 1;
  }

  // Remove any existing socket.
  params.sockpath = Filename(params.statedir, "socket");
  Filename::remove(params.sockpath, false, true);

  // Remove any existing stop file.
  params.stopfile = Filename(params.statedir, "stop");
  Filename::remove(params.stopfile, false, true);

  // Create pid file.
  Filename pidfile(params.statedir, "pid");
  FILE *f = fopen(pidfile, "w");
  fprintf(f, "%ld", (long) getpid());
  fclose(f);

  // Read black list.
  Filename topdir = Filename(params.statedir).directory();
  Filename blacklistFile(topdir, "blacklist.txt");
  if (blacklistFile.exists() && blacklistFile.isReadable())
  {
    std::string line;
    std::ifstream f(blacklistFile);
    do
    {
      getline(f, line);
      if (line.empty())
	continue;

      StringList s = StringOps::split(line, ' ', 0, 2, 0, -1);
      params.blacklist[s[1]] = Time::ValueType(atof(s[0].c_str())*1e9);
    } while (f);
  }

  // Re-capture signals from ROOT after ROOT has initialised.
  ROOT::GetROOT();
  for (int sig = 1; sig < NSIG; ++sig)
    if (sig == SIGPIPE)
      Signal::ignore(sig);
    else
      Signal::revert(sig);

  Signal::handleFatal(argv[0], IOFD_INVALID, 0, 0, FATAL_OPTS);

  // Start serving.
  VisDQMImageServer server(params);
  server.run();
  return 0;
}
