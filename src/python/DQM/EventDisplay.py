#!/usr/bin/env python

from stat import *
from copy import deepcopy, copy
from threading import Thread, Lock
from Monitoring.Core.Utils import _logerr, _logwarn, _loginfo, natsorted
from cherrypy import expose, HTTPError, tree, response, engine, thread_data, log
from cherrypy.lib.static import serve_file
import os, re, time, shutil, tempfile

# --------------------------------------------------------------------
# Event display file upload interface.
class EVDSnapshotUpload:
  STATUS_OK               = 100 # Requested operation succeeded
  STATUS_BAD_REQUEST      = 200 # The request is malformed
  STATUS_ERROR_PARAMETER  = 300 # Request parameter value is unacceptable.
  STATUS_ERROR_EXISTS     = 301 # Cannot overwrite an existing object.
  STATUS_ERROR_NOT_EXISTS = 302 # Requested file does not exist.
  STATUS_FAIL_EXECUTE     = 400 # Failed to execute the request.

  def refresh(self, *args): pass
  def __init__(self, server, datadir):
    self.lock = Lock()
    self.server = server
    self.datadir = datadir
    if not os.path.exists(datadir):
      os.makedirs(datadir)
    tree.mount(self, script_name=server.baseUrl + "/iguana-snapshot",
               config={"/": {'request.show_tracebacks': False}})

  # Set response headers to indicate our status data.
  def _status(self, code, message, detail=None):
    response.headers['evd-status-code'] = str(code)
    response.headers['evd-status-message'] = message
    response.headers['evd-status-detail'] = detail

  # Set response headers to indicate an error, then get out.
  def _error(self, code, message, detail=None):
    _logerr("code=%d, message=%s, detail=%s" % (code, message, detail))
    self._status(code, message, detail)
    raise HTTPError(500, message)

  # Check that a required parameter has been given just once,
  # and the value matches the given regular expression.
  def _check(self, name, arg, rx):
    if not arg or not isinstance(arg, str):
      self._error(self.STATUS_BAD_REQUEST,
                  "Incorrect or missing %s parameter" % name,
                  "Must provide single argument")
    if not re.match(rx, arg):
      self._error(self.STATUS_BAD_REQUEST,
                  "Malformed %s argument" % name,
                  "Argument must match regular expression '%s'" % rx)

  # ------------------------------------------------------------------
  # Store an event display image to the server.  Validates all the
  # parameters then attempts to save the file safely.  Sets headers in
  # the response to indicate what happened and the operation success.
  @expose
  def store(self, file=None, size=None, meta=None, *args, **kwargs):
    # Basic argument validation.
    if file == None \
       or not getattr(file, "file", None) \
       or not getattr(file, "filename", None):
      self._error(self.STATUS_BAD_REQUEST,
                  "Incorrect or missing file argument",
                  "Must provide a single file-type argument")

    self._check("filename", file.filename, r"^screenShot-[\d:.]+-[\d.]+\.png$")
    self._check("size",     size,          r"^\d+$")
    self._check("meta",     meta,          r"^.+\n")

    size = int(size)
    (base, time, date) = re.match(r"(.*)-([\d:.]+)-([\d.]+)\.png", file.filename).groups()

    # Determine where we would save this file.
    dir = "%s/%s" % (self.datadir, date)
    fname = "%s/%s" % (dir, file.filename)
    tmp = None

    if os.path.exists(fname) or os.path.exists(fname + ".info"):
      self._error(self.STATUS_ERROR_EXISTS,
                  "File exists, cannot overwrite", fname)

    # Try saving the file safely.  First we write the file to a
    # temporary directory, with a fake name, to the same directory as
    # the final file would be.  We then move it in place atomically
    # and leave a marker to indicate where the MSS replica of the file
    # can be found. If anything goes wrong, clean up so the upload can
    # be re-attempted later without tripping over the checks above.
    try:
      self.lock.acquire()
      if not os.path.exists(dir):
        os.makedirs(dir)
      (fd, tmp) = tempfile.mkstemp(".upload", "", dir)
      nsaved = 0
      while True:
        data = file.file.read(32*1024)
        if not data:
          break
        os.write(fd, data)
        nsaved += len(data)
      os.close(fd)
      os.chmod(tmp, 0644)
      if nsaved != size:
        self._error(self.STATUS_FAIL_EXECUTE,
                    "Failed to save file data",
                    "Wrote %d bytes, expected to write %d" % (nsaved, size))
      shutil.move(tmp, fname)
      open(fname + ".info", "w").write(meta)
      self.lock.release()

    except Exception, e:
      self.lock.release()
      if os.path.exists(fname):
        os.remove(fname)
      if os.path.exists(fname + ".info"):
        os.remove(fname + ".info")
      if tmp and os.path.exists(tmp):
        os.remove(tmp)
      if isinstance(e, HTTPError):
        raise e
      self._error(self.STATUS_FAIL_EXECUTE,
                  "Failed to save file data",
                  str(e).replace("\n", "; "))

    # Indicate success.
    self._status(self.STATUS_OK, "File saved", "Wrote %d bytes" % nsaved)
    _loginfo("saved file path=%s size=%d" % (fname, size))
    return "Thanks.\n"

# --------------------------------------------------------------------
# Event display snapshot access.
class EVDSnapshotSource:
  # Notify plot fairy the sub-url where we serve images.
  plothook = "iguana-snapshot"

  def __init__(self, server, statedir, datadir):
    self.datadir = datadir

  def plot(self, date, time, *path, **options):
    if re.match(r"^[\d:.]+$", date + time):
      date = ".".join(date.split(".")[::-1])
      path = "%s/%s/screenShot-%s-%s.png" % (self.datadir, date, time, date)
      if os.path.exists(path):
        return ("image/png", open(path).read())
    return (None, None)

# --------------------------------------------------------------------
# Event display workspace.
#
# .sessiondef
# .statedef
# .gui
# .name
# .datadir

class EVDSnapshotWorkspace:
  sessiondef = { 'evd.date': "", 'evd.page': "" }
  def __init__(self, gui, rank, category, name, datadir):
    gui._addJSFragment("%s/javascript/DQM/EVD.js" % gui.contentpath)
    self.gui       = gui
    self.rank      = rank
    self.category  = category
    self.name      = name
    self.datadir   = datadir

  # -----------------------------------------------------------------
  def _state(self, session):
    available = {}
    for dir, subdirs, files in os.walk(self.datadir):
      if re.match(r"^.*/\d\d.\d\d.\d\d\d\d$", dir):
        l = [f for f in files if f.startswith("screenShot-") and f.endswith(".png")]
	date = ".".join(dir.rsplit("/", 1)[-1].split(".")[::-1])
        if len(l): available[date] = natsorted(l)

    dates = natsorted(available.keys())[::-1]

    date = session.get('evd.date', '')
    if date in available:
      images = available[date]
    elif dates:
      date = dates[0]
      images = available[date]
    else:
      images = []

    page = session.get('evd.page', '')
    if len(images) == 0:
      page = 0
      image = ''
    else:
      if page == '':
        page = len(images)
      else:
        page = min(int(page), len(images))
      image = images[page-1].split('-')[1]

    self.gui._noResponseCaching()
    return ("([{kind: 'AutoUpdate', interval: %d, stamp: %d},"
            + "{kind: 'DQMHeaderRow', run: null, lumi: null, event: null,"
            + " service: '%s', workspace: 'Event display', page: %d, pages: %d,"
            + " services: [%s], workspaces: [%s], runs: [], runid: null },"
            + "{kind: 'EVD', dates: %s, date: '%s', images: %d, curimage: %d, image: '%s'}])") \
      % (300, self.gui.stamp, self.gui.serviceName, min(page, len(images)), len(images),
	 ",".join("{title: '%s', href: '%s'}"
                  % (x[0], x[1]) for x in self.gui.services),
	 ",".join("{title: '%s', label: '%s', category: '%s', rank: %d}"
                  % (x.name, x.name.lower(), x.category, x.rank)
                  for x in self.gui.workspaces),
         dates, date, len(images), page, image)

  # -----------------------------------------------------------------
  # Initialise a new session with the "start" URL.
  def start(self, session, date = None, page = None, **kwargs):
    if date != None:
      if not isinstance(date, str) or not re.match(r"^[\d.]*$", date):
        raise HTTPError(500, "Incorrect date parameter")
      session['evd.date'] = date

    if page != None:
      if not isinstance(page, str) or not re.match(r"^\d*$", page):
        raise HTTPError(500, "Incorrect page parameter")
      if page == '0':
        page = '1'
      session['evd.page'] = page

  def initialiseSession(self, session):
    for (var, value) in self.sessiondef.iteritems():
      if var not in session:
        session[var] = deepcopy(value)

  # -----------------------------------------------------------------
  # Return JSON object for the current session state.  This method is
  # invoked when the GUI session page is loaded, which occurs either
  # when a completely new session has been started, the "reload"
  # button is pushed or the session URL was copied between browser
  # windows, or the session wants a period refresh.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

  # -----------------------------------------------------------------
  # Set parameter selection.
  def sessionSet(self, session, *args, **kwargs):
    self.start(session, date=kwargs.get('date', ''), page=kwargs.get('page', ''))
    self.gui._saveSession(session)
    return self._state(session)

  def sessionSetPage(self, session, *args, **kwargs):
    self.start(session, page=kwargs.get('page', ''))
    self.gui._saveSession(session)
    return self._state(session)
