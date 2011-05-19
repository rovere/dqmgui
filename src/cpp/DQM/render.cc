#define DEBUG(n,x)
#include "DQM/DQMRenderPlugin.h"
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
#include "TObjString.h"
#include "TDirectory.h"
#include "TImageDump.h"
#include "TASImage.h"
#include "TCanvas.h"
#include "TText.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TColor.h"
#include "TAxis.h"
#include "TList.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include <fstream>
#include <iostream>
#include <iostream>
#include <cassert>
#include <cerrno>
#include <map>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <math.h>

using namespace lat;

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
  i.trend = DQM_TREND_OBJECT;
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
    if (!    parseInt       (p, "w=",        2, i.width)
	&& ! parseInt       (p, "h=",        2, i.height)
	&& ! parseReference (p, "ref=",      4, i.reference)
	&& ! parseStripChart(p, "trend=",    6, i.trend)
	&& ! parseDouble    (p, "xmin=",     5, i.xaxis.min)
	&& ! parseDouble    (p, "xmax=",     5, i.xaxis.max)
	&& ! parseAxisType  (p, "xtype=",    6, i.xaxis.type)
	&& ! parseDouble    (p, "ymin=",     5, i.yaxis.min)
	&& ! parseDouble    (p, "ymax=",     5, i.yaxis.max)
	&& ! parseAxisType  (p, "ytype=",    6, i.yaxis.type)
	&& ! parseDouble    (p, "zmin=",     5, i.zaxis.min)
	&& ! parseDouble    (p, "zmax=",     5, i.zaxis.max)
	&& ! parseAxisType  (p, "ztype=",    6, i.zaxis.type)
	&& ! parseOption    (p, "drawopts=", 9, i.drawOptions))
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
	graph_->Draw("P");

      if (flags_ & DQM_PROP_STRIP_AXIS_TIME)
	gPad->SetBottomMargin(0.25);
    }
};

TObject *
makeStripChart(VisDQMImgInfo &i,
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
  size_t axis = (i.trend >= DQM_TREND_X_MEAN && i.trend <= DQM_TREND_X_NUM_BINS ? 0
		 : i.trend >= DQM_TREND_Y_MEAN && i.trend <= DQM_TREND_Y_NUM_BINS ? 1
		 : i.trend >= DQM_TREND_Z_MEAN && i.trend <= DQM_TREND_Z_NUM_BINS ? 2
		 : 3);
  TH1 *extra = (numobjs > 1 ? dynamic_cast<TH1 *>(objs[1].object) : 0);
  size_t nextra = extra ? 1 : 0;
  size_t nattrs = attrs.size();
  std::vector<double> vals(nattrs);
  std::vector<double> yerr;
  std::vector<double> ylo;
  std::vector<double> yhi;

  switch (i.trend)
  {
  case DQM_TREND_OBJECT:
    assert(false);
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
  double vmax = 0;
  double vmin = 0;
  size_t skip = vals.size() / 30 + 1;
  TH1D *bins = new TH1D("Trend", fulltitle.c_str(), vals.size(), &axisvals[0]);
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

    if (i % skip == 0)
      xaxis->SetBinLabel(i+1, labels[nattrs-i-1].c_str());

    axisvals[i] = (axisvals[i] + axisvals[i+1])/2;
  }
  bins->SetStats(kFALSE);
  bins->LabelsOption("v");
  bins->SetMaximum(1.1*vmax);
  bins->SetMinimum(1.1*vmin);
  //xaxis->CenterLabels(kFALSE);
  //xaxis->SetNdivisions(530);

  // Construct a graph of data points.
  TGraph *graph = 0;
  if (yerr.empty() && yhi.empty())
    graph = new TGraph(vals.size(), &axisvals[0], &vals[0]);
  else if (! yerr.empty())
    graph = new TGraphErrors(vals.size(), &axisvals[0], &vals[0], 0, &yerr[0]);
  else if (! yhi.empty())
    graph = new TGraphAsymmErrors(vals.size(), &axisvals[0], &vals[0],
				  0, 0, &ylo[0], &yhi[0]);

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
  static const uint32_t DQM_REPLY_IMAGE_DATA	= 105;

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
  releaseFromWait(Bucket *msg, WaitObject &w, Object *o)
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
      else if (type == DQM_MSG_GET_IMAGE_DATA)
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
	if (! parseImageSpec(info, spec, error))
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

	if (! readRequest(info, odata, objs, numobjs))
	  return false;

	// Now render and build response.
	DataBlob imgdata;
	uint32_t words[2] = { sizeof(words), DQM_REPLY_IMAGE_DATA };
	bool blacklisted = doRender(info, &objs[0], numobjs, imgdata);
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
      double xmin = NAN, xmax = NAN;
      double ymin = NAN, ymax = NAN;
      double zmin = NAN, zmax = NAN;
      if (! ob)
      {
        std::string label;
        stripNameToLabel(label, o.name);
        Color_t c = TColor::GetColor(64, 64, 64);
        TText *t1 = new TText(.5, .58, label.c_str());
        TText *t2 = new TText(.5, .42,
			      i.reference != DQM_REF_REFERENCE
			      ? "is not available now"
			      : "has no reference");
        t1->SetNDC(kTRUE);     t2->SetNDC(kTRUE);
        t1->SetTextSize(0.10); t2->SetTextSize(0.10);
        t1->SetTextAlign(22);  t2->SetTextAlign(22);
        t1->SetTextColor(c);   t2->SetTextColor(c);
        t1->Draw();            t2->Draw();
        nukem.push_back(t1);   nukem.push_back(t2);
      }

      // If it exists but was black-listed, inform user.
      else if (ri.blacklisted)
      {
        std::string label;
        stripNameToLabel(label, o.name);
        Color_t c = TColor::GetColor(178, 32, 32);
        TText *t1 = new TText(.5, .58, label.c_str());
        TText *t2 = new TText(.5, .42, "blacklisted for crashing ROOT");
        t1->SetNDC(kTRUE);     t2->SetNDC(kTRUE);
        t1->SetTextSize(0.10); t2->SetTextSize(0.10);
        t1->SetTextAlign(22);  t2->SetTextAlign(22);
        t1->SetTextColor(c);   t2->SetTextColor(c);
        t1->Draw();            t2->Draw();
        nukem.push_back(t1);   nukem.push_back(t2);
      }

      // If the stats are bad avoid drawing it, ROOT destabilises.
      else if (unsafe)
      {
        std::string label;
        stripNameToLabel(label, o.name);
        Color_t c = TColor::GetColor(178, 32, 32);
        TText *t1 = new TText(.5, .58, label.c_str());
        TText *t2 = new TText(.5, .42, "not shown due to NaNs");
        t1->SetNDC(kTRUE);     t2->SetNDC(kTRUE);
        t1->SetTextSize(0.10); t2->SetTextSize(0.10);
        t1->SetTextAlign(22);  t2->SetTextAlign(22);
        t1->SetTextColor(c);   t2->SetTextColor(c);
        t1->Draw();            t2->Draw();
        nukem.push_back(t1);   nukem.push_back(t2);
      }

      // It's there and wasn't black-listed, paint it.
      else
      {
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

        // If we have an object that has axes, apply the requested params.
        // Set only the bounds that were specified, and leave the rest to
        // the natural range.  Unset bounds have NaN as their value.
        if (TH1 *h = dynamic_cast<TH1 *>(ob))
        {
          if (TAxis *a = h->GetXaxis())
          {
	    if (! (isnan(i.xaxis.min) && isnan(i.xaxis.max)))
	    {
	      xmin = a->GetXmin();
	      xmax = a->GetXmax();
	      a->SetRangeUser(isnan(i.xaxis.min) ? xmin : i.xaxis.min,
			      isnan(i.xaxis.max) ? xmax : i.xaxis.max);
	    }

	    if (i.xaxis.type == "lin")
	      c.SetLogx(0);
	    else if (i.xaxis.type == "log")
	      c.SetLogx(1);
          }

          if (TAxis *a = h->GetYaxis())
          {
	    if (! (isnan(i.yaxis.min) && isnan(i.yaxis.max)))
	    {
	      ymin = a->GetXmin();
	      ymax = a->GetXmax();
	      a->SetRangeUser(isnan(i.yaxis.min) ? ymin : i.yaxis.min,
			      isnan(i.yaxis.max) ? ymax : i.yaxis.max);
	    }

	    if (i.yaxis.type == "lin")
	      c.SetLogy(0);
	    else if (i.yaxis.type == "log")
	      c.SetLogy(1);
          }

          if (TAxis *a = h->GetZaxis())
          {
	    if (! (isnan(i.zaxis.min) && isnan(i.zaxis.max)))
	    {
	      zmin = a->GetXmin();
	      zmax = a->GetXmax();
	      a->SetRangeUser(isnan(i.zaxis.min) ? zmin : i.zaxis.min,
			      isnan(i.zaxis.max) ? zmax : i.zaxis.max);
	    }

	    if (i.zaxis.type == "lin")
	      c.SetLogz(0);
	    else if (i.zaxis.type == "log")
	      c.SetLogz(1);
          }
        }

        // Draw the main object on top.
        ob->Draw(ri.drawOptions.c_str());

        // Maybe draw overlay from reference and other objects.
	for (size_t n = 0; n < numobjs; ++n)
        {
	  TObject *refobj = 0;
	  if (n == 0 && i.reference == DQM_REF_OVERLAY)
	    refobj = o.reference;
	  else if (n > 0)
	    refobj = objs[n].object;

	  TH1F *ref1f = dynamic_cast<TH1F *>(refobj);
	  TH1D *ref1d = dynamic_cast<TH1D *>(refobj);
	  TProfile *refp = dynamic_cast<TProfile *>(refobj);
	  if (refp)
          {
            refp->SetLineColor(40 + n);
            refp->SetLineWidth(0);
            refp->GetListOfFunctions()->Delete();
            refp->Draw("same hist");
          }
          else if (ref1f || ref1d)
          {
            double norm = 1.;
            if (TH1F *th1f = dynamic_cast<TH1F *>(ob))
              norm = th1f->GetSumOfWeights();
            else if (TH1D *th1d = dynamic_cast<TH1D *>(ob))
              norm = th1d->GetSumOfWeights();

	    TH1 *ref = (ref1f
			? static_cast<TH1 *>(ref1f)
			: static_cast<TH1 *>(ref1d));
            ref->SetLineColor(40 + n);
            ref->SetLineWidth(0);
            ref->SetFillColor(40 + n);
            ref->SetFillStyle(3002);
            ref->GetListOfFunctions()->Delete();
            if (norm)
              nukem.push_back(ref->DrawNormalized("same hist",norm));
            else
              ref->Draw("same hist");
          }
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
  for (int sig = 1; sig < _NSIG; ++sig)
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
