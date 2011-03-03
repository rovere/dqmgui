import urllib2, re, time, urlparse, socket
from urllib import quote_plus
from Monitoring.Core.Utils import _logwarn
from threading import Thread, Lock
from cherrypy import HTTPError, engine
from copy import deepcopy

def mklnk(baseurl, **kwargs):
  return baseurl + "&".join("%s=%s" % (k, quote_plus(str(v), "/"))
			    for k, v in sorted(kwargs.iteritems()))

def lemonimg(prov, kind, images, match, **kwargs):
  return {'html': mklnk("http://lemonweb.cern.ch/lemon-%s/%s.php?" % (prov, kind),
			type="host", cluster=1, **kwargs),
	  'match': match, 'images': images}

def lrfimg(images, match, **kwargs):
  return {'html': mklnk("http://lsf-rrd.cern.ch/lrf-lsf/info.php?", **kwargs),
	  'match': match, 'images': images}

def slsimg(images, match, **kwargs):
  return {'html': mklnk("http://sls.cern.ch/sls/history.php?", **kwargs),
	  'match': match, 'images': images}

class CompWorkspace:
  def __init__(self, gui, rank, category, tag, name, views, *args):
    self.gui = gui
    self.rank = rank
    self.category = category
    self.tag = tag
    self.name = name
    self.views = views
    for v in views:
      if not getattr(v, 'label', None):
	v.label = re.sub(r"[^-A-Za-z0-9 ]", "", v.name.lower()).replace(" ", "-")
      if not getattr(v, 'state', None) and getattr(v, 'plugin', None):
        v.__class__.state = lambda me, session: "{'kind': '%s'}" % me.plugin
    gui.css = [x for x in gui.css if x[0] != '%s/css/Core/style.css' % gui.contentpath]
    gui._addCSSFragment("%s/css/Overview/style.css" % gui.contentpath)
    gui._addJSFragment('%s/javascript/Overview/Overview.js' % gui.contentpath)

  def _dostate(self, session, helptopic = None):
    view = self._getView(session[self.tag + ".view"])
    self.gui._noResponseCaching()
    return ("([{'kind': 'AutoUpdate', 'interval': %d, 'stamp': %d},"
            "{'kind': 'OverviewHeader', 'workspace': '%s',"
	    " 'workspaces': [%s], 'view': '%s', 'views':[%s],"
	    " 'menu': '%s'%s},"
	    " %s])") % \
	   (300, self.gui.stamp, self.name,
	    ",".join("{'title': '%s', 'label': '%s', 'category': '%s', 'rank': %d}" %
                     (x.name, x.name.lower(), x.category, x.rank)
                     for x in self.gui.workspaces),
            view.name,
            ",".join("{'title': '%s', 'label':'%s'}" %
		     (x.name, x.label) for x in self.views),
            session['comp.menu'],
	    (helptopic and (", 'help': '%s'" % helptopic)) or "",
	    view.state(session))


  def _getView(self,label):
    view = self.views[0]
    for v in self.views:
      if v.label == label:
        view = v
        break
    return view

  # -----------------------------------------------------------------
  def initialiseSession(self, session):
    viewid = self.tag + ".view"
    if viewid not in session:
      session[viewid] = self.views[0].label

    view = self._getView(session[viewid])

    if 'comp.menu' not in session:
      session['comp.menu'] = 'none'

    if getattr(self, 'sessiondef', None):
      for (key, value) in self.sessiondef.iteritems():
        if key not in session:
          session[key] = deepcopy(value)

    if getattr(view, 'initialiseSession', None):
      view.initialiseSession(session)


  # -----------------------------------------------------------------
  # Set session parameters, checking them for validity.
  def _set(self, session,
           submenu = None,
           view = None,
           **kwargs):

    if submenu != None:
      if not isinstance(submenu, str) \
         or submenu not in ("none", "workspace", "view"):
        raise HTTPError(500, "Incorrect submenu parameter")
      session['comp.submenu'] = submenu

    if view != None:
      if not isinstance(view, str) \
         or view.lower() not in (x.label for x in self.views):
        raise HTTPError(500, "Incorrect view parameter")
      session[self.tag + '.view'] = view.lower()

  # Initialise a new session with the "start" URL.
  def start(self, session, **kwargs):
    self._set(session, **kwargs)
    self.initialiseSession(session)

  # -----------------------------------------------------------------
  # Return JSON object for the current session state.  This method is
  # invoked when the GUI session page is loaded, which occurs either
  # when a completely new session has been started, the "reload"
  # button is pushed or the session URL was copied between browser
  # windows, or the session wants a period refresh.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

  # -----------------------------------------------------------------
  # Return DQM-specific index page.
  def sessionIndex(self, session, *args, **kwargs):
    return self.gui._templatePage("index-comp", {
        'TITLE'      : self.gui.title,
        'SESSION_ID' : session['core.name'],
        'USER'       : session['core.user'],
        'HOSTNAME'   : socket.gethostname(),
        'ROOTPATH'   : self.gui.baseUrl
      })

  # -----------------------------------------------------------------
  # Reset the canvas back to the workspace default.
  def sessionResetWorkspace(self, session, *args, **kwargs):
    self.initialiseSession(session)
    self.gui._saveSession(session)
    return self._state(session)

  # Switch session to another workspace.  Resets to sub-menu
  # then re-uses GUI core workspace method.
  def sessionWorkspace(self, session, *args, **kwargs):
    self._set(session, submenu = "none")
    return self.gui.sessionWorkspace(session, *args, **kwargs)

  # Switch workspace view.
  def sessionView(self, session, *args, **kwargs):
    self._set(session, view = kwargs.get('name', self.views[0].label))
    view = self._getView(session[self.tag + '.view'])
    if getattr(view, 'initialiseSession', None):
      view.initialiseSession(session, *args, **kwargs)
    self.gui._saveSession(session)
    return self._state(session)

  # Set sub-menu.  For notification only.
  def sessionSetSubMenu(self, session, *args, **kwargs):
    self._set(session, submenu = kwargs.get("n", "data"))
    self.gui._saveSession(session)
    return "Ta."

  # Set content search pattern.
  # Change panel display parameters.
  def sessionPanel(self, session, *args, **kwargs):
    name = kwargs.get('n')
    show = kwargs.get('show', None)
    x = kwargs.get('x', None)
    y = kwargs.get('y', None)

    if not isinstance(name, str) or name not in ("tools",):
      raise HTTPError(500, "Panel operation without a valid panel name")

    if show != None:
      if not isinstance(show, str) or show not in ("yes", "no"):
        raise HTTPError(500, "Invalid panel show parameter")
      session['comp.panel.%s.show' % name] = show == "yes"

    if x != None:
      if not isinstance(x, str) or not re.match(r"^\d+$", x):
        raise HTTPError(500, "Invalid panel position parameter")
      session['comp.panel.%s.x' % name] = int(x)

    if y != None:
      if not isinstance(y, str) or not re.match(r"^\d+$", y):
        raise HTTPError(500, "Invalid panel position parameter")
      session['comp.panel.%s.y' % name] = int(y)

    self.gui._saveSession(session)
    return self._state(session)

# --------------------------------------------------------------------
class ImageScraperSource:
  def __init__(self, plothook, images):
    self.plothook = plothook
    self.images = images
    self.scraper = WebImageScraper(300, self.images)
    engine.subscribe('exit', self.scraper.stop, priority=100)
    self.scraper.start()

  def plot(self, name, **attrs):
    i = self.scraper.image(name)
    if i:
      return (i['type'], i['data'])
    else:
      return (None, None)

# Scrape images off the web.
class WebImageScraper(Thread):
  "Class to get images from the web"

  def __init__(self, period, infosrc):
    """
    period - ?
    infosrc - Images to get in form [ {
      'images' : [list of images name],
      'html' : 'html address where from get',
      'match' : 'pattern to match image(s)' }  ]

    Images will be held in 'images' dictionary
    """

    Thread.__init__(self, name="WebImageScraper")
    self.lock = Lock()
    self.stopme = False
    self.period = period
    self.infosrc = infosrc
    self.images = {}
    for i in self.infosrc:
      for name in i['images']:
        self.images[name] = None

  def stop(self):
    self.lock.acquire()
    self.stopme = True
    self.lock.release()

  def image(self, name):
    self.lock.acquire()
    i = self.images.get(name, None)
    self.lock.release()
    return i

  def run(self):
    while True:
      self.lock.acquire()
      stopme = self.stopme
      self.lock.release()

      if stopme:
        break

      rximg = re.compile(r"<img\s+(.*?)/?>", re.I)
      for s in self.infosrc:
        img = 0
        imgrefs = {}
        try:
          data = urllib2.urlopen(s['html']).read().replace("\n", " ")
          for imgtag in re.findall(rximg, data):
            while True:
              m = re.match("^([-A-Za-z0-9]+)=(\"[^\"]*\"|'[^']*'|\S+)\s*/?", imgtag)
              if not m:
                break

              arg = m.group(2)
              if len(arg) >= 2 and arg[0] == '"' and arg[-1] == '"':
                arg = arg[1:-1]
              elif len(arg) >= 2 and arg[0] == "'" and arg[-1] == "'":
                arg = arg[1:-1]

              if m.group(1) == "src" and arg.find(s['match']) >= 0:
                imgrefs[s['images'][img]] = arg
                img = img+1
              imgtag = imgtag[m.end():]
        except Exception, e:
          _logwarn("failed to fetch document '%s': %s" % (s['html'], str(e)))

        imgdata = {}
        imgurl = None
        for (name, url) in imgrefs.iteritems():
          try:
            imgurl = urlparse.urljoin(s['html'], url)
            img = urllib2.urlopen(imgurl)
            info = img.info()
            imgdata[name] = {'data': img.read(), 'type': info['Content-Type']}
          except Exception, e:
            _logwarn("failed to fetch image %s from '%s': %s" % (name, imgurl, str(e)))
            imgdata[name] = None

        self.lock.acquire()
        for (name, info) in imgdata.iteritems():
          self.images[name] = info
        self.lock.release()

      t = time.time()
      while True:
        time.sleep(0.5)
        self.lock.acquire()
        stopme = self.stopme
        self.lock.release()
        if stopme or time.time() - t > self.period:
          break
