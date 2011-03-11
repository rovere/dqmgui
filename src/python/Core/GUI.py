#!/usr/bin/env python

# *IMPORTANT NOTE ON THREADS AND LOCKING*: Python currently supports
# multi-threading, but the interpreter is not multi-threaded.  The
# interpreter has a global lock, which it yields on "slow" operations
# such as file and socket operations.  On "straight" python code the
# interpreter yields the lock only every N byte code instructions;
# this server configures a large N (1'000'000).

from copy import deepcopy
from cgi import escape
from socket import gethostname
from threading import Thread, Lock
from cherrypy import expose, HTTPError, request, response, engine, log
from cherrypy.lib.static import serve_file
from Cheetah.Template import Template
from Monitoring.Core.Utils import _logerr, _logwarn, _loginfo
from cStringIO import StringIO
from stat import *
from jsmin import jsmin
import cPickle as pickle
import sys, os, os.path, re, tempfile, time, inspect, logging, traceback, hashlib

_SESSION_REDIRECT = ("<html><head><script>location.replace('%s')</script></head>"
                     + "<body><noscript>Please enable JavaScript to use this"
                     + " service</noscript></body></html>")

def import_module(name):
  mod = __import__(name)
  for part in name.split('.')[1:]:
    mod = getattr(mod, part, None)
    if not mod: break
  return mod
  
def extension(modules, what, *args):
  for m in modules:
    ctor = getattr(m, what, None)
    if ctor:
      return ctor(*args)
  _logwarn("extension '%s' not found" % what)
  return None

# -------------------------------------------------------------------
# Background thread for managing the server's sessions.
#
# Saving each session in the HTTP service threads reduces server
# responsiveness.  For a session modified at a high burst rate in
# asynchronous requests it would be pointless to save every
# modifiation to disk.
#
# The server runs this separate thread to save modified sessions back
# to disk in python's pickled format.  It is essential the per-session
# data is of limited size as pickling is slow.  The output is done in
# as safe a manner as possible to avoid data loss if the server
# crashes or is restarted, and even if it is terminated forcefully
# with SIGKILL.  Modified sessions are written out about once a
# second.
#
# ._stopme   Set by main server thread to indicate it's time to exit.
# ._path     The directory where the sessions are stored.
# ._save     A dictionary of sessions that must be saved.
class SessionThread(Thread):
  def __init__(self, path):
    Thread.__init__(self, name="GUI session thread")
    self._lock = Lock()
    self._stopme = False
    self._path = path
    self._save = {}

  # Record a session for saving.  The session's data in VALUE will
  # become the property of this thread exclusively.
  def save(self, data):
    self._lock.acquire()
    self._save[data['core.name']] = data
    self._lock.release()

  # Tell the thread to stop after it has flushed all the data to disk.
  def stop(self):
    self._lock.acquire()
    self._stopme = True
    self._lock.release()

  # The thread run loop.  Checks for dirty sessions about once a
  # second, and if there are any, grabs the list of currently dirty
  # sessions and starts writing them to disk in pickled format.
  # Manipulates session files as safely as possible.
  def run(self):
    while True:
      self._lock.acquire()
      stopme = self._stopme
      save = self._save
      self._save = {}
      self._lock.release()
      for (name, data) in save.iteritems():
        path = self._path + '/' + name
        tmppath = path + ".tmp"
        f = file(tmppath, "w")
        pickle.dump(data, f)
        f.close()

        try:
          os.remove(path)
        except os.error:
          pass

        try:
          os.rename(tmppath, path)
        except os.error:
          self._lock.acquire()
          self._save[name] = data
          self._lock.release()

      if stopme:
	break
      time.sleep(1)

# -------------------------------------------------------------------
# The main server process, a CherryPy actor mounted to the URL tree.
# The basic server core orchestrates basic services such as session
# management, templates, static content and switching workspaces.
# Behaviour beyond these basic functions is delegated to workspaces;
# all other invocations are delegated to the current workspace of the
# session in question.
#
# Note that all volatile user data is stored in session objects.  The
# backends and worspaces cache data to store global state that is not
# pertinent to any particular user/browser session, allowing them to
# respond HTTP requests as quickly as possible (ideally using only
# data currently in memory).
#
# Configuration data:
# .title            Web page banner title.
# .baseUrl          URL root for this service.
# .serviceName      Label for this service, to show on the web page.
# .services         List of (label, url) for all monitoring services.
# .workspaces       List of workspaces configured for this service.
# .sources          List of data sources attached to this service.
# .sessiondir       Directory where session data is kept.
# .logdir           Directory where logs are sent.
#
# Dynamic data or automatically determined configuration:
# .contentpath      Directory where static content will be found.
# .templates        Cheetah template files in .contentpath.
# .sessions         Currently active sessions.
# .sessionthread    Background thread for saving sessions.
# .lock             Lock for modifying variable data.
# .stamp            Server start time for forcing session reloads.
# .checksums        Server integrity check of source files.
class Server:
  def __init__(self, cfgfile, cfg, modules):
    modules = map(import_module, modules)
    self.instrument = cfg.instrument
    self.checksums = []
    self.stamp = time.time()
    self.lock = Lock()
    self.services = cfg.services
    self.serviceName = cfg.serviceName
    self.templates = {}
    self.css = []
    self.js = []

    monitor_root = os.getenv("MONITOR_ROOT")
    if os.access("%s/xdata/templates/index.tmpl" % monitor_root, os.R_OK):
      self.contentpath = "%s/xdata" % monitor_root
    else:
      self.contentpath = "%s/data" % monitor_root

    self.baseUrl = cfg.baseUrl
    self.sessiondir = cfg.serverDir + '/sessions'
    self.logdir = cfg.logFile.rsplit('/', 1)[0]
    self.title = cfg.title
    for file in os.listdir(self.contentpath + "/templates"):
      m = re.match(r'(.*)\.tmpl$', file)
      if m:
        (base,) = m.groups()
        filename = "%s/templates/%s" % (self.contentpath, file)
        self.templates[base] = [ filename, os.stat(filename)[ST_MTIME], open(filename).read() ]

    self._yui   = os.getenv("YUI_ROOT") + "/build"
    self._extjs = os.getenv("EXTJS_ROOT")
    self._addCSSFragment("%s/css/Core/style.css" % self.contentpath)
    self._addJSFragment("%s/yahoo/yahoo.js" % self._yui)
    self._addJSFragment("%s/event/event.js" % self._yui)
    self._addJSFragment("%s/connection/connection.js" % self._yui)
    self._addJSFragment("%s/dom/dom.js" % self._yui)
    self._addJSFragment("%s/javascript/Core/sprintf.js" % self.contentpath)
    self._addJSFragment("%s/javascript/Core/Utils.js" % self.contentpath)
    self._addJSFragment("%s/javascript/Core/Core.js" % self.contentpath)

    self.sessions = {}
    self.sessionthread = SessionThread(self.sessiondir)
    self.extensions = [extension(modules, e[0], self, *e[1])
                       for e in cfg.extensions]
    self.sources = [extension(modules, s[0] + "Source", self,
                              cfg.serverDir + '/' + s[1], *s[2])
                    for s in cfg.sources]
    self.workspaces = [extension(modules, w[0] + "Workspace", self, *w[1])
                       for w in cfg.workspaces]

    for w in self.workspaces:
      if getattr(w, 'customise', None):
        w.customise()

    self._addJSFragment("%s/javascript/Core/End.js" % self.contentpath)
    self._addChecksum(None, cfgfile, open(cfgfile).read())
    for name, m in sys.modules.iteritems():
      if ((name.startswith("Monitoring.")
           and name.count(".") % 2 == 0
           and name.rsplit(".", 1)[-1][0].isupper())
          or name == "__main__") \
	 and m and m.__dict__.has_key('__file__'):
        self._addChecksum(name,
			  inspect.getsourcefile(m) \
			  or inspect.getabsfile(m)
			  or name,
			  inspect.getsource(m))

    self.sessionthread.start()
    engine.subscribe('stop', self.sessionthread.stop)

  def _addChecksum(self, modulename, file, data):
    s = (os.path.exists(file) and os.stat(file)) or None
    self.checksums.append({
      'module': modulename,
      'srcfile': file,
      'srcname': file.rsplit("/", 1)[-1],
      'mtime': (s and s[ST_MTIME]) or -1,
      'srclen': (s and s[ST_SIZE]) or -1,
      'srcmd5': hashlib.md5(data).digest().encode('hex')
    })

  def _maybeRefreshFile(self, dict, name):
    self.lock.acquire()
    fileinfo = dict[name]
    mtime = os.stat(fileinfo[0])[ST_MTIME]
    if mtime != fileinfo[1]:
      fileinfo[1] = mtime
      fileinfo[2] = open(fileinfo[0]).read()
    self.lock.release()
    return fileinfo[2]

  def _templatePage(self, name, variables):
    template = self._maybeRefreshFile(self.templates, name)
    params = { 'CSS':        "".join(x[1] for x in self.css),
               'JAVASCRIPT': "".join(x[1] for x in self.js) }
    return str(Template(template, searchList=[variables, params]))

  def _noResponseCaching(self):
    response.headers['Pragma'] = 'no-cache'
    response.headers['Expires'] = 'Sun, 19 Nov 1978 05:00:00 GMT'
    response.headers['Cache-Control'] = \
      'no-store, no-cache, must-revalidate, post-check=0, pre-check=0'

  def _addCSSFragment(self, filename):
    if not dict(self.css).has_key(filename):
      text = file(filename).read()
      if filename.startswith(self._yui):
        path = filename[len(self._yui)+1:].rsplit('/', 1)[0]
        text = re.sub(r"url\((\.\./)+([-a-z._/]+)\)", r"url(%s/yui/\2)" % self.baseUrl,
                 re.sub(r"url\(([-a-z._]+)\)", r"url(%s/yui/%s/\1)" % (self.baseUrl, path),
                   text))
      if self._extjs and filename.startswith(self._extjs):
        path = filename[len(self._extjs)+1:].rsplit('/', 1)[0]
        text = re.sub(r"url\((\.\./)+([-a-z._/]+)\)", r"url(%s/extjs/resources/\2)" % self.baseUrl,
                 re.sub(r"url\(([-a-z._]+)\)", r"url(%s/extjs/resources/%s/\1)" % (self.baseUrl, path),
                   text))
      clean = re.sub(r'\n+', '\n',
                re.sub(re.compile(r'\s+$', re.M), '',
                  re.sub(re.compile(r'^[ \t]+', re.M), ' ',
                    re.sub(r'[ \t]+', ' ',
                      re.sub(r'/\*(?:.|[\r\n])*?\*/', '', text)))))
      self.css += [(filename, "\n" + clean + "\n")]

  def _addJSFragment(self, filename, minimise=True):
    if not dict(self.js).has_key(filename):
      text = file(filename).read()
      if minimise:
        text = jsmin(text)
      self.js += [(filename, "\n" + text + "\n")]

  # -----------------------------------------------------------------
  # Session methods.
  def _sessionClientData(self):
    for id in ('CMS-AUTH-CERT', 'CMS-AUTH-HOST'):
      if id in request.headers:
	return "%s:%s" % (id, request.headers[id])
    return request.remote.ip

  # Check NAME is valid and a known session.  If yes, returns the
  # session data, otherwise None.  Locks the session before returning
  # it, making sure all other threads have released the session.  The
  # caller _MUST_ release the session lock before the HTTP request
  # handling returns, or the next access to the session will hang.
  def _getSession(self, name):
    s = None
    if re.match("^[-A-Za-z0-9_]+$", name):
      self.lock.acquire()
      if name not in self.sessions:
	path = "%s/%s" % (self.sessiondir, name)
        if os.path.exists(path):
	  try:
            self.sessions[name] = pickle.load(file(path, "r"))
	  except Exception, e:
	    _logerr("FAILURE: cannot load session data: " + str(e))

      s = self.sessions.get(name, None)
      self.lock.release()

      if s:
        current = self._sessionClientData()
        if s['core.clientid'] == current and s['core.name'] == name:
          if 'core.lock' not in s:
            s['core.lock'] = Lock()
          s['core.lock'].acquire()
	  s['core.stamp'] = time.time()
	else:
	  s = None
    return s

  # Save the SESSION state.
  def _saveSession(self, session):
    self.lock.acquire()
    session['core.stamp'] = time.time()
    self.sessions[session['core.name']] = session
    self.lock.release()
    self.sessionthread.save(dict((k, deepcopy(session[k]))
                                 for k in session.keys()
                                 if k != 'core.lock'))

  # Release the SESSION for use by other threads.
  def _releaseSession(self, session):
    session['core.lock'].release()

  # Create and initialise a new session with some default workspace.
  # This just initialises a session; it will not become locked.
  def _newSession(self, workspace):
    # Before creating a new one, purge from memory sessions that have
    # not been used for 15 minutes, to avoid building up memory use.
    self.lock.acquire()
    old = time.time() - 900
    for key in [k for k, s in self.sessions.iteritems()
		if s['core.stamp'] < old]:
      del self.sessions[key]
    self.lock.release()

    # Generate a new session key.
    (fd, path) = tempfile.mkstemp("", "", self.sessiondir)
    sessionid = (path.split('/'))[-1]
    os.close(fd)

    # Build session data.  We record:
    #  - client data for later verification
    #  - which workspace we are in
    session = {}
    session['core.name'] = sessionid
    session['core.clientid'] = self._sessionClientData()
    session['core.public'] = False
    session['core.workspace'] = self.workspaces[0].name

    user = session['core.clientid']
    if user.startswith("CMS-AUTH-CERT"):
      user = [x for x in re.findall(r"/CN=([^/]+)", user)]
      neat = [x for x in user if x.find(" ") >= 0]
      if len(neat):
	session['core.user'] = neat[0]
      else:
	session['core.user'] = user[0]
    elif user.startswith("CMS-AUTH-HOST"):
      session['core.user'] = "Console %s" % user.split(" ")[-1]
    else:
      session['core.user'] = "Host %s" % user

    # Get default objects in the workspace.
    workspace = workspace.lower()
    for w in self.workspaces:
      if w.name.lower() == workspace:
        session['core.workspace'] = w.name
      w.initialiseSession(session)

    # Get defaults also from the sources.
    for s in self.sources:
      if getattr(s, 'prepareSession', None):
        s.prepareSession(session)

    # Save the pickled session state file.
    self._saveSession(session)

    # Return the final component of the session path.
    return sessionid

  # Tell the client browser the URL is invalid.
  def _invalidURL(self):
    return self._templatePage("invalid", {
	'TITLE'		 : re.sub(r"\&\#821[12];", "-", self.title),
	'HEADING'	 : self.title,
	'URL'		 : escape(request.request_line.split(' ')[1]),
	'ROOTPATH'	 : self.baseUrl,
	'NEWSESSION'	 : self.baseUrl,
	'HOSTNAME'	 : gethostname(),
      })

  # Get the workspace object corresponding to NAME.  If no such
  # workspace exists, returns the first (= default) workspace.
  def _workspace(self, name):
    name = name.lower()
    for w in self.workspaces:
      if w.name.lower() == name:
	return w
    return self.workspaces[0]

  # -----------------------------------------------------------------
  # Server access points.

  # Main root index address: the landing address.  Create a new
  # session and redirect the client there.
  @expose
  def index(self):
    return self.start(workspace = self.workspaces[0].name);

  # Access our own static content.
  @expose
  def static(self, *args, **kwargs):
    if len(args) != 1 or not re.match(r"^[-a-z]+\.(png|gif)$", args[0]):
      return self._invalidURL()
    return serve_file(self.contentpath + '/images/' + args[0])

  # Access YUI static content.
  @expose
  def yui(self, *args, **kwargs):
    path = "/".join(args)
    if not re.match(r"^[-a-z_/]+\.(png|gif)$", path):
      return self._invalidURL()
    return serve_file(self._yui + '/' + path)

  # Access ExtJS static content.
  @expose
  def extjs(self, *args, **kwargs):
    path = "/".join(args)
    if not (self._extjs and re.match(r"^[-a-z_/]+\.(png|gif)$", path)):
      return self._invalidURL()
    return serve_file(self._extjs + '/' + path)

  # -----------------------------------------------------------------
  # Jump to some content.  This creates and configures a new session with
  # the desired content, as if a sequence of actions was carried out.
  #
  # In the end, redirects the client browser to a new session URL.  We
  # send a HTML page with JavaScript to change the page "location",
  # rather than raise a HTTPRedirect.  The main reason is HTTPRedirect
  # results in HTTP 303 response and web browsers remember the
  # original address not the new one we send to them.  If the user
  # would then reload the page, they would not be sent back to their
  # session but back to the root address which would create them again
  # another session.  The second and minor reason is that we can
  # verify that JavaScript is enabled in the client browser.
  @expose
  def start(self, *args, **kwargs):
    if len(args) != 0:
      return self._invalidURL()
    workspace = self._workspace(kwargs.get("workspace", self.workspaces[0].name))
    sessionid = self._newSession(workspace.name)
    session = self._getSession(sessionid)
    workspace.start(session, **kwargs)
    self._saveSession(session)
    self._releaseSession(session)
    return _SESSION_REDIRECT % (self.baseUrl + "/session/" + sessionid)

  # Backward compatible version of 'start' which understands one
  # parameter, the name of the workspace to begin in.  Note that
  # within a session the sessionWorkspace method is used to switch
  # between workspaces.
  @expose
  def workspace(self, *args, **kwargs):
    if len(args) == 1:
      return self.start(workspace = self._workspace(args[0]).name)
    else:
      return self.start(workspace = self.workspaces[0].name)

  # -----------------------------------------------------------------
  # General session-independent access path for dynamic images.
  # The first subdirectory argument contains the name of the
  # "source plot hook" able to handle the plotting request.
  # The rest of the processing is given over to the hook.
  @expose
  def plotfairy(self, *args, **kwargs):
    try:
      if len(args) >= 1:
        for s in self.sources:
          if getattr(s, 'plothook', None) == args[0]:
            (type, data) = s.plot(*args[1:], **kwargs)
            if type != None:
              self._noResponseCaching()
              response.headers['Content-Length'] = str(len(data))
              response.headers['Content-Type'] = type
              return data
            break
    except Exception, e:
      o = StringIO()
      traceback.print_exc(file=o)
      log("WARNING: unable to produce a plot: "
          + (str(e) + "\n" + o.getvalue()).replace("\n", " ~~ "),
          severity=logging.WARNING)

    self._noResponseCaching()
    return serve_file(self.contentpath + "/images/missing.png",
                      content_type = "image/png")

  # -----------------------------------------------------------------
  # Report code running in this server.
  @expose
  def digest(self, *args, **kwargs):
    maxsize = max(len(str(i['srclen'])) for i in self.checksums) + 1
    maxlen = max(len(str(i['module'])) for i in self.checksums) + 2
    fmt = "%(srclen)-" + str(maxsize) \
	  + "d %(mtime)-10d  %(srcmd5)s  %(module)-" + str(maxlen) \
          + "s %(srcname)s\n"
    summary = ""

    self.checksums.sort(lambda a, b: cmp(a['srcfile'], b['srcfile']))
    for i in self.checksums:
      summary += fmt % i

    response.headers['Content-Type'] = "text/plain"
    return summary

  # -----------------------------------------------------------------
  # A hook for authenticating users.  We don't actually authenticate
  # anyone here, all the authentication is done in the front-end
  # reverse proxy servers.  But do provide a URL authentication can
  # use to retrieve the required proxy cookies.
  @expose
  def authenticate(self, *args, **kwargs):
    response.headers['Content-Type'] = "text/plain"
    return "Authenticated"

  # -----------------------------------------------------------------
  # Main session address.  All AJAX calls to the session land here.
  # The URL is of the form "[/ROOT]/session/ID[/METHOD].  We check
  # the session ID is valid for this user, and the METHOD is one we
  # support.  A METHOD "foo" results in call to "sessionFoo()" in
  # this class.  If no METHOD is given, default to "index": generate
  # the main GUI page. All other METHODs are AJAX calls from the
  # client, which normally just return a JSON result object.
  @expose
  def session(self, *args, **kwargs):
    # If the URL has been truncated, just start a new session.
    if len(args) < 1:
      return self.start(workspace = self.workspaces[0].name)

    # If the URL specifies extra arguments, reply with an error page.
    sessionid = args[0]
    if len(args) > 2:
      return self._invalidURL()

    # Check the session is something we can use.  This locks the
    # session so no other thread can access it at the same time.
    session = self._getSession(sessionid)
    if not session:
      return self._invalidURL()

    # Find the session method to invoke and verify we have a good one.
    # Look for the method first in this server object, and failing
    # that in the current workspace object.  Release the session lock
    # on the way out.
    try:
      methodName = 'sessionIndex'
      if len(args) == 2:
        methodName = 'session' + args[1][0].upper() + args[1][1:]

      method = None
      workspace = session['core.workspace']
      if workspace:
        method = getattr(self._workspace(workspace), methodName, None)
      if not method:
        method = getattr(self, methodName, None)
      if not method:
        return self._invalidURL()

      # Let the session method handle the rest.
      return method(session, *args, **kwargs)
    finally:
      self._releaseSession(session)

  # Generate top level session index.  This produces the main GUI web
  # page, with practically no content in it; the client will contact
  # us for the content using AJAX calls.
  def sessionIndex(self, session, *args ,**kwargs):
    return self._templatePage("index", {
	'TITLE'		 : re.sub(r"\&\#821[12];", "-", self.title),
	'HEADING'	 : self.title,
        'WORKSPACE'	 : session['core.workspace'],
        'SESSION_ID'     : session['core.name'],
        'SESSION_STATUS' : 'modifiable',
	'USER'		 : session['core.user'],
        'HOSTNAME'       : gethostname(),
	'ROOTPATH'	 : self.baseUrl
      });

  # Switch session to another workspace.
  def sessionWorkspace(self, session, *args, **kwargs):
    workspace = self._workspace(kwargs.get('name', session['core.workspace']))
    session['core.workspace'] = workspace.name
    workspace.initialiseSession(session)
    self._saveSession(session)
    return workspace.sessionState(session)

  # -----------------------------------------------------------------
  # Drop a profile dump if running under profiler and the current
  # workspace has a suitable extension (aka igprof native call).
  def sessionProfileSnapshot(self, session, *args, **kwargs):
    if self.instrument and self.instrument.startswith("igprof "):
      snapshot = None
      workspace = session['core.workspace']
      if workspace:
        snapshot = getattr(self._workspace(workspace), "_profilesnap", None)
      if snapshot:
        snapshot()
    return "OK"

  # Return JSON object for the current session state.  This method is
  # invoked when the GUI session page is loaded, which occurs either
  # when a completely new session has been started, the "reload"
  # button is pushed or the session URL was copied between browser
  # windows, or the session wants a period refresh.
  def sessionState(self, session, *args, **kwargs):
    raise HTTPError(500, "Internal implementation error")
