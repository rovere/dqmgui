#ifndef DQM_VISDQMRENDERLINK_H
#define  DQM_VISDQMRENDERLINK_H

#include <pthread.h>

#include <fstream>
#include <string>
#include <map>
#include <list>

#include "classlib/utils/Argz.h"
#include "classlib/utils/Regexp.h"
#include "classlib/utils/RegexpMatch.h"
#include "classlib/iobase/SubProcess.h"
#include "classlib/iobase/LocalSocket.h"
#include "classlib/iobase/Filename.h"

#include "boost/shared_ptr.hpp"
#include "boost/gil/image.hpp"
#include "boost/gil/typedefs.hpp"
#undef HAVE_PROTOTYPES // conflict between python and libjpeg
#undef HAVE_STDLIB_H // conflict between python and libjpeg
#include "boost/gil/extension/io/jpeg_io.hpp"
#include "boost/gil/extension/io/png_io.hpp"
#include "boost/gil/extension/numeric/kernel.hpp"
#include "boost/gil/extension/numeric/convolve.hpp"
#include "boost/gil/extension/numeric/sampler.hpp"
#include "boost/gil/extension/numeric/resample.hpp"
#include "boost/lambda/algorithm.hpp"
#include "boost/lambda/casts.hpp"
#include "boost/algorithm/string.hpp"
#include "rtgu/image/rescale.hpp"
#include "rtgu/image/filters.hpp"

#include "DQM/DQMNet.h"
#include "DQM/Locks.h"
#include "DQM/utils.h"

using namespace lat;

struct IMGOPT { std::string label; Regexp &verify; };
static Regexp RX_OPT_INT("^[0-9]+$");
static Regexp RX_OPT_FLOAT("^([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)?$");
static Regexp RX_OPT_DRAWOPT("^[A-Za-z ]*$");
static Regexp RX_OPT_REFTYPE("^(|object|reference|overlay|ratiooverlay|stacked|samesample)$");
static Regexp RX_OPT_AXISTYPE("^(def|lin|log)$");
static Regexp RX_OPT_TREND_TYPE("^(num-(entries|bins|bytes)|value|"
				"[xyz]-(min|max|bins|mean(-rms|-min-max)?))$");
static IMGOPT STDIMGOPTS[] = {
  { "w",	   RX_OPT_INT },
  { "h",	   RX_OPT_INT },
  { "drawopts",	   RX_OPT_DRAWOPT },
  { "ref",	   RX_OPT_REFTYPE },
  { "showstats",   RX_OPT_INT },
  { "showerrbars", RX_OPT_INT },
  { "xtype",	   RX_OPT_AXISTYPE },
  { "xmin",	   RX_OPT_FLOAT },
  { "xmax",	   RX_OPT_FLOAT },
  { "ytype",	   RX_OPT_AXISTYPE },
  { "ymin",	   RX_OPT_FLOAT },
  { "ymax",	   RX_OPT_FLOAT },
  { "ztype",	   RX_OPT_AXISTYPE },
  { "zmin",	   RX_OPT_FLOAT },
  { "zmax",	   RX_OPT_FLOAT },
  { "ktest",	   RX_OPT_FLOAT },
  { "",		   RX_OPT_INT }

};

static IMGOPT TRENDIMGOPTS[] = {
  { "w",	RX_OPT_INT },
  { "h",	RX_OPT_INT },
  { "trend",	RX_OPT_TREND_TYPE },
  { "xmin",	RX_OPT_FLOAT },
  { "xmax",	RX_OPT_FLOAT },
  { "",		RX_OPT_INT }
};

/** Utility to fetch an image from visDQMRender process.  Validates the
    image request options and builds a spec string to send to the
    backend render process.  Waits for the backend to reply with an
    image and returns either (MimeType, ImageData) tuple on success or
    (None, None) on failure.

    The remaining arguments should be the object name, the streamers
    and the streamed object data, each a string.  The streamers should
    be serialised ROOT TStreamerInfo required to deserialise the data
    (for ROOT objects) or empty (for scalar values).  The data should
    be either serialised ROOT objects or the scalar value as a string.

    The call will wait as long as the render process takes to reply.  A
    persistent per-thread socket is used to talk to the render process.

    PIL is used to scale down images that would be very small, as ROOT
    is not good at drawing small anti-aliased images.

    A cache of recent identical images is maintained, keyed by the spec
    and serialised object data, to avoid unnecessarily re-rendering the
    frequently accessed images. */
class VisDQMRenderLink
{
  static const uint32_t DQM_MSG_GET_IMAGE_DATA  	= 4;
  static const uint32_t DQM_MSG_GET_JSON_DATA		= 6;
  static const uint32_t DQM_MSG_REPLY_IMAGE_DATA 	= 105;
  static const uint32_t DQM_REPLY_JSON_DATA 		= 106;

  static const int MIN_WIDTH = 532;
  static const int MIN_HEIGHT = 400;
  static const int IMAGE_CACHE_SIZE = 1024;

  typedef boost::shared_ptr<SubProcess> SubProcessPtr;

  struct Image;
  struct Server
  {
    Filename		path;
    Filename		sockpath;
    LocalSocket		socket;
    SubProcessPtr	proc;
    std::list<Image *>	pending;
    std::string		lastimg;
    bool		checkme;
  };

  struct Image
  {
    int64_t		id;
    uint64_t		hash;
    uint64_t		version;
    uint32_t		flags;
    uint32_t		tag;
    std::string		pathname;
    std::string		imagespec;
    std::string		databytes;
    std::string		qdata;
    std::string		pngbytes;
    size_t		numparts;
    int			width;
    int			height;
    int			inuse;
    bool		busy;
  };

  Filename		dir_;
  bool			debug_;
  bool			quiet_;
  std::string		plugin_;
  Pipe			logpipe_;
  SubProcess		logger_;
  std::vector<Server>	servers_;
  Image			cache_[IMAGE_CACHE_SIZE];
  pthread_mutex_t	lock_;
  pthread_cond_t	imgavail_;
  pthread_cond_t	srvavail_;
  pthread_t		thread_;
  static VisDQMRenderLink *s_instance;

public:
  static VisDQMRenderLink *
      instance(void) {
    return s_instance;
  }

  VisDQMRenderLink(const std::string &basedir,
                   const std::string &logdir,
		   const std::string &plugin,
		   int nproc,
		   bool debug);

  ~VisDQMRenderLink(void){}

  void
      start(void) {
    pthread_mutex_lock(&lock_);
    quiet_ = false;
    pthread_mutex_unlock(&lock_);
    logme() << "INFO: render link enabled\n";
  }

  void
      stop(void) {
    logme() << "INFO: render link disabled\n";
    pthread_mutex_lock(&lock_);
    quiet_ = true;
    pthread_mutex_unlock(&lock_);
  }

  bool render(std::string &image,
              std::string &type,
              const std::string &path,
              const std::map<std::string, std::string> &opts,
              const std::string *streamers,
              DQMNet::Object *obj,
              size_t numobj,
              IMGOPT *IMGOPTS);
  bool prepareJson(std::string &jsonData,
                   std::string &type,
                   const std::string &path,
                   const std::map<std::string, std::string> &,
                   const std::string *streamers,
                   DQMNet::Object *obj,
                   size_t numobj,
                   IMGOPT *);
  

private:
  static std::ostream &
      logme(void) {
    Time now = Time::current();
    return std::cerr
	<< now.format(true, "%Y-%m-%d %H:%M:%S.")
	<< now.nanoformat(3, 3)
	<< " visDQMRenderLink[" << getpid()
	<< '/' << pthread_self() << "]: ";
  }

  void
      startsrv(Server &srv) {
    assert(srv.pending.empty());
    IOChannel *null = 0;
    const char *serverArgz[] = {
      "visDQMRender", "--state-directory", srv.path.name(),
      "--load", plugin_.c_str(), (debug_ ? "--debug" : 0), 0
    };
    srv.proc.reset(new SubProcess(serverArgz,
                                  SubProcess::First
                                  | SubProcess::Search
                                  | SubProcess::NoCloseOutput
                                  | SubProcess::NoCloseError,
                                  null, logpipe_.sink(),
                                  logpipe_.sink()));
    srv.checkme = true;
  }

  static void
      initimg(Image &proto,
              const std::string &path,
              const std::string &imagespec,
              const std::string *streamers,
              const DQMNet::Object *obj,
              size_t numobj,
              int width  = 0,
              int height = 0);

  static void lrunuke(Image &img);
  Image & lruimg(int &imgidx, Image &proto);
  void blacklistimg(Server &srv);
  void requestimg(Image &img, std::string &imgbytes, const bool json=false);
  void resizeimg(std::string &imgdata, Image &destimg, const Image &srcspec);
  void compress(Image &img, const std::string &imgbytes);
  bool makepng(std::string &imgdata,
              std::string &imgtype,
              Image &protoreq,
              const std::string &spec,
              int width,
              int height);

  bool makeJson(std::string &jsonData,
                std::string &imgtype,
                Image &protoreq,
                const std::string &spec);
  void expandpng(std::string &imgbytes, const Image &img);
};

#endif // DQM_VISDQMRENDERLINK_H
