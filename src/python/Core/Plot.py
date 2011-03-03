from matplotlib.figure import Figure
from matplotlib.transforms import nonsingular
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import Locator, Formatter, ScalarFormatter
from matplotlib.patches import Wedge, Shadow, Rectangle, Polygon, Circle
from matplotlib.lines import Line2D
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.backends.backend_svg import FigureCanvasSVG
from matplotlib.backends.backend_pdf import FigureCanvasPdf
from matplotlib.backends.backend_ps import FigureCanvasPS
from matplotlib.pylab import setp
from matplotlib.cm import Accent
from matplotlib.numerix import ma
from matplotlib import cm
from cStringIO import StringIO
from time import strftime, gmtime
from tempfile import mkstemp
from Monitoring.Core.Utils import thousands, _loginfo, _logerr, _logwarn
import math, numpy, os, colorsys, pylab
from PIL import Image
import re
from copy import copy
import random

# Utility function to convert text RGB colours to RGB triple in range [0, 1]
def text2rgb(str):
  return [ int(str[i:i+2], 16)/255. for i in xrange(1, len(str), 2) ]

def makeThumbnail(plot,x=100,y=75):
  png_header = '\x89\x50\x4e\x47\x0d\x0a\x1a\x0a'
  if plot.startswith(png_header):
    plotfile = StringIO(plot)
    plotimg = Image.open(plotfile)
    plotimg.thumbnail((x,y),Image.ANTIALIAS)
    f=StringIO()
    plotimg.save(f,'PNG')
    return f.getvalue()
  return ''

def stitchPlotAndLegend(plot,legend):
  """
  Simple implementation.
  Requires that the 'png' imagetype is used. Extending this for SVG, PDF, PS etc would require presumably
  some digging into the Figure backends for matplotlib, which gets a bit byzantine...
  """
  png_header = '\x89\x50\x4e\x47\x0d\x0a\x1a\x0a'
  if plot.startswith(png_header) and legend.startswith(png_header):
    plotfile = StringIO(plot)
    legendfile = StringIO(legend)

    plotimg = Image.open(plotfile)
    legendimg = Image.open(legendfile)

    composite = Image.new(plotimg.mode,(plotimg.size[0],plotimg.size[1]+legendimg.size[1]))
    composite.paste(plotimg,(0,0,plotimg.size[0],plotimg.size[1]))
    composite.paste(legendimg,(0,plotimg.size[1],plotimg.size[0],plotimg.size[1]+legendimg.size[1]))
    f = StringIO()
    composite.save(f,'PNG')
    return f.getvalue()
  # Now supports SVG images as well, since they're nice and simple text formats
  # and we can just use RE's to increment all the y coordinates in the legend
  # Note this is not a general SVG stitching method, it will only work providing the
  # format matches exactly what matplotlib outputs (otherwise we'll need to use xml.dom)
  elif '!DOCTYPE svg' in plot[0:100] and '!DOCTYPE svg' in legend[0:100]:
    try:
      plot = copy(plot)
      legend = copy(legend)
      re_svg = re.compile("<svg width=\"([0-9]+)\" height=\"([0-9]+)\" viewBox=\"0 0 ([0-9]+) ([0-9]+)\"")
      plot_y = int(re_svg.search(plot).group(2))
      legend_y = int(re_svg.search(legend).group(2))
      new_y = plot_y+legend_y
      plot = re_svg.sub(r'<svg width="\1" height="%s" viewBox="0 0 \3 %s"' % (new_y,new_y),plot)
      re_legcontent = re.compile("<g id=\"figure1\">(.*?)</g>",re.DOTALL)
      legend = re_legcontent.search(legend).group(1)
      re_polypoints = re.compile("<polygon(.*?)points = \"(.*?)\"")
      re_text_y = re.compile("<text(.*?)y=\"(.*?)\"")
      def polyfix(match):
        points = match.group(2)
        results = []
        for p in points.split(" "):
          x,y = map(float,p.split(','))
          y += plot_y
          results.append("%f,%f"%(x,y))
        return r'<polygon%spoints = "%s"' % (match.group(1),' '.join(results))
      def textfix(match):
        y = float(match.group(2))
        return r'<text%sy="%s"' % (match.group(1),y+plot_y)
      legend = re_polypoints.sub(polyfix,legend)
      legend = re_text_y.sub(textfix,legend)
      plot = plot.replace('</svg>','<g id="legend">%s</g>\n</svg>' % (legend))
      return plot
    except re.error:
      return 'ERROR: Regex error during plot-legend stitching'
  else:
    #raise NotImplemented
    return ''

def SIFormatter(val,unit='',long=False):
  suffix = [(1e18,'E','exa'),(1e15,'P','peta'),(1e12,'T','tera'),(1e9,'G','giga'),(1e6,'M','mega'),(1e3,'k','kilo'),(1,'',''),(1e-3,'m','mili'),(1e-6,'u','micro'),(1e-9,'n','nano'),(1e-12,'p','pico'),(1e-15,'f','femto'),(1e-18,'a','atto')]
  use = 1
  if long:
    use = 2
  for s in suffix:
    if abs(val)>=100*s[0]:
      return "%.0f%s%s"%(val/s[0],s[use],unit)
    if abs(val)>=10*s[0]:
      return "%.1f%s%s"%(val/s[0],s[use],unit)
    if abs(val)>=s[0]:
      return "%.2f%s%s"%(val/s[0],s[use],unit)
  return str(val)

def BinFormatter(val,unit='',long=False):
  suffix = [(2.**60,'E','exa'),(2.**50,'P','peta'),(2.**40,'T','tera'),(2.**30,'G','giga'),(2.**20,'M','mega'),(2.**10,'k','kilo'),(1.,'','')]
  use = 1
  if long:
    use = 2
  for s in suffix:
    if abs(val)>=100*s[0]:
      return "%.0f%s%s"%(val/s[0],s[use],unit)
    if abs(val)>=10*s[0]:
      return "%.1f%s%s"%(val/s[0],s[use],unit)
    if abs(val)>=s[0]:
      return "%.2f%s%s"%(val/s[0],s[use],unit)
  return str(val)


# Format axis value as a byte integer number with thousand separators.
class ByteScalarFormatter(Formatter):
  def __call__(self, val, pos=None):
    return thousands("%d" % val)

# Add thousand separators to normal scalar formatter.
class ThousandScalarFormatter(ScalarFormatter):
  def __call__(self, val, pos=None):
    return thousands(ScalarFormatter.__call__(self, val, pos))

# Display a number as a percent.
class PercentFormatter(ScalarFormatter):
  def __call__(self, val, pos=None):
    return "%d%%" % round(val*100)

# For axis range vmin, vmax, finds the largest power of two >= vmax,
# then places ticks linearly between vmin and vmax at N*power-of-two
# positions for integer N such that there are no more than 8 ticks.
# The axis scale range is assumed to be integer and positive.
class ByteScalarLocator(Locator):
  def __call__(self):
    vmin, vmax = self.axis.get_view_interval()
    if vmin > vmax:
      vmin, vmax = vmax, vmin

    rmax = (vmax >= 1 and 2**math.ceil(math.log(vmax, 2))) or 0
    rmin = (vmin >= 1 and 2**math.floor(math.log(vmin, 2))) or 0
    step = max(1, int((rmax - rmin)/8))
    rmin += math.ceil((vmin - rmin)/step)
    ticks = numpy.arange(rmin, rmax+0.001, step)
    return ticks

  def autoscale(self):
    self.verify_intervals()
    dmin, dmax = self.axis.get_data_interval()
    if dmin > dmax:
      dmin, dmax = dmax, dmin
    if dmin == dmax:
      dmin -= 1
      dmax += 1
    exp, rem = divmod(math.log(dmax - dmin, 2), 1)
    if rem > 0.5:
      exp -= 1
    scale = 2**(-exp)
    dmin = math.floor(scale*dmin)/scale
    dmax = math.ceil(scale*dmax)/scale
    return nonsingular(dmin, dmax)

# Place time series axis tick marks at intervals adapted
# to the time span unit and the length of the axis in time.
class TimeSeriesLocator(Locator):
  def pick(self, series, spec):
    for (interval, roundto) in spec:
      if len(series) / interval < 12:
        break
    base = [i for i in xrange(0, interval) if series[i][0] % roundto == 0][0]
    self.ticks = [t[0] for t in series[base::interval]]

  def __init__(self, span, series):
    if span == 'hour':
      self.pick(series, [(1, 3600), (4, 14400), (8, 14400), (12, 14400), (24, 14400), (48, 86400)])
    elif span == 'day':
      self.pick(series, [(1, 86400), (7, 7*86400), (14, 7*86400), (28, 7*86400), (56, 14*86400)])
    elif span == 'week':
      self.pick(series, [(1, 1), (4, 1), (13, 1), (26, 1), (52, 1)])
    elif span == 'month':
      self.pick(series, [(1, 1), (4, 1), (6, 1), (12, 1), (18, 1), (24, 1), (36, 1), (48, 1)])
    else:
      self.pick(series, [(1, 1), (2, 1), (4, 1), (8, 1), (16, 1), (32, 1), (64, 1)])

  def __call__(self):
    return self.ticks

  def autoscale(self):
    return self.dataInterval.get_bounds()

class TimeSeriesFormatter(Formatter):
  def __init__(self, format):
    self.format = format

  def __call__(self, val, pos=None):
    return strftime(self.format, gmtime(val))

# Chart base class.
class Chart:
  def __init__(self, props, subdefaults):
    self.file = None
    self.filename = None
    self.props = {
      "imageType":	  "png",	# Output image type.
      "dpi":		  72,		# Image DPI resolution.
      "width":		  800,		# Output image width in pixels (vs. DPI).
      "height":		  600,		# Output image height in pixels (vs. DPI).
      "padding":	  50,		# Padding around the figure edge, in pixels.
      "textPadding":	  3,		# Padding around text, in pixels.
      "fontFamily":	  "sans-serif",	# Font family.
      "fontName":	  "Luxi Sans",	# Font name.
      "fontSize":	  12,		# Size of non-title text, in pixels.
      "titleFontName":	  "Bitstream Vera Sans",
      "titleFontSize":	  18,		# Size of title text, in pixels.
      "subtitleFontSize": 14,		# Size of subtitle text, in pixels.
      "axtitleFontSize":  16,		# Size of axis title text, in pixels.
      "xAxisScale":	  "lin",	# X axis scale, lin or log.
      "yAxisScale":	  "lin",	# Y axis scale, lin or log.
      "square":		  False,		# Force square layout.
      "integral":   False, #force integral display on legend
      "notitle": False, #suppress title
      "nolabels": False, #suppress axis labels
      "sort":False, #sort items and legend entries by value
      "max_legend":100 #maximum items in the legend
    }
    for (k, v) in subdefaults.iteritems():
      self.props[k] = v
    for (k, v) in props.iteritems():
      self.props[k] = v

    self.font = FontProperties()
    self.font.set_family(self.get('fontFamily'))
    self.font.set_name(self.get('fontName'))
    self.font.set_size(float(self.get('fontSize')))
    self.tfont = FontProperties()
    self.tfont.set_family(self.get('titleFontFamily', self.font.get_family()[-1]))
    self.tfont.set_name(self.get('titleFontName', self.font.get_name()))
    self.tfont.set_size(self.get('titleFontSize', self.font.get_size()))
    self.tfont.set_weight('bold')
    self.sfont = FontProperties()
    self.sfont.set_family(self.get('subtitleFontFamily', self.tfont.get_family()[-1]))
    self.sfont.set_name(self.get('subtitleFontName', self.tfont.get_name()))
    self.sfont.set_size(self.get('subtitleFontSize', self.tfont.get_size()))
    self.afont = FontProperties()
    self.afont.set_family(self.get('axtitleFontFamily', self.tfont.get_family()[-1]))
    self.afont.set_name(self.get('axtitleFontName', self.tfont.get_name()))
    self.afont.set_size(self.get('axtitleFontSize', self.tfont.get_size()))
    self.afont.set_weight('bold')

  def __del__(self):
    if self.file != None:
      os.close(self.file)

  def get(self, key, default=None):
    return getattr(self, key, self.props.get(key, default))

  def draw(self):
    (fig, canvas, w, h) = self.canvas()
    fig.text(.5, .5, "No data.", horizontalalignment='center', fontproperties=self.font)
    return self.save(fig, canvas)

  def legend(self):
    (fig, canvas, w, h) = self.canvas()
    fig.text(.5, .5, "No legend.", horizontalalignment='center', fontproperties=self.font)
    return self.save(fig, canvas)

  def details(self):
    return {}

  def canvas(self):
    type = self.get("imageType", "png")
    fig = Figure()
    if type == "png":
      canvas = FigureCanvasAgg(fig)
      (self.file, self.filename) = mkstemp(".%s" % type)
    elif type == "svg":
      canvas = FigureCanvasSVG(fig)
      (self.file, self.filename) = mkstemp(".%s" % type)
    elif type == "pdf":
      canvas = FigureCanvasPdf(fig)
      (self.file, self.filename) = mkstemp(".%s" % type)
    elif type == "ps" or type == "eps":
      canvas = FigureCanvasPS(fig)
      (self.file, self.filename) = mkstemp(".%s" % type)
    else:
      raise "Invalid render target requested"

    # Set basic figure parameters
    dpi = float(self.get('dpi'))
    (w, h) = (float(self.get('width')), float(self.get('height')))
    (win, hin) = (w/dpi, h/dpi)
    fig.set_size_inches(win, hin)
    fig.set_dpi(dpi)
    fig.set_facecolor('white')
    return (fig, canvas, w, h)

  def save(self, fig, canvas):
    canvas.draw()
    if not self.filename:
      canvas.print_figure(self.file, dpi=float(self.get('dpi')))
      return self.file
    else:
      canvas.print_figure(self.filename, dpi=float(self.get('dpi')))
      f = file(self.filename, "r") # PS backend writes over the file
      os.remove(self.filename)     #    return os.fdopen(self.file)
      os.close(self.file)
      self.file = None
      self.filename = None
      return f

  def prepare(self):
    # Create canvas and determine figure parameters
    (fig, canvas, w, h) = self.canvas()
    dpif = float(self.get('dpi')) / 72
    padding = float(self.get('padding')) * dpif / h
    textPadding = float(self.get('textPadding')) * dpif
    titleFontSize = float(self.get('titleFontSize'))
    axFontSize = float(self.get('axtitleFontSize'))
    subFontSize = float(self.get('subtitleFontSize'))
    title = self.get("title", "").split("\n")
    if not self.get("notitle",False):
      hsub = (len(title)-1) * (subFontSize * dpif + 4) + textPadding
      htitle = hsub + titleFontSize * dpif + textPadding * 2
    else:
      hsub = 0
      htitle = 0
    if self.get("nolabels",False):
      padding = 0

    # Configure axes
    if self.get('square'):
      minsize = 1 - 2*padding
      axrect = (.5 - minsize/2 * h/w, padding, h/w * minsize, minsize)
    else:
      axrect = (padding, padding, 1 - 1.25*padding, 1 - htitle/h - padding)
    ax = fig.add_axes(axrect)
    #frame = ax.get_frame()
    #frame.set_fill(False)

    xlog = (str(self.get('xAxisScale', "lin")) == 'log')
    ylog = (str(self.get('yAxisScale', "lin")) == 'log')
    if xlog:
      ax.semilogx()
    if ylog:
      ax.semilogy()

    setp(ax.get_xticklabels(), fontproperties=self.font)
    setp(ax.get_yticklabels(), fontproperties=self.font)
    setp(ax.get_xticklines(), markeredgewidth=2.0, zorder=4.0)
    setp(ax.get_yticklines(), markeredgewidth=2.0)
    ax.grid(True, alpha=0.25, color='#000000', linewidth=0.1)

    # Set titles
    if not self.get("notitle",False):
      ax.title = ax.text(.5, 1+(hsub+textPadding)/(axrect[-1]*h), title[0],
                      verticalalignment='bottom',
                      horizontalalignment='center',
                      transform=ax.transAxes,
                      clip_box=None,
                      fontproperties=self.tfont)
      ax._set_artist_props(ax.title)
      if len(title) > 1:
        ax.subtitle = ax.text(.5, 1+textPadding/(axrect[-1]*h), "\n".join(title[1:]),
                           verticalalignment='bottom',
                           horizontalalignment='center',
                           transform=ax.transAxes,
                           clip_box=None,
                           fontproperties=self.sfont)

    if not self.get("nolabels",False):
      ax.set_xlabel(self.get("xAxisTitle", ""), fontproperties=self.afont)
      ax.set_ylabel(self.get("yAxisTitle", ""), fontproperties=self.afont)
    return (fig, canvas, ax)

  def map(self):
    return ''

  def make(self):
    img = self.draw().read()
    legend = self.legend().read()
    map = self.map()
    return img,legend,map

class LegendChart(Chart):
  def legend(self):

    legend = self._getLegend()
    if len(legend)>=int(self.get('max_legend')):
      _logwarn("PLOT: Legend too long (%s) - truncating"%len(legend))
      legend = legend[:int(self.get('max_legend'))]
      legend.append((int(self.get('max_legend')),'Some entries truncated',{'facecolor':'#000000'},0))
    if len(legend)==0:
      (fig, canvas, w, h) = self.canvas()
      return self.save(fig,canvas)
    namelen = max(map(lambda x: len(x[1]),legend))
    i = len(legend)+1

    dpif = float(self.get('dpi')) / 72
    wcol = 10 + (2 + 0.6*namelen)*self.font.get_size()*dpif
    ncol = max(1, math.floor(float(self.get('width')) / wcol))
    wcol = float(self.get('width')) / ncol
    nrow = int((i + ncol - 1) / ncol)
    self.height = (nrow + 1) * self.font.get_size() * 1.4 * dpif
    (fig, canvas, w, h) = self.canvas()
    step = self.font.get_size() * 1.4 * dpif / h
    x = 0
    y = 0.5 * self.font.get_size() * dpif / h
    xbox = 10./w
    wbox = self.font.get_size() * dpif / w * 1.25
    hbox = self.font.get_size() * dpif / h * 0.75
    xtext = xbox + wbox*1.5
    row = 0
    for item in legend:
      fig.text(x + xtext, 1 - y, item[1],
           horizontalalignment='left',
           verticalalignment='top',
           fontproperties=self.font)
      p = Rectangle((x + xbox, 1-y-hbox), wbox, hbox, linewidth=0.5, fill=True)
      p.update(item[2])
      fig._set_artist_props(p)
      fig.patches.append(p)
      y += step
      row += 1
      if row == nrow:
        x += wcol/w
        y = 0.5 * self.font.get_size() * dpif / h
        row = 0
    return self.save(fig, canvas)

class BaobabChart(Chart):
  defaults={}
  def __init__(self,data,legend,props):
    Chart.__init__(self,props,self.defaults)
    self._data = data
    self._legend = legend
    self._map = {}
    self.title = self.get('title', '')

  def draw(self):
    if not self._data['value']>0:
      return Chart.draw(self)
    fig,canvas,ax = self.prepare()

    ax.set_axis_off()
    #theta = lambda x: x*6.2831/self._data['value']
    theta = lambda x: x*360./self._data['value']

    def fontsize(n):
      if len(n)>24:
        return 6
      if len(n)>16:
        return 8
      if len(n)>8:
        return 10
      return 12

    def baobab_recursor(depth,here,startx):
      if len(here['children'])>0:
        left=[theta(startx)]
        height=[1]
        width=[theta(here['value'])]
        bottom=[depth]
        name=[here['name']]
        value=[here['value']]
        dumped = 0
        for c in here['children']:
          #if c['value']>here['value']*0.05:
          #if theta(c['value'])>(9./(depth+1)**2):
          if theta(c['value'])>min((9./(depth+1)**2),1):
            cleft,cheight,cwidth,cbottom,cname,cvalue = baobab_recursor(depth+1,c,startx)
            left.extend(cleft)
            height.extend(cheight)
            width.extend(cwidth)
            bottom.extend(cbottom)
            name.extend(cname)
            value.extend(cvalue)
            startx += c['value']
          else:
            dumped += c['value']
        if dumped>0:
          left.append(theta(startx))
          height.append(0.75)
          width.append(theta(dumped))
          bottom.append(depth+1)
          name.append('')
          value.append(dumped)
        return left,height,width,bottom,name,value
      else:
        return [theta(startx)],[1],[theta(here['value'])],[depth],[here['name']],[here['value']]

    left,height,width,bottom,name,value = baobab_recursor(0,self._data,0)

    # this works in 0.99 but not this version
    #bars = ax.bar(left=left[1:],height=height[1:],width=width[1:],bottom=bottom[1:])

    bars = []
    max_height = max(bottom)

    if not self.get('nolabels',False):
      bar_location = self._data['value']/5.
      #magnitude = int(math.log10(bar_location))
      #possible = [i*10**magnitude for i in (1.,2.,5.,10.)]
      #best_delta = 10*10**magnitude
      magnitude = int(math.log(bar_location,2))
      possible = [i*2**magnitude for i in (1.,2.,4.,8.)]
      best_delta = 8*2**magnitude

      use_bar_location = 0
      for p in possible:
        if abs(p-bar_location)<best_delta:
          best_delta=abs(p-bar_location)
          use_bar_location=p

      for i in range(int(self._data['value']/use_bar_location)+1):
        endtheta = theta(i*use_bar_location)
        endx = (max_height+3)*math.cos(math.radians(endtheta))
        endy = (max_height+3)*math.sin(math.radians(endtheta))
        line = Line2D((0,endx),(0,endy),linewidth=1,linestyle='-.',zorder=-1)
        ax.add_line(line)
        ax.text(endx,endy,BinFormatter(i*use_bar_location,self.get('unit','')),horizontalalignment='center',verticalalignment='center',color='blue')

    colours = [Accent(l/360.) for l in left]
    for i, h in enumerate(height):
      if h<1:
        colours[i] = '#cccccc'

    for l,h,w,b,n,v,c in sorted(zip(left[1:],height[1:],width[1:],bottom[1:],name[1:],value[1:],colours[1:]),key=lambda x:x[3],reverse=True):
      wedge = Wedge((0,0),b+h,l,l+w,h,facecolor=c)
      ax.add_patch(wedge)
      bars.append(wedge)

      ctheta = l+w*0.5
      cr = b+h*0.5

      cx = cr*math.cos(math.radians(ctheta))
      cy = cr*math.sin(math.radians(ctheta))

      angle_rad = ctheta
      angle_tan = ctheta - 90
      if ctheta>90 and ctheta<180:
        angle_rad += 180
      if ctheta>180 and ctheta<270:
        angle_rad -= 180
        angle_tan -= 180
      if ctheta>270 and ctheta<360:
        angle_tan -= 180

      if b==max_height and self.get('outer',True):
        ax.text((cr+1.5)*math.cos(math.radians(ctheta)), (cr+1.5)*math.sin(math.radians(ctheta)),n,horizontalalignment='center',verticalalignment='center',rotation=angle_rad,fontsize=fontsize(n))
      elif b==1:
        ax.text(cx,cy,n,horizontalalignment='center',verticalalignment='center',rotation=angle_tan,fontsize=fontsize(n))
      else:
        ax.text(cx,cy,n,horizontalalignment='center',verticalalignment='center',rotation=angle_rad,fontsize=fontsize(n))

    ax.add_patch(Circle((0,0),1,facecolor='#ffffff'))
    ax.text(0,0,BinFormatter(self._data['value'],self.get('unit','')),horizontalalignment='center',verticalalignment='center',fontsize=16)
    ax.set_xlim(-(max_height+3),max_height+3)
    ax.set_ylim(-(max_height+3),max_height+3)

    return self.save(fig,canvas)

  def prepare(self):
    (fig, canvas, w, h) = self.canvas()
    dpif = float(self.get('dpi')) / 72
    padding = float(self.get('padding')) * dpif / h
    textPadding = float(self.get('textPadding')) * dpif
    titleFontSize = float(self.get('titleFontSize'))
    axFontSize = float(self.get('axtitleFontSize'))
    subFontSize = float(self.get('subtitleFontSize'))
    title = self.get("title", "").split("\n")
    if not self.get("notitle",False):
      hsub = (len(title)-1) * (subFontSize * dpif + 4) + textPadding
      htitle = hsub + titleFontSize * dpif + textPadding * 2
    else:
      hsub = 0
      htitle = 0
    if self.get("nolabels",False):
      padding = 0

    # Configure axes
    if self.get('square'):
      minsize = 1 - 2*padding
      axrect = (.5 - minsize/2 * h/w, padding, h/w * minsize, minsize)
    else:
      axrect = (padding, padding, 1 - 1.25*padding, 1 - htitle/h - padding)
    ax = fig.add_axes(axrect)
    frame = ax.get_frame()
    frame.set_fill(False)

    if not self.get("notitle",False):
      ax.title = ax.text(.5, 1+(hsub+textPadding)/(axrect[-1]*h), title[0],
                       verticalalignment='bottom',
                       horizontalalignment='center',
                       transform=ax.transAxes,
                       clip_box=None,
                       fontproperties=self.tfont)
      ax._set_artist_props(ax.title)
      if len(title) > 1:
        ax.subtitle = ax.text(.5, 1+textPadding/(axrect[-1]*h), "\n".join(title[1:]),
                            verticalalignment='bottom',
                            horizontalalignment='center',
                            transform=ax.transAxes,
                            clip_box=None,
                            fontproperties=self.sfont)

    return (fig, canvas, ax)

  def map(self):
    return ""

  def legend(self):
    dpif = float(self.get('dpi')) / 72
    self.height = 1 * self.font.get_size() * 1.4 * dpif
    (fig, canvas, w, h) = self.canvas()
    fig.text(.5, .5, "No legend",
            horizontalalignment='center',
            verticalalignment='center')
    return self.save(fig, canvas)


class PieChart(LegendChart):
  defaults = {}
  def __init__(self,data,legend,props):
    Chart.__init__(self,props,self.defaults)
    self._data = data
    self._legend = legend
    self._map = {}
    self.title = "%s\nfrom %s to %s UTC" % (self.get('title', ''),
                  strftime("%A %Y-%m-%d %H:%M", gmtime(props['starttime'])),
                  strftime("%A %Y-%m-%d %H:%M", gmtime(props['endtime'])))

  def _getLegend(self):
    legend=[]
    i=1
    for item in self._legend:
      if self.get('integral'):
        name = "%s [%s]" % (item['name'],SIFormatter(self._data[item['name']]))
      else:
        name = item['name']
      legend.append((i, name, item['attrs'],self._data[item['name']]))
      i+=1
    if self.get('sort'):
      legend = sorted(legend, key=lambda x: x[3],reverse=True)
    return legend

  def draw(self):
    if len(self._data) == 0:
      return Chart.draw(self)
    fig,canvas,ax = self.prepare()

    getcol = lambda x: [item['attrs']['facecolor'] for item in self._legend if item['name']==x][0]

    labels = []
    values = []
    colours = []

    items = self._data.keys()
    if self.get('sort'):
      items = sorted(items,key=lambda x:self._data[x])


    for d in items:
      labels.append(d)
      values.append(self._data[d])
      colours.append(getcol(d))

    slices, texts = ax.pie(values,[0.1 for v in values],None,colours,shadow=True)

    # The list of vertices from a Wedge contains vertices at 0.01 radian intervals
    # reduce the vertex list with every nth point from the curved segment
    def vert_reduce(x):
      interval = 40
      verts = [x[0]]
      verts.extend(x[1::interval])
      if not len(x) % interval == 2:
        verts.append(x[-1])

      # Flatten the list from ((x,y),(x,y)... to (x,y,x,y...
      flat = []
      for v in verts:
        flat.extend((v[0],float(self.get('height')) - v[1]))
      return flat

    for l,s in zip(labels,slices):
      verts = s.get_transform().transform(s.get_verts())
      self._map[l]=vert_reduce(verts)

    return self.save(fig,canvas)


  def prepare(self):
    (fig, canvas, w, h) = self.canvas()
    dpif = float(self.get('dpi')) / 72
    padding = float(self.get('padding')) * dpif / h
    textPadding = float(self.get('textPadding')) * dpif
    titleFontSize = float(self.get('titleFontSize'))
    axFontSize = float(self.get('axtitleFontSize'))
    subFontSize = float(self.get('subtitleFontSize'))
    title = self.get("title", "").split("\n")
    if not self.get("notitle",False):
      hsub = (len(title)-1) * (subFontSize * dpif + 4) + textPadding
      htitle = hsub + titleFontSize * dpif + textPadding * 2
    else:
      hsub = 0
      htitle = 0
    if self.get("nolabels",False):
      padding = 0

    # Configure axes
    if self.get('square'):
      minsize = 1 - 2*padding
      axrect = (.5 - minsize/2 * h/w, padding, h/w * minsize, minsize)
    else:
      axrect = (padding, padding, 1 - 1.25*padding, 1 - htitle/h - padding)
    ax = fig.add_axes(axrect)
    frame = ax.get_frame()
    frame.set_fill(False)

    if not self.get("notitle",False):
      ax.title = ax.text(.5, 1+(hsub+textPadding)/(axrect[-1]*h), title[0],
                       verticalalignment='bottom',
                       horizontalalignment='center',
                       transform=ax.transAxes,
                       clip_box=None,
                       fontproperties=self.tfont)
      ax._set_artist_props(ax.title)
      if len(title) > 1:
        ax.subtitle = ax.text(.5, 1+textPadding/(axrect[-1]*h), "\n".join(title[1:]),
                            verticalalignment='bottom',
                            horizontalalignment='center',
                            transform=ax.transAxes,
                            clip_box=None,
                            fontproperties=self.sfont)

    return (fig, canvas, ax)

  def map(self):
    result = ""

    total = sum(self._data.values())

    if len(self._map) == 0 and len(self._data) > 0:
      self.draw()

    polyStr = lambda x: ",".join([str(int(p)) for p in x])
    mapFormat = lambda fstr,name,value: fstr.replace('$n',str(name)).replace('$v',str(value))

    for name in self._map:
      value = self._data[name]
      if value > total/1000.:
        attribs = []
        for aname in [n for n in self.props if n.startswith('map_')]:
          aval = self.get(aname)
          if aval:
            attribs.append('%s="%s"' % (aname[4:],mapFormat(aval,name,value)))
        result += """<area %s shape="polygon" coords="%s">\n""" % (' '.join(attribs),polyStr(self._map[name]))

    return result

  def jsonmap(self):
    result = []
    for name in self._map:
      value = self._data[name]
      result.append({'coords':[self._map[name][2*i:(2*i)+1] for i in range(len(self._map[name])/2)],'title':name,'value':value})
    return [{'start':0,'end':0,'areas':result}]

class CumulativeTimeSeriesChart(LegendChart):
  defaults={}
  def __init__(self,span,series,data,legend,props):
    Chart.__init__(self,props,self.defaults)
    self._span = span
    self._series = series
    self._data = data
    self._legend = legend
    self._map = {}
    self._integral = {}
    if span == 'hour':
      self._axisFormat = "%H:%M\n%a %d"
      spanFormat = "%A %Y-%m-%d %H:%M"
    elif span == 'day':
      self._axisFormat = "%a %d\n%b %Y"
      spanFormat = "%A %Y-%m-%d"
    elif span == 'week':
      self._axisFormat = "Week %V\n%d %b, %Y"
      spanFormat = "%Y-%V"
    elif span == 'month':
      self._axisFormat = "%B\n%Y"
      spanFormat = "%Y-%m"
    elif span == 'year':
      self._axisFormat = "%Y"
      spanFormat = "%Y"
    else:
      raise "Bad time series span"

    self.title = "%s\n%d %ss from %s to %s UTC" % \
      (self.get('title', ''), len(series), span,
       strftime(spanFormat, gmtime(series[0][0])),
       strftime(spanFormat, gmtime(series[-1][0])))

  def prepare(self):
    fig,canvas,ax = Chart.prepare(self)
    if len(self._series) > 0:
      ax.set_xlim(xmin = self._series[0][0], xmax = self._series[-1][1])
      ax.xaxis.set_major_locator(TimeSeriesLocator(self._span, self._series))
      ax.xaxis.set_major_formatter(TimeSeriesFormatter(self._axisFormat))
      ax.xaxis.set_clip_on(False)
      ax.yaxis.set_major_formatter(ThousandScalarFormatter())
    return (fig, canvas, ax)

  def draw(self):
    if len(self._series) == 0:
      return Chart.draw(self)

    keys = []
    color = {}
    for item in self._legend:
      keys.append(item['key'])
      color[item['key']] = item['attrs']['facecolor']



    xcoords = numpy.zeros(len(self._series)+1)
    xcoords[0] = self._series[0][0]
    for i,s in enumerate(self._series):
      xcoords[i+1] = s[1]

    (fig, canvas, ax) = self.prepare()
    surfaces = []
    surfaces.append(numpy.zeros(len(self._series)+1))

    for k in keys:
      self._integral[k] = 0.
      for d in self._data:
        if k in d:
          self._integral[k] += d[k]

    if self.get('sort'):
      keys = sorted(keys,key=lambda x: self._integral[x])

    for k in keys:
      cur = numpy.zeros(len(self._series)+1)
      total = 0
      for i,d in enumerate(self._data):
        if k in d:
          value = d[k]
          total += value
        cur[i+1] = surfaces[len(surfaces)-1][i+1] + total
      surfaces.append(cur)

    #xcoords2 = numpy.zeros(2*(len(self._series)+1))
    #for i,x in enumerate(xcoords):
    #  xcoords2[i] = x
    #for i,x in enumerate(xcoords[::-1]):
    #  xcoords2[i+len(xcoords)] = x

    map_patches = {}
    for i,k in enumerate(keys):
    #  surface2 = numpy.zeros(len(xcoords2))
    #  for j,s in enumerate(surfaces[i]):
    #    surface2[j] = s

    #  for p,q in enumerate(surfaces[i+1][::-1]):
        #surface2[p+len(xcoords)] = q

      # This function (fill_between) is unavailable at this version of matplotlib, so we need to make complete polygons
      patch = ax.fill_between(xcoords,surfaces[i],surfaces[i+1],None,facecolor=color[k],closed=True)
      #patches = ax.fill(xcoords2,surface2,facecolor=color[k])
      #map_patches[k] = patches[0]
      map_patches[k] = patch

    for k in map_patches:
      p = map_patches[k]
      maparea = []

      def reduce_verts(verts):
        if len(verts) >= 4:
          if not (verts[0][0] == verts[-1][0] and verts[0][1] == verts[-1][1]):
            return verts
          if not (verts[1][0] == verts[-2][0] and verts[1][1] == verts[-2][1]):
            return verts
          # First two points are co-located - ie zero area, truncate them
          verts=verts[1:-1]
          return reduce_verts(verts)
        # If there are less than 4, then there must be only two points (at the same x) left - pointless
        return None
      # There is only ever a single patch here
      #verts = reduce_verts(p.get_transform().transform(p.get_verts()))
      verts = reduce_verts(p.get_transform().transform(p.get_paths()[0].vertices))
      if len(verts)>0:
        for v in verts:
          maparea.extend((v[0],float(self.get('height')) - v[1]))
        self._map[k] = maparea

    ax.set_ylim(ymin=0)
    return self.save(fig,canvas)


  def _getLegend(self):
    seen = {}
    legend=[]
    i=1
    for d in self._data:
      for k in d:
        seen[k] = 1
    for item in self._legend:
      if not item['key'] or item['key'] not in seen: continue
      if self.get('integral'):
        name = "%s [%s]" % (item['name'],SIFormatter(self._integral[item['key']]))
      else:
        name=item['name']
      legend.append((i, name, item['attrs'],self._integral[item['key']]))
      i+=1
    if self.get('sort'):
      legend = sorted(legend, key=lambda x:x[3],reverse=True)
    return legend


  def map(self):
    result = ""

    if len(self._map) == 0 and len(self._data) > 0:
      self.draw()

    polyStr = lambda x: ",".join([str(int(p)) for p in x])
    mapFormat = lambda fstr,name,value: fstr.replace('$n',str(name)).replace('$v',str(value))

    for name in self._map:
      value = self._integral[name]
      attribs = []
      for aname in [n for n in self.props if n.startswith('map_')]:
        aval = self.get(aname)
        if aval:
          attribs.append('%s="%s"' % (aname[4:],mapFormat(aval,name,value)))
      result += """<area %s shape="polygon" coords="%s">\n""" % (' '.join(attribs),polyStr(self._map[name]))

    return result

  def jsonmap(self):
    result = []
    for name in self._map:
      value = self._integral[name]
      result.append({'coords':[self._map[name][2*i:(2*i)+1] for i in range(len(self._map[name])/2)],'title':name,'value':value})
    return [{'start':0,'end':0,'areas':result}]





# Time series stacked bar chart.
class TimeSeriesBarChart(LegendChart):
  defaults = {}
  def __init__(self, span, series, data, legend, props):
    Chart.__init__(self, props, self.defaults)
    self._span = span
    self._series = series
    self._data = data
    self._legend = legend
    self._map = []
    self._integral = {}

    if span == 'hour':
      self._axisFormat = "%H:%M\n%a %d"
      spanFormat = "%A %Y-%m-%d %H:%M"
    elif span == 'day':
      self._axisFormat = "%a %d\n%b %Y"
      spanFormat = "%A %Y-%m-%d"
    elif span == 'week':
      self._axisFormat = "Week %V\n%d %b, %Y"
      spanFormat = "%Y-%V"
    elif span == 'month':
      self._axisFormat = "%B\n%Y"
      spanFormat = "%Y-%m"
    elif span == 'year':
      self._axisFormat = "%Y"
      spanFormat = "%Y"
    else:
      raise "Bad time series span"

    self.title = "%s\n%d %ss from %s to %s UTC" % \
      (self.get('title', ''), len(series), span,
       strftime(spanFormat, gmtime(series[0][0])),
       strftime(spanFormat, gmtime(series[-1][0])))

  def prepare(self):
    (fig, canvas, ax) = Chart.prepare(self)
    if len(self._series) > 0:
      ax.set_xlim(xmin = self._series[0][0], xmax = self._series[-1][1])
      ax.xaxis.set_major_locator(TimeSeriesLocator(self._span, self._series))
      ax.xaxis.set_major_formatter(TimeSeriesFormatter(self._axisFormat))
      ax.xaxis.set_clip_on(False)
      ax.yaxis.set_major_formatter(ThousandScalarFormatter())
    return (fig, canvas, ax)

  def draw(self):
    if len(self._series) == 0:
      return Chart.draw(self)

    (fig, canvas, ax) = self.prepare()
    spatch=[]
    for item in self._legend:
      self._integral[item['key']]=0
    for item in self._legend:
      for i in range(len(self._series)):
        if item['key'] in self._data[i]:
          self._integral[item['key']]+=self._data[i][item['key']]
    item_list = self._legend
    if self.get('sort'):
      item_list = sorted(item_list, key = lambda x: self._integral[x['key']])

    for i in xrange(0, len(self._series)):
      smap = {}

      bin = self._series[i]
      data = self._data[i]
      if len(data) == 0:
        data = { None: 0 }
      l = numpy.zeros(len(data))
      b = numpy.zeros(len(data))
      h = numpy.zeros(len(data))
      a = [None for x in data]
      n = [None for x in data]
      j = 0
      for item in item_list:
        if item['key'] in data:
          l[j] = bin[0]
          b[j] = (j > 0 and b[j-1] + h[j-1]) or 0
          h[j] = data[item['key']]
          a[j] = item['attrs']
          n[j] = item['key']
          j += 1
      p = ax.bar(l, h, bottom=b, width=bin[1]-bin[0], linewidth=.5)
      for i in xrange(0, len(p)):
        p[i].update(a[i] or {})
        smap[n[i]]=p[i]
      spatch.append(smap)
    for s in spatch:
      sresult={}
      for key in s:
        patch = s[key]
        verts = patch.get_transform().transform(patch.get_verts())
        maparea=[]
        for v in verts[::2]:
          maparea.extend((v[0],float(self.get('height'))-v[1]))
        sresult[key]=maparea
      self._map.append(sresult)
    return self.save(fig, canvas)

  def _getLegend(self):
    seen = {}
    legend = []
    i=1
    for d in self._data:
      for k in d:
        seen[k] = 1
    for item in self._legend:
      if not item['key'] or item['key'] not in seen: continue
      if self.get('integral'):
        name = "%s [%s]" % (item['name'],SIFormatter(self._integral[item['key']]))
      else:
        name=item['name']
      legend.append((i, name, item['attrs'],self._integral[item['key']]))
      i+=1
    if self.get('sort'):
      legend = sorted(legend, key=lambda x:x[3],reverse=True)
    return legend

  def map(self):
    result = ""
    polyStr = lambda x: ",".join([str(int(p)) for p in x])
    for i,data in enumerate(self._data):
      starttime = self._series[i][0]
      endtime = self._series[i][1]

      mapFormat = lambda fstr,name,value: fstr.replace('$n',str(name)).replace('$v',str(value)).replace('$b',strftime("%H:%M", gmtime(starttime))).replace('$e',strftime("%H:%M", gmtime(endtime)))

      for name in data:
        value = data[name]
        if value>0:
          attribs = []
          for aname in [n for n in self.props if n.startswith('map_')]:
            aval = self.get(aname)
            if aval:
              attribs.append('%s="%s"' % (aname[4:],mapFormat(aval,name,value)))
          result += """<area %s shape="rect" coords="%s">\n""" % (' '.join(attribs),polyStr(self._map[i][name]))

    return result

  def jsonmap(self):
    series=[]
    for i,data in enumerate(self._data):
      starttime = self._series[i][0]
      endtime = self._series[i][1]
      area = []
      for name in data:
        value = data[name]
        if value>0:
          area.append({'title':name,'value':value,'coords':[self._map[i][name][2*i:(2*i)+1] for i in range(len(self._map[i][name])/2)]})
      series.append({'start':starttime,'end':endtime,'areas':areas})
    return series


class TimeSeriesByteBarChart(TimeSeriesBarChart):
  def prepare(self):
    (fig, canvas, ax) = TimeSeriesBarChart.prepare(self)
    ax.yaxis.set_major_locator(ByteScalarLocator())
    ax.yaxis.set_major_formatter(ByteScalarFormatter())
    return (fig, canvas, ax)

class RateQualityScatterChart(Chart):
  defaults = {}
  def __init__(self, span, series, data, legend, props):
    Chart.__init__(self, props, self.defaults)
    self._span = span
    self._series = series
    self._data = data
    self._legend = legend

    if span == 'hour':
      spanFormat = "%A %Y-%m-%d %H:%M"
    elif span == 'day':
      spanFormat = "%A %Y-%m-%d"
    elif span == 'week':
      spanFormat = "%Y-%V"
    elif span == 'month':
      spanFormat = "%Y-%m"
    elif span == 'year':
      spanFormat = "%Y"
    else:
      raise "Bad time series span"

    self.title = "%s\n%d %ss from %s to %s UTC" % \
      (self.get('title', ''), len(series), span,
       strftime(spanFormat, gmtime(series[0][0])),
       strftime(spanFormat, gmtime(series[-1][0])))

  def prepare(self):
    (fig, canvas, ax) = Chart.prepare(self)
    ax.xaxis.set_major_formatter(PercentFormatter())
    ax.yaxis.set_major_formatter(ThousandScalarFormatter())
    return (fig, canvas, ax)

  def draw(self):
    if len(self._series) == 0 or len(self._data) == 0:
      return Chart.draw(self)

    (fig, canvas, ax) = self.prepare()
    i = ymax = 0
    x = numpy.zeros(len(self._data))
    y = numpy.zeros(len(self._data))
    s = numpy.zeros(len(self._data))
    c = ['#000000' for item in self._data]
    t = self.get('letters', False)
    for item in self._legend:
      if item['key'] not in self._data:
        continue
      (rate, qval, n) = self._data[item['key']]
      ymax = max(ymax, rate)
      x[i] = qval
      y[i] = rate
      s[i] = 5 * (1 + math.log(n, 10))
      c[i] = item['attrs']['facecolor']
      i += 1
      if t:
        bright = colorsys.rgb_to_yiq(*text2rgb(c[i-1]))[0] > 0.6
        ax.text(qval, rate, str(i),
               verticalalignment='center',
               horizontalalignment='center',
               clip_box = None,
               color=(bright and '#000000') or '#ffffff',
               fontproperties=self.font)
    if self.get('bubbles', True):
      ax.scatter(x, y, s * s, facecolor=c)

    if self.get('bubbles', True):
      ymin, ymax = ax.get_ylim()
    else:
      ymax = ymax * 1.1
    ax.set_xlim(xmin=-0.01, xmax=1.01)
    ax.set_ylim(ymin=-ymax/100, ymax=ymax)
    return self.save(fig, canvas)

  def legend(self):
    i = 1
    namelen = 0
    legend = []
    seen = dict((k, 1) for k in self._data)
    for item in self._legend:
      if not item['key'] or item['key'] not in seen: continue
      legend.append((i, item['name'], item['attrs']))
      namelen = max(namelen, len(item['name']))
      i += 1

    dpif = float(self.get('dpi')) / 72
    wcol = 10 + (5 + 0.6*namelen)*self.font.get_size() * dpif
    ncol = max(1, math.floor(float(self.get('width')) / wcol))
    wcol = float(self.get('width')) / ncol
    nrow = int((i + ncol - 1) / ncol)
    self.height = (nrow + 1) * self.font.get_size() * 1.4 * dpif
    (fig, canvas, w, h) = self.canvas()
    x = 0
    y = self.font.get_size() * 0.5 * dpif / h
    step = self.font.get_size() * 1.4 * dpif / h
    xnr = 10./w
    xbox = xnr + math.ceil(math.log(i, 10)) * self.font.get_size() * dpif / w
    wbox = self.font.get_size() * dpif / w * 1.25
    hbox = self.font.get_size() * dpif / h * 0.75
    xtext = xbox + wbox*1.5
    row = 0
    for item in legend:
      fig.text(x + xnr, 1 - y, str(item[0]) + ".",
              horizontalalignment='left',
              verticalalignment='top',
              fontproperties=self.font)
      fig.text(x + xtext, 1 - y, item[1],
              horizontalalignment='left',
              verticalalignment='top',
              fontproperties=self.font)
      p = Rectangle((x + xbox, 1-y-hbox), wbox, hbox, linewidth=0.5, fill=True)
      p.update(item[2])
      fig._set_artist_props(p)
      fig.patches.append(p)
      y += step
      row += 1
      if row == nrow:
        x += wcol/w
        y = 0.5 * self.font.get_size() * dpif / h
        row = 0
    return self.save(fig, canvas)

class LatencyDensityMap(Chart):
  defaults = {}
  def __init__(self, span, series, data, legend, props):
    Chart.__init__(self, props, self.defaults)
    self._span = span
    self._series = series
    self._data = data
    self._legend = legend

    if span == 'hour':
      spanFormat = "%A %Y-%m-%d %H:%M"
    elif span == 'day':
      spanFormat = "%A %Y-%m-%d"
    elif span == 'week':
      spanFormat = "%Y-%V"
    elif span == 'month':
      spanFormat = "%Y-%m"
    elif span == 'year':
      spanFormat = "%Y"
    else:
      raise "Bad time series span"

    self.title = "%s\n%d %ss from %s to %s UTC" % \
      (self.get('title', ''), len(series), span,
       strftime(spanFormat, gmtime(series[0][0])),
       strftime(spanFormat, gmtime(series[-1][0])))

    self.keys = []
    for item in self._legend:
      if item['key'] in self._data:
        self.keys.append(item)

  def draw(self):
    if len(self._series) == 0 \
       or len(self._data) == 0 \
       or len(self.keys) == 0:
      return Chart.draw(self)

    (fig, canvas, ax) = self.prepare()
    w, h = float(self.get('width')), float(self.get('height'))
    dpif = float(self.get('dpi')) / 72
    axrect = ax.get_position().get_points()
    nkeys = len(self.keys)
    cmap = cm.Paired

    tickfont = self.font
    wlabels = nkeys*tickfont.get_size()*dpif
    if wlabels / w > (axrect[1, 0]-axrect[0, 0]) / 1.2:
      tickfont = tickfont.copy()
      tickfont.set_size(max(5, w * (axrect[1, 0]-axrect[0, 0]) / nkeys / 1.2))

    maxname = max(len(i['name']) for i in self.keys)
    maxname = 0.6*(1 + maxname)*tickfont.get_size()*dpif/h
    cbartitle = 1.2*self.afont.get_size()*dpif/h;
    ax.set_position((axrect[0, 0], maxname,
                    axrect[1, 0]-axrect[0, 0]-cbartitle,
                    axrect[1, 1]-maxname))

    valuef = self.get('yAxisValue')
    nbins = self.get('yAxisBins')
    xmax = max(nkeys, 16)
    x, y = numpy.meshgrid(numpy.arange(xmax+1), [valuef(bin) for bin in xrange(0, nbins+1)])
    c = numpy.zeros((nbins+1, xmax))
    for i in xrange(0, nkeys):
      item = self.keys[i]
      d = self._data[item['key']]
      hmin = hmax = None
      for hbin, n in d.iteritems():
        c[hbin, i] = n
        if hmin == None:
          hmin = valuef(hbin)
          hmax = valuef(hbin+1)
        else:
          hmin = min(hmin, valuef(hbin))
          hmax = max(hmax, valuef(hbin+1))
      if hmin != None:
        ax.axhline(hmin, float(i)/xmax, float(i+1)/xmax, color="#000000", linewidth=.95)
        ax.axhline(hmax, float(i)/xmax, float(i+1)/xmax, color="#000000", linewidth=.95)

    map = ax.pcolor(x, y, ma.masked_where(c == 0, c), shading='flat', cmap=cmap)
    cb = fig.colorbar(map, aspect=50, fraction=0.025, pad=0.01)
    cb.ax.set_ylabel(self.get("yScaleAxisTitle", ""), fontproperties=self.afont)
    setp(cb.ax.get_yticklabels(), fontproperties=self.font)

    setp(ax.get_xticklines(), markeredgewidth=0, zorder=4.0)
    ax.set_xticks(numpy.arange(0, xmax, 1) + 0.5)
    ax.set_xticklabels([x['name'] for x in self.keys],
                      rotation='vertical',
                      horizontalalignment='center',
                      fontproperties=tickfont)
    ax.set_xlim(xmin=0, xmax=xmax)
    ax.set_ylim(ymin=valuef(0), ymax=valuef(nbins+1))
    return self.save(fig, canvas)

  def legend(self):
    dpif = float(self.get('dpi')) / 72
    self.height = 1 * self.font.get_size() * 1.4 * dpif
    (fig, canvas, w, h) = self.canvas()
    fig.text(.5, .5, "No legend",
            horizontalalignment='center',
            verticalalignment='center')
    return self.save(fig, canvas)
