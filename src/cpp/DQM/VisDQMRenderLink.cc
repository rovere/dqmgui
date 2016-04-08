#include "DQM/VisDQMRenderLink.h"

#include <fstream>


#include "DQM/VisDQMServerTools.h"
#include "DQM/DQMNet.h"
#include "classlib/utils/StringOps.h"
#include "classlib/utils/TimeInfo.h"

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

namespace boost { namespace gil {
  /** Kernel for sinc filter of radius 2.0.  NB: Some say sinc(x) is
      sin(x)/x.  Others say it's sin(PI*x)/(PI*x), a horizontal
      compression of the former which is zero at integer values.  We
      use the latter.  */
  struct sinc_filter
  {
    static float width(void) { return 2.0f; }
    static float filter(float x)
      {
	if (x == 0.0) return 1.0;
	return sinf(M_PI*x)/(M_PI*x);
      }
  };

  /** Kernel for Lanczos filter of radius 3.0. */
  struct lanczos_filter
  {
    static float width(void) { return 3.0f; }
    static float filter(float x)
      {
	if (x < 0.0f)
	  x = -x;
	if (x < 3.0f)
	  return sinc_filter::filter(x) * sinc_filter::filter(x/3.0);
	return 0.0f;
      }
  };

  template <typename DstPixel, typename SrcPixel>
  class blendop
  {
    typedef typename channel_type<DstPixel>::type DstChannel;
    typedef typename channel_type<SrcPixel>::type SrcChannel;
    typedef channel_traits<DstChannel> DstInfo;
    typedef channel_traits<SrcChannel> SrcInfo;
    float f_;
  public:
    blendop(float f) : f_(f) {}
    DstChannel operator()(SrcChannel a, SrcChannel b) const
      { return channel_convert<DstChannel>
	  (std::min(SrcInfo::max_value(),
		    std::max(SrcInfo::min_value(),
			     SrcChannel(a + f_ * (b - a))))); }
  };

  /** Blend two source views @a and @b to a destination view @a dst
      with a blending coefficient @a f.  The result destination image
      will be $a + f * (b - a)$.  Hence f=0 results in source a, f=1
      results in source b, f values (0, 1) blend the sources a and b,
      and f>1 results in boosting source b and subtracting some part
      of source a. */
  template <typename SrcView1, typename SrcView2, typename Scalar, typename DstView>
  inline void blend(const SrcView1 &a, const SrcView2 &b, Scalar f, DstView &dst)
  {
    typedef typename SrcView1::value_type SrcPixel;
    typedef typename DstView::value_type DstPixel;
    assert(a.dimensions() == dst.dimensions());
    assert(b.dimensions() == dst.dimensions());
    int height = a.height();
    for (int rr = 0; rr < height; ++rr)
    {
      typename SrcView1::x_iterator ai = a.row_begin(rr);
      typename SrcView1::x_iterator ae = a.row_end(rr);
      typename SrcView2::x_iterator bi = b.row_begin(rr);
      typename DstView::x_iterator di = dst.row_begin(rr);
      for ( ; ai != ae; ++ai, ++bi, ++di)
	static_transform(*ai, *bi, *di, blendop<DstPixel,SrcPixel>(f));
    }
  }

  /** Sharpen source image @a src by @a amount and write the result
      into destination image @a dst.  Performs the sharpening using an
      operation known as unsharp masking: subtracting a slightly
      smoothed version of the image from the image itself.
      Intermediate results are in 32-bit float pixel format. */
  inline void sharpen(rgb8_view_t &src, rgb8_view_t &dst, float amount)
  {
    static const float SMOOTH[] = { 1./7., 5./7., 1./7. };
    kernel_1d_fixed<float,3> smooth(SMOOTH,1);
    rgb32f_image_t smoothed(src.width(), src.height());
    copy_and_convert_pixels(src, view(smoothed));
    convolve_rows_fixed<rgb32f_pixel_t>
      (const_view(smoothed), smooth, view(smoothed));
    convolve_cols_fixed<rgb32f_pixel_t>
      (const_view(smoothed), smooth, view(smoothed));
    blend(const_view(smoothed),
	  color_converted_view<rgb32f_pixel_t>(src),
	  amount, dst);
  }
}}


VisDQMRenderLink::VisDQMRenderLink(const std::string &basedir,
                                   const std::string &logdir,
                                   const std::string &plugin,
                                   int nproc,
                                   bool debug)
{
  assert(! s_instance);
  s_instance = this;
  plugin_ = plugin;
  debug_ = debug;
  dir_ = basedir;
  quiet_ = false;

  // Initialise cache to 5000 unused images.
  for (int i = 0; i < IMAGE_CACHE_SIZE; ++i)
    lrunuke(cache_[i]);

  // Initialise requested number of sub-processes, all feeding to one logger.
  std::string logout(logdir + "/renderlog-%Y%m%d.log");
  logger_.run(Argz("rotatelogs", logout.c_str(), "86400").argz(),
              SubProcess::Write | SubProcess::NoCloseError | SubProcess::Search,
              &logpipe_);

  servers_.resize(nproc);
  for (int i = 0; i < nproc; ++i)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "render-%d", i);
    servers_[i].path = Filename(basedir, buf);
    servers_[i].sockpath = Filename(servers_[i].path, "socket");
    Filename::makedir(servers_[i].path, 0755, true, true);
    Filename::remove(servers_[i].sockpath, false, true);
    startsrv(servers_[i]);
  }

  // Initialise locks for image cache and server list.
  pthread_mutex_init(&lock_, 0);
  pthread_cond_init(&imgavail_, 0);
  pthread_cond_init(&srvavail_, 0);
}

void VisDQMRenderLink::start(void)
{
  pthread_mutex_lock(&lock_);
  quiet_ = false;
  pthread_mutex_unlock(&lock_);
  logme() << "INFO: render link enabled\n";
}

void VisDQMRenderLink::stop(void)
{
  logme() << "INFO: render link disabled\n";
  pthread_mutex_lock(&lock_);
  quiet_ = true;
  pthread_mutex_unlock(&lock_);
}

bool VisDQMRenderLink::render(std::string &image,
                              std::string &type,
                              const std::string &path,
                              const std::map<std::string, std::string> &opts,
                              const std::string *streamers,
                              DQMNet::Object *obj,
                              size_t numobj,
                              IMGOPT *IMGOPTS)
{
  // Verify incoming arguments and build perliminary specification.
  std::string finalspec;
  std::string reqspec;
  int destwidth = 0;
  int destheight = 0;
  for (int opt = 0; ! IMGOPTS[opt].label.empty(); ++opt)
  {
    std::map<std::string, std::string>::const_iterator pos
        = opts.find(IMGOPTS[opt].label);
    if (pos != opts.end())
    {
      if (! IMGOPTS[opt].verify.match(pos->second)
          && !(IMGOPTS[opt].label == "drawopts" && pos->second.empty()))
      {
        logme() << "WARNING: option '" << pos->first << "' value '"
                << pos->second << "' does not pass verification\n";
        return false;
      }

      if (opt == 0) // w
      {
        destwidth = atoi(pos->second.c_str());
        continue;
      }

      if (opt == 1) // h
      {
        destheight = atoi(pos->second.c_str());
        continue;
      }

      if (! reqspec.empty())
        reqspec += ';';
      reqspec += pos->first;
      reqspec += '=';
      reqspec += pos->second;
    }
  }

  // If absurd image size was requested, return nothing.
  if (destwidth <= 0 || destheight <= 0
      || destwidth >= 2500 || destheight >= 2500)
    return false;

  // If the requested image is small, generate a larger image in
  // the space aspect ratio and use rescale the image with high
  // quality (lanczos) anti-aliasing resizing filter. ROOT itself
  // generates hideously ugly output for small images.
  int width = destwidth;
  int height = destheight;
  char buf[64];
  sprintf(buf, "%sw=%d;h=%d",
          reqspec.empty() ? "" : ";",
          destwidth, destheight);
  finalspec = reqspec + buf;

  if (width < MIN_WIDTH || height < MIN_HEIGHT)
  {
    double wratio = double(MIN_WIDTH) / double(width);
    double hratio = double(MIN_HEIGHT) / double(height);
    if (wratio > hratio)
    {
      width = MIN_WIDTH;
      height = int(height * wratio + .5);
    }
    else
    {
      height = MIN_HEIGHT;
      width = int(width * hratio + .5);
    }
  }

  // If absurd image sizes were re-computed due to some weirdly
  // formatted request (extremely large ratio), return nothing.
  if (width >= 2500 || height >= 2500)
    return false;

  sprintf(buf, "%sw=%d;h=%d",
          reqspec.empty() ? "" : ";",
          width, height);
  reqspec += buf;

  VisDQMRenderLink::Image protoimg;
  initimg(protoimg, path, reqspec, streamers, obj, numobj, width, height);

  // See if we have the image already cached. If we are going to
  // resize the image, we will try both image specifications, in
  // the order we need them.
  Lock gate(&lock_);
  return ! quiet_ && makepng(image, type, protoimg, finalspec, destwidth, destheight);
}

bool VisDQMRenderLink::prepareJson(std::string &jsonData,
                                   std::string &type,
                                   const std::string &path,
                                   const std::map<std::string, std::string> &,
                                   const std::string *streamers,
                                   DQMNet::Object *obj,
                                   size_t numobj,
                                   IMGOPT *)
{
  std::string reqspec;
  VisDQMRenderLink::Image protoimg;
  initimg(protoimg, path, reqspec, streamers, obj, numobj);
  Lock gate(&lock_);
  return ! quiet_ && makeJson(jsonData, type, protoimg, reqspec);
}

void VisDQMRenderLink::startsrv(Server &srv)
{
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

void VisDQMRenderLink::initimg(VisDQMRenderLink::Image &proto,
                               const std::string &path,
                               const std::string &imagespec,
                               const std::string *streamers,
                               const DQMNet::Object *obj,
                               size_t numobj,
                               int width  /* = 0 */,
                               int height /* = 0 */)
{
  assert(numobj);
  lrunuke(proto);
  proto.id = Time::current().ns();
  proto.version = obj[0].version;
  proto.flags = obj[0].flags;
  proto.tag = obj[0].tag;
  proto.pathname = path;
  proto.qdata = obj[0].qdata;
  proto.imagespec = imagespec;
  proto.numparts = numobj;
  proto.width = width;
  proto.height = height;

  size_t datalen = 0;
  for (size_t i = 0; i < numobj; ++i)
  {
    assert(obj[i].flags == 0 // totally missing
           || isScalarType(obj[i].flags)
           || isROOTType(obj[i].flags)
           || isBLOBType(obj[i].flags));
    assert(i == 0 || !isScalarType(obj[i].flags));
    assert(i == 0
           || (isBLOBType(obj[0].flags) && numobj <= 2) // stripchart
           || getType(obj[i].flags) == getType(obj[0].flags)); // other

    if (isROOTType(obj[i].flags) || isBLOBType(obj[i].flags))
      datalen += 2*sizeof(uint32_t) + streamers[i].size() + obj[i].rawdata.size();
    else
      datalen += obj[i].scalar.size();
  }

  proto.databytes.reserve(datalen);
  for (size_t i = 0; i < numobj; ++i)
  {
    if (isROOTType(obj[i].flags) || isBLOBType(obj[i].flags))
    {
      uint32_t words[2];
      words[0] = streamers[i].size();
      words[1] = obj[i].rawdata.size();
      proto.databytes.append((const char *) words, sizeof(words));
      proto.databytes.append(streamers[i]);
      if (obj[i].rawdata.size())
        proto.databytes.append((const char *) &obj[i].rawdata[0],
                               obj[i].rawdata.size());
    }
    else
      proto.databytes = obj[i].scalar;
  }

  proto.hash = (((uint64_t) DQMNet::dqmhash(&path[0], path.size()) << 32)
                | DQMNet::dqmhash(&proto.databytes[0], datalen));
}

void VisDQMRenderLink::lrunuke(VisDQMRenderLink::Image &img)
{
  img.id = 0;
  img.hash = 0;
  img.version = 0;
  img.flags = 0;
  img.tag = 0;
  img.pathname.clear();
  std::string().swap(img.imagespec);
  std::string().swap(img.databytes);
  std::string().swap(img.qdata);
  std::string().swap(img.pngbytes);
  img.numparts = 0;
  img.width = 0;
  img.height = 0;
  img.busy = false;
  img.inuse = 0;
}

VisDQMRenderLink::Image & VisDQMRenderLink::lruimg(int &imgidx,
                                                   VisDQMRenderLink::Image &proto)
{
  int lruidx = -1;
  imgidx = -1;

  for (int i = 0; i < IMAGE_CACHE_SIZE; ++i)
  {
    if (cache_[i].id
        && cache_[i].hash == proto.hash
        && cache_[i].width == proto.width
        && cache_[i].height == proto.height
        && cache_[i].databytes == proto.databytes
        && cache_[i].qdata == proto.qdata
        && cache_[i].pathname == proto.pathname
        && cache_[i].imagespec == proto.imagespec)
    {
      imgidx = i;
      break;
    }
    else if (cache_[i].id == 0 && (lruidx < 0 || cache_[lruidx].id))
      lruidx = i;
    else if (cache_[i].inuse)
      continue;
    else if (lruidx < 0 || cache_[i].id < cache_[lruidx].id)
      lruidx = i;
  }

  assert(imgidx >= 0 || lruidx >= 0);
  if (imgidx < 0)
  {
    assert(! cache_[lruidx].inuse);
    assert(! cache_[lruidx].busy);
    lrunuke(cache_[lruidx]);
    return cache_[lruidx];
  }
  else
    return cache_[imgidx];
}

void VisDQMRenderLink::blacklistimg(Server &srv)
{
  typedef std::map<std::string, Time> BlackList;

  if (srv.lastimg.empty())
    return;

  BlackList bad;
  Filename blacklist(dir_, "blacklist.txt");
  if (blacklist.exists() && blacklist.isReadable())
  {
    std::string line;
    std::ifstream f(blacklist);
    do
    {
      getline(f, line);
      if (line.empty())
        continue;

      StringList s = StringOps::split(line, ' ', 0, 2, 0, -1);
      bad[s[1]] = Time::ValueType(atof(s[0].c_str())*1e9);
    } while (f);
  }

  bad[srv.lastimg] = Time::current();
  std::ofstream f(blacklist);
  for (BlackList::iterator i = bad.begin(), e = bad.end(); i != e; ++i)
    f << i->second << ' ' << i->first << std::endl;
}

void VisDQMRenderLink::requestimg(VisDQMRenderLink::Image &img,
                                  std::string &imgbytes,
                                  const bool json /* = false */)
{
  assert(imgbytes.empty());

  // Pick the least loaded server to talk to.  While we do that,
  // check the servers are still running; restart those that are
  // not running yet or any more.  Prefer already running server
  // to one we just restarted, as the former will have a socket
  // ready.
  size_t proc = servers_.size();
  size_t maybe = servers_.size();
  for (size_t i = 0; i < servers_.size(); ++i)
  {
    Server &srv = servers_[i];
    SubProcessPtr p = srv.proc;
    if (srv.checkme && p && p->done())
    {
      pid_t pid = p->pid();
      int code = p->wait();
      char buf[128];
      if (! p->exitBySignal(code))
        sprintf(buf, " exited with code %d", p->exitStatus(code));
      else
        sprintf(buf, " was killed by signal %d", p->exitSignal(code));

      logme() << "WARNING: restarting render process #"
              << i << ", process " << pid
              << buf << " while rendering '"
              << srv.lastimg << "'\n";
      blacklistimg(srv);
      srv.socket.abort();
      srv.proc.reset();
      srv.pending.clear();
      p.reset();
    }

    if (! p)
    {
      startsrv(srv);
      if (maybe == servers_.size())
        maybe = i;
    }
    else if (proc == servers_.size()
             || srv.socket.fd() == IOFD_INVALID
             || (srv.pending.size() < servers_[proc].pending.size()))
      proc = i;
  }

  assert(proc < servers_.size() || maybe < servers_.size());
  Server &srv = servers_[proc < servers_.size() ? proc : maybe];
  LocalSocket &sock = srv.socket;
  if (sock.fd() == IOFD_INVALID)
  {
    // Wait up to one second to connect to the server.
    int ntry;
    for (ntry = 10; ntry >= 0; TimeInfo::msleep(100), --ntry)
      if (srv.sockpath.exists())
        break;

    try
    {
      if (ntry >= 0)
      {
        sock.create();
        sock.connect(srv.sockpath.name());
      }
    }
    catch (Error &e)
    {
      logme() << "WARNING: failed to connect to server #" << maybe
              << ": " << e.explain () << std::endl;
      sock.abort();
    }

    if (sock.fd() == IOFD_INVALID)
    {
      logme() << "WARNING: no connection to server #" << maybe << '\n';
      srv.checkme = true;
      return;
    }
  }

  // Wait for the socket to become available, issue an image
  // request and wait for result.  While waiting, release the lock
  // so others can do work.
  assert(sock.fd() != IOFD_INVALID);
  assert(img.inuse > 0);
  assert(img.busy);
  srv.lastimg = img.pathname;
  srv.pending.push_back(&img);

  while (srv.pending.size() && srv.pending.front() != &img)
    pthread_cond_wait(&srvavail_, &lock_);

  if (srv.pending.empty())
  {
    srv.checkme = true;
    return;
  }

  pthread_mutex_unlock(&lock_);

  try
  {
    uint32_t words[11] =
        {
          (uint32_t)(sizeof(words) + img.pathname.size() + img.imagespec.size()
                     + img.databytes.size() + img.qdata.size()),
          (json ? DQM_MSG_GET_JSON_DATA : DQM_MSG_GET_IMAGE_DATA),
          img.flags,
          img.tag,
          (uint32_t)((img.version >> 0 ) & 0xffffffff),
          (uint32_t)((img.version >> 32) & 0xffffffff),
          (uint32_t)img.numparts,
          (uint32_t)img.pathname.size(),
          (uint32_t)img.imagespec.size(),
          (uint32_t)img.databytes.size(),
          (uint32_t)img.qdata.size()
        };

    std::string message;
    message.reserve(words[0]);
    message.append((const char *)&words[0], sizeof(words));
    message.append(img.pathname);
    message.append(img.imagespec);
    message.append(img.databytes);
    message.append(img.qdata);
    sock.xwrite(&message[0], message.size());
    message.clear();

    if (sock.xread(&words[0], 2*sizeof(uint32_t)) == 2*sizeof(uint32_t)
        && words[0] >= 2*sizeof(uint32_t)
        && (words[1] == DQM_MSG_REPLY_IMAGE_DATA || words[1] == DQM_REPLY_JSON_DATA ))
    {
      message.resize(words[0] - 2*sizeof(uint32_t), '\0');
      if (sock.xread(&message[0], message.size()) == message.size())
        imgbytes = message;
    }
  }
  catch (Error &e)
  {
    logme() << "ERROR: failed to retrieve image: " << e.explain() << std::endl;
    sock.abort();
  }

  // Reacquire locks and wake up waiters.
  pthread_mutex_lock(&lock_);
  assert(img.inuse > 0);
  assert(img.busy);
  srv.checkme = true;
  if (sock.fd() == IOFD_INVALID)
    srv.pending.clear();
  else
  {
    if (! imgbytes.empty())
    {
      srv.checkme = false;
      srv.lastimg.clear();
      if (!json)
        compress(img, imgbytes);
    }
    srv.pending.pop_front();
  }
  pthread_cond_broadcast(&srvavail_);
}

void VisDQMRenderLink::resizeimg(std::string &imgdata,
                                 VisDQMRenderLink::Image &destimg,
                                 const VisDQMRenderLink::Image &srcspec)
{
  using namespace boost::gil;
  std::string newdata;
  assert(destimg.busy);
  assert(destimg.inuse);
  assert(destimg.pngbytes.empty());
  assert(imgdata.size() == size_t(srcspec.width) * size_t(srcspec.height) * 3);
  pthread_mutex_unlock(&lock_);
  try
  {
    newdata.resize(destimg.width * destimg.height * 3, '\x00');
    rgb8c_view_t srcview
        = interleaved_view(srcspec.width, srcspec.height,
                           (rgb8c_pixel_t *) &imgdata[0],
                           srcspec.width * 3);
    rgb8_view_t destview
        = interleaved_view(destimg.width, destimg.height,
                           (rgb8_pixel_t *) &newdata[0],
                           destimg.width * 3);
    rescale(srcview, destview, catmull_rom_filter()); // lanczos_filter()
    sharpen(destview, destview, 1.4);
  }
  catch (std::exception &e)
  {
    // Ignore actual error, we handle the situation below.
    logme() << "WARNING: failed to scale image: "
            << e.what() << std::endl;
    std::string().swap(newdata);
  }
  catch (...)
  {
    logme() << "WARNING: failed to scale image, unknown reason\n";
    std::string().swap(newdata);
  }

  imgdata.swap(newdata);
  pthread_mutex_lock(&lock_);
}

void VisDQMRenderLink:: compress(VisDQMRenderLink::Image &img,
                                 const std::string &imgbytes)
{
  assert(! imgbytes.empty());
  assert(img.pngbytes.empty());
  assert(img.inuse);
  assert(img.busy);
  FILE *png = 0;
  char *pngdata = 0;
  size_t pngsize = 0;
  std::string pngbytes;
  pthread_mutex_unlock(&lock_);

  try
  {
    if ((png = open_memstream(&pngdata, &pngsize)))
    {
      using namespace boost::gil;
      boost::gil::detail::png_writer writer(png);
      writer.apply(interleaved_view
                   (img.width, img.height,
                    (const rgb8_pixel_t *) &imgbytes[0],
                    img.width * 3));
      fflush(png);
      pngbytes.append(pngdata, pngsize);
      fclose(png);
      png = 0;
    }
  }
  catch (std::exception &e)
  {
    logme() << "WARNING: failed to encode png image: "
            << e.what() << std::endl;
    std::string().swap(pngbytes);
  }
  catch (...)
  {
    logme() << "WARNING: failed to encode png image, unknown reason\n";
    std::string().swap(pngbytes);
  }

  if (png)
    fclose(png);

  if (pngdata)
    free(pngdata);

  pthread_mutex_lock(&lock_);
  img.pngbytes = pngbytes;
}

bool VisDQMRenderLink::makepng(std::string &imgdata,
                               std::string &imgtype,
                               VisDQMRenderLink::Image &protoreq,
                               const std::string &spec,
                               int width,
                               int height)
{
  imgdata.clear();
  imgtype.clear();
  int imgidx;
  VisDQMRenderLink::Image proto = protoreq;
  proto.id = Time::current().ns();
  proto.imagespec = spec;
  proto.width = width;
  proto.height = height;
  VisDQMRenderLink::Image &img = lruimg(imgidx, proto);
  if (imgidx >= 0)
    img.inuse++;
  else
  {
    std::string srcbytes;
    assert(! img.busy);
    assert(! img.inuse);
    assert(img.pngbytes.empty());
    img = proto;
    img.busy = true;
    img.inuse++;

    // If we are not rescaling, request and compress image.
    if (width == protoreq.width && height == protoreq.height)
      requestimg(img, srcbytes);

    // Otherwise, we are rescaling. First get original bigger image,
    // then rescale and compress the image. We might either find the
    // original as-is, in which case we need to expand the PNG form,
    // or create it, in which case we get the raw bytes for free.
    else
    {
      VisDQMRenderLink::Image &srcimg = lruimg(imgidx, protoreq);
      if (imgidx >= 0)
        srcimg.inuse++;
      else
      {
        assert(! srcimg.busy);
        assert(! srcimg.inuse);
        assert(srcimg.pngbytes.empty());
        srcimg = protoreq;
        srcimg.busy = true;
        srcimg.inuse++;
        requestimg(srcimg, srcbytes);
        assert(srcimg.inuse > 0);
        assert(srcimg.busy);
        srcimg.busy = false;
        pthread_cond_broadcast(&imgavail_);
      }

      while (srcimg.busy)
        pthread_cond_wait(&imgavail_, &lock_);

      assert(srcimg.width == protoreq.width);
      assert(srcimg.height == protoreq.height);
      if (srcbytes.empty() && ! srcimg.pngbytes.empty())
        expandpng(srcbytes, srcimg);

      assert(srcimg.inuse > 0);
      srcimg.inuse--;
      assert(img.inuse > 0);
      if (! srcbytes.empty())
        resizeimg(srcbytes, img, protoreq);

      if (! srcbytes.empty())
        compress(img, srcbytes);
    }

    assert(img.inuse > 0);
    assert(img.busy);
    img.busy = false;
    pthread_cond_broadcast(&imgavail_);
  }

  while (img.busy)
    pthread_cond_wait(&imgavail_, &lock_);

  assert(img.inuse > 0);
  img.inuse--;
  if (! img.pngbytes.empty())
  {
    imgdata = img.pngbytes;
    imgtype = "image/png";
    return true;
  }
  else
    return false;
}

bool VisDQMRenderLink::makeJson(std::string &jsonData,
                                std::string &imgtype,
                                VisDQMRenderLink::Image &protoreq,
                                const std::string &spec)
{
  jsonData.clear();
  imgtype.clear();
  VisDQMRenderLink::Image proto = protoreq;
  proto.id = Time::current().ns();
  proto.imagespec = spec;
  VisDQMRenderLink::Image &img = proto;

  assert(! img.busy);
  assert(! img.inuse);
  assert(img.pngbytes.empty());
  img.busy = true;
  img.inuse++;
  requestimg(img, jsonData, true);

  assert(img.inuse > 0);
  assert(img.busy);
  img.busy = false;
  pthread_cond_broadcast(&imgavail_);

  while (img.busy)
    pthread_cond_wait(&imgavail_, &lock_);

  assert(img.inuse > 0);
  img.inuse--;

  return true;
}

void VisDQMRenderLink::expandpng(std::string &imgbytes,
                                 const VisDQMRenderLink::Image &img)
{
  // FIXME: Lock, unlock?
  FILE *png = 0;
  try
  {
    imgbytes.resize(img.width * img.height * 3, '\0');
    if ((png = fmemopen(const_cast<char *>(&img.pngbytes[0]),
                        img.pngbytes.size(), "r")))
    {
      using namespace boost::gil;
      boost::gil::detail::png_reader reader(png);
      reader.apply(interleaved_view
                   (img.width, img.height,
                    (rgb8_pixel_t *) &imgbytes[0],
                    img.width * 3));
      fclose(png);
      png = 0;
    }
  }
  catch (std::exception &e)
  {
    logme() << "WARNING: failed to decode png image: "
            << e.what() << std::endl;
    imgbytes.clear();
  }
  catch (...)
  {
    logme() << "WARNING: failed to decode png image, unknown reason\n";
    imgbytes.clear();
  }

  if (png)
    fclose(png);
}
