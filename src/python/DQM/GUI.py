#!/usr/bin/env python

from stat import *
from copy import deepcopy, copy
from threading import Thread, Lock
from Monitoring.DQM import Accelerator
from Monitoring.Core.Utils import _logerr, _logwarn, _loginfo, natsorted, thousands
from cherrypy import expose, HTTPError, HTTPRedirect, tree, request, response, engine, thread_data, log, url
from cherrypy.lib.static import serve_file
from cherrypy.lib import cptools, http
from struct import pack, unpack
from cStringIO import StringIO
import os, re, time, socket, shutil, tempfile, cgi

DEF_DQM_PORT = 9090

# Validate DQM file paths.
RX_SAFE_PATH = re.compile(r"^[A-Za-z0-9][-A-Za-z0-9_]*(?:\.(?:root|zip|ig))?$")

# --------------------------------------------------------------------
# Utility type for providing layouts from python configuration files.
class DQMItem:
  def __init__(self, layout=None):
    self.layout = layout

# --------------------------------------------------------------------
# DQM extension to manage DQM file uploads.
class DQMFileAccess:
  STATUS_OK               = 100 # Requested operation succeeded
  STATUS_BAD_REQUEST      = 200 # The request is malformed
  STATUS_ERROR_PARAMETER  = 300 # Request parameter value is unacceptable.
  STATUS_ERROR_EXISTS     = 301 # Cannot overwrite an existing object.
  STATUS_ERROR_NOT_EXISTS = 302 # Requested file does not exist.
  STATUS_FAIL_EXECUTE     = 400 # Failed to execute the request.

  def refresh(self, *args): pass
  def __init__(self, server, aclfile, uploads, roots):
    self.lock = Lock()
    self.server = server
    self.uploads = uploads
    self.roots = roots
    if uploads and not os.path.exists(uploads):
      os.makedirs(uploads)
    tree.mount(self, script_name=server.baseUrl + "/data",
               config={"/": {'request.show_tracebacks': False}})

  # Set response headers to indicate our status data.
  def _status(self, code, message, detail=None):
    response.headers['dqm-status-code'] = str(code)
    response.headers['dqm-status-message'] = message
    response.headers['dqm-status-detail'] = detail

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

  # Check the current request is into a directory, and if not,
  # redirect to add a trailing slash.
  def _prepdir(self, time):
    pi = request.path_info
    if not pi.endswith('/'):
      raise HTTPRedirect(url(pi + '/', request.query_string))
    request.is_index = True
    response.headers['Content-Type'] = 'text/html'
    response.headers['Last-Modified'] = http.HTTPDate(time)
    cptools.validate_since()

  # ------------------------------------------------------------------
  # Store a file to the server.  Validates all the parameters, then
  # attempts to save the file safely.  Sets headers in the response
  # to indicate what happened, either an error or success.
  # FIXME: Verify the request was submitted securely and is permitted.
  # FIXME: Determine producer from certificate information.
  @expose
  def put(self,
          size = None,
          checksum = None,
          file = None,
          *args,
          **kwargs):
    # Bail out if upload is not supported.
    if not self.uploads:
      raise HTTPError(404, "Not found")

    # Argument validation.
    if file == None \
       or not getattr(file, "file", None) \
       or not getattr(file, "filename", None):
      self._error(self.STATUS_BAD_REQUEST,
                  "Incorrect or missing file argument",
                  "Must provide a single file-type argument")

    self._check("size",     size,          r"^\d+$")
    self._check("checksum", checksum,      r"^(md5:[A-Za-z0-9]+|crc32:\d+)$")
    self._check("filename", file.filename, r"^[-A-Za-z0-9_]+\.root$")

    m = re.match(r"^(DQM)_V\d+(_[A-Za-z]+)?_R(\d+)(__.*)?\.root", file.filename)
    if not m:
      self._error(self.STATUS_ERROR_PARAMETER,
                  "File name does not match the expected convention")

    size = int(size)
    runnr = int(m.group(3))
    if m.group(2) and m.group(4):
      self._error(self.STATUS_ERROR_PARAMETER,
                  "File cannot have both online system and dataset names")
    elif m.group(4):
      self._check("workflow", m.group(4).replace("__", "/"),
		  r"^(/[-A-Za-z0-9_]+){3}$")

    # Determine where we would save this file.  Update file version
    # until we find a version that doesn't yet exist.  This permits
    # multiple concurrent upload versions of the same file.  We need
    # to lock around this so multiple concurrent uploads can't yield
    # the same version number for two separate uploads.
    try:
      self.lock.acquire()
      version = 1
      while True:
        dir = "%s/%04d" % (self.uploads, version)
        fname = "%s/%s" % (dir, file.filename)
        if not os.path.exists(fname) and \
	   not os.path.exists(fname + ".origin"):
	  break
        version += 1

      tmp = None
      if fname.find("..") >= 0 or fname.find("./") >= 0 or fname.find("/.") >= 0:
        self._error(self.STATUS_ERROR_PARAMETER, "Invalid file path name",
                    "Path name cannot refer to '.' or '..'")

      # Try saving the file safely.  First we write the file to a
      # temporary directory, with a fake name, to the same directory as
      # the final file would be.  We then move it in place atomically
      # and write a metadata descriptor file.  If anything goes wrong,
      # clean up so the upload can be re-attempted later.
      if not os.path.exists(dir):
        os.makedirs(dir)
      (fd, tmp) = tempfile.mkstemp(".upload", "", dir)
      nsaved = 0
      first = ""
      while True:
        data = file.file.read(8*1024*1024)
        if not data:
          break
	if len(first) < 5:
	  first += data[0:5]
        os.write(fd, data)
        nsaved += len(data)
      os.close(fd)
      os.chmod(tmp, 0644)
      if first[0:5] != "root\0":
	self._error(self.STATUS_ERROR_PARAMETER, "Not a ROOT file",
		    "File data contents do not represent a ROOT file")
      if nsaved != size:
        self._error(self.STATUS_FAIL_EXECUTE,
                    "Failed to save file data",
                    "Wrote %d bytes, expected to write %d" % (nsaved, size))
      shutil.move(tmp, fname)
      open(fname + ".origin", "w").write("%s %d %s\n" % (checksum, size, fname))
      self.lock.release()

    except Exception, e:
      self.lock.release()
      if os.path.exists(fname):
        os.remove(fname)
      if os.path.exists(fname + ".origin"):
        os.remove(fname + ".origin")
      if tmp and os.path.exists(tmp):
        os.remove(tmp)
      if isinstance(e, HTTPError):
        raise e
      self._error(self.STATUS_FAIL_EXECUTE,
                  "Failed to save file data",
                  str(e).replace("\n", "; "))

    # Indicate success.
    self._status(self.STATUS_OK, "File saved", "Wrote %d bytes" % nsaved)
    _loginfo("saved file %s size %d checksum %s" % (fname, size, checksum))
    return "Thanks.\n"

  # ------------------------------------------------------------------
  # Retrieve files from the server.  Pretends to be somewhat like the
  # apache mod_dir file browsing scheme.  Serves only files with valid
  # and safe path references.  FIXME: Bandwidth limiting and ACLs?
  @expose
  def browse(self, *path, **kwargs):
    rooturl = self.server.baseUrl + "/data/browse"
    if not self.roots or len(self.roots) == 0:
      raise HTTPError(404, "Not found")

    for x in path:
      if not re.match(RX_SAFE_PATH, x):
	raise HTTPError(404, "Not found")

    # If no path was given, provide list of top level roots.
    if len(path) == 0:
      self._prepdir(self.server.stamp)
      return "<html><head><title>Index of %(index)s</title></head><body>" \
             "<h1>%(index)s</h1><table border='0' valign='top' cellpadding='3'>" \
	     "%(files)s</table></body></html>" % \
             { 'index': cgi.escape("DQM files"),
	       'files': "\n".join(["<tr><td><a href='%(root)s/%(name)s/'>%(name)s"
				   "</a></td><td>&nbsp;</td><td>&nbsp;</td>"
				   % { 'root': rooturl, 'name': cgi.escape(x) }
				   for x in sorted(self.roots.keys())]) }

    # We have a path, check under the given root
    root = path[0]
    if root not in self.roots:
      raise HTTPError(404, "Not found")

    pathname = "/".join([self.roots[root]] + list(path[1:]))
    if not os.path.exists(pathname):
      raise HTTPError(404, "Not found")

    if os.path.isdir(pathname):
      try:
        st = os.stat(pathname)
        if not S_ISDIR(st.st_mode):
          raise HTTPError(404, "Not found")
      except OSError:
        raise HTTPError(404, "Not found")
      self._prepdir(st.st_mtime)
      files = [(x, pathname + "/" + x)
	       for x in os.listdir(pathname)
	       if re.match(RX_SAFE_PATH, x)]
      files = [(x[0], x[1], os.path.isdir(x[1]), os.stat(x[1]))
	       for x in files if not os.path.islink(x[1])]
      return "<html><head><title>Index of %(index)s</title></head><body>" \
             "<h1>%(index)s</h1><p style='padding-left:5px'>" \
	     "<a href='%(root)s/%(parent)s'>Up</a></p>" \
	     "<table border='0' valign='top' cellpadding='3'>" \
	     "%(files)s</table></body></html>" % \
             { 'index': cgi.escape("/".join(path)),
	       'parent': cgi.escape("/".join(path[:-1])), 'root': rooturl,
	       'files': "\n".join(["<tr><td><a href='%(root)s/%(path)s/%(name)s%(slash)s'>"
				   "%(name)s%(slash)s</a></td><td>%(size)s</td>"
				   "<td>%(mtime)s UTC</td></tr>"
				   % { 'root': rooturl,
				       'path': cgi.escape("/".join(path)),
				       'name': cgi.escape(x[0]),
				       'slash': (x[2] and '/') or "",
				       'size': (x[2] and '-') or x[3].st_size,
				       'mtime': time.strftime("%Y-%m-%d %H:%M:%S",
						  time.gmtime(x[3].st_mtime)) }
				   for x in sorted(files)[::-1]]) }
    else:
      return serve_file(pathname, content_type="application/octet-stream")

# --------------------------------------------------------------------
# DQM extension to manage DQM file uploads.
class DQMToJSON(Accelerator.DQMToJSON):
  def refresh(self, *args): pass
  def __init__(self, server):
    Accelerator.DQMToJSON.__init__(self)
    self.server = server
    tree.mount(self, script_name=server.baseUrl + "/data/json",
               config={"/": {'request.show_tracebacks': False}})

  @expose
  def samples(self, *args, **options):
    sources = dict((s.plothook, s) for s in self.server.sources
		   if getattr(s, 'plothook', None))
    (stamp, result) = self._samples(sources.values(), options)
    response.headers["Content-Type"] = "text/plain"
    response.headers["Last-Modified"] = http.HTTPDate(stamp)
    return result

  @expose
  def default(self, srcname, runnr, dsP, dsW, dsT, *path, **options):
    sources = dict((s.plothook, s) for s in self.server.sources
		   if getattr(s, 'plothook', None))
    layoutSrc = None
    for s in self.server.sources:
      if isinstance(s, DQMLayoutSource):
        layoutSrc = s
    if srcname in sources:
      (stamp, result) = self._list(layoutSrc,
                                   sources[srcname],
                                   int(runnr),
                                   "/".join(("", dsP, dsW, dsT)),
                                   "/".join(path), options)
      response.headers["Content-Type"] = "text/plain"
      response.headers["Last-Modified"] = http.HTTPDate(stamp)
      return result
    else:
      response.headers["Content-Type"] = "text/plain"
      response.headers["Last-Modified"] = http.HTTPDate(server.stamp)
      return "{}"

# --------------------------------------------------------------------
# Management interface for talking to the ROOT rendering process.
class DQMRenderLink(Accelerator.DQMRenderLink):
  def __init__(self, server, plugin, nproc = 5, debug = False):
    Accelerator.DQMRenderLink.__init__ \
      (self, server.sessiondir.rsplit('/', 1)[0],
       server.logdir, plugin, nproc, debug)
    engine.subscribe('start', lambda *args: self._start(), priority=1)
    engine.subscribe('stop', lambda *args: self._stop(), priority=1)

# --------------------------------------------------------------------
# Source for plotting non-existent DQM objects.  Feeds the names of
# missing objects the visDQMRender for "missing in action" image.
class DQMUnknownSource(Accelerator.DQMUnknownSource):
  def __init__(self, server, statedir):
    Accelerator.DQMUnknownSource.__init__(self)

  # Generate a "missing in action" image for an unknown object.
  def plot(self, *path, **options):
    return self._plot("/".join(path[4:]), options)

# --------------------------------------------------------------------
# Source for plotting overlaid DQM objects.  Requests the object from
# all other sources and sends them over for rendering.
class DQMOverlaySource(Accelerator.DQMOverlaySource):
  def __init__(self, server, statedir):
    Accelerator.DQMOverlaySource.__init__(self)
    self.server = server

  # Generate an overlaid image.  Finds all the servers sources
  # and generates final list of (source, runnr, dataset, path)
  # tuples to pass to C++ layer to process.
  def plot(self, *junk, **options):
    sources = dict((s.plothook, s) for s in self.server.sources
		   if getattr(s, 'plothook', None))

    objs = options.get("obj", [])
    if isinstance(objs, str): objs = [objs]

    final = []
    for o in objs:
      (srcname, runnr, dsP, dsW, dsT, path) = o.split("/", 5)
      if srcname in sources and srcname != "unknown":
        final.append((sources[srcname], int(runnr),
                      "/%s/%s/%s" % (dsP, dsW, dsT), path))
    return self._plot(final, options)

# --------------------------------------------------------------------
# Source for plotting strip charts of DQM objects.
class DQMStripChartSource(Accelerator.DQMStripChartSource):
  def __init__(self, server, statedir):
    Accelerator.DQMStripChartSource.__init__(self)
    self.server = server

  # Generate a strip chart.  Passes list of server sources, an
  # optional source for the "current" object choice, object path
  # and the render options to the C++ layer to process.
  def plot(self, *path, **options):
    if 'trend' not in options:
      raise HTTPError(500, "Missing trend argument")

    sources = dict((s.plothook, s) for s in self.server.sources
		   if getattr(s, 'plothook', None))
    info = None
    current = options.get("current", None)
    if current != None:
      if not isinstance(current, str):
        raise HTTPError(500, "Incorrect current option")
      (srcname, runnr, dataset) = current.split("/", 2)
      if srcname in sources and srcname != "unknown":
        info = (sources[srcname], int(runnr), "/" + dataset)

    return self._plot(sources.values(), info, "/".join(path), options)

# --------------------------------------------------------------------
# DQM data source which provides layout content from python
# configuration files.  There is no backend process attached to this
# data source.
#
# Takes a list of python configuration filenames as the configuration
# parameter.  The files contain code to initialise a static structure
# of layout definitions.
#
# Whenever the source is prompted to update its data, it first reviews
# the list of layout configuration files.  Changed files are reloaded;
# if the file fails to load it is skipped, although layout
# registrations up to the point of error are included.
#
# Once the layout definitions are up-to-date, simply returns the list
# of known layouts.  No checks are made to ensure the listed monitor
# elements actually exist; non-existent ones will simply generate a
# warning when rendered.
#
# This source does not produce plots.  Plots are provided by the
# sources which provide the objects the layouts refer to.  All the
# real functionality is implemented in the C++ layer.
class DQMLayoutSource(Accelerator.DQMLayoutSource):
  def __init__(self, server, statedir, *filenames):
    # Filter only existing files.
    files = [f for f in filenames if os.path.exists(f)]

    # Pass the filtered list to the implementation.
    Accelerator.DQMLayoutSource.__init__(self, files)

    # Add file checksums to server configuration.
    for f in files:
      server._addChecksum(None, f, open(f).read())

# --------------------------------------------------------------------
# DQM data source providing content from DQM data delivered over
# network from upstream "DQMCollector".
#
# Takes as parameter the options for the "visDQMRender" process.  This
# GUI server side object decodes those arguments and determines the
# host and port to connect to reach the backend processes.
#
# All the monitoring and processing is done in the C++ accelerator
# class this python class inherits from.  The python layer is just
# a glue layer to pass data between the two.
#
# This source works with visDQMRender to make object images: we get
# the object data via DQMCollector and pass it to visDQMRender which
# returns back the PNG image.  The image requests block until the
# render process returns the image data.  Each HTTP server thread
# has a semi-persistent socket to the render process; cf. ROOTImage.
# Note that there is no time out on retrieving image data from the
# upstream source, the DQM network layer will automatically time out
# after a while.  If the image rendering gets stuck, the process will
# automatically commit a suicide and restart.
#
# .opts         Configuration parameters
#   dataset     - Dataset name for online datasets (fixed)
#   verbose     - Flag to cause DQM net to be chatty or quiet (fixed)
#   dqmhost     - DQMCollector host name, taken from --collector option.
#   dqmport     - DQMCollector port, taken from --collector option.
#   plotport    - Port at which visDQMRender is listening, taken from --listen option.
class DQMLiveSource(Accelerator.DQMLiveSource):
  # Start off the live backend and hook up to server transitions.
  def __init__(self, server, statedir, collector, *params):
    self.opts = { "dataset": "/Global/Online/ALL",
                  "verbose": False,
                  "dqmhost": "localhost",
                  "dqmport": DEF_DQM_PORT }

    m = re.match(r"((\S+):)?(\d+)", collector)
    if m:
      if m.group(2) != "":
        self.opts['dqmhost'] = m.group(2)
      self.opts['dqmport'] = int(m.group(3))

    Accelerator.DQMLiveSource.__init__(self, server, self.opts)
    engine.subscribe('exit', lambda *args: self._exit(), priority=100)

  # Generate an object image given an object path and options.
  def plot(self, runnr, dsP, dsW, dsT, *path, **options):
    return self._plot("/".join(path), options)

# --------------------------------------------------------------------
# DQM data source providing content from archived DQM data files.
# All the real functionality is implemented in the C++ layer.  This
# source works with visDQMRender to make object images: we ship the
# streamer info and streamed objects to the backend and it returns
# back the PNG image.
#
# .opts         Configuration parameters
#   plotport    - Port at which visDQMRender is listening.
#   rxonline    - Regexp to recognise online dataset name.
#   index       - Path to the index directory.
class DQMArchiveSource(Accelerator.DQMArchiveSource):
  def __init__(self, server, statedir, indexdir, rxonline, *params):
    self.opts = { "index": indexdir, "rxonline": rxonline }
    Accelerator.DQMArchiveSource.__init__(self, server, self.opts)
    engine.subscribe('exit', lambda *args: self._exit(), priority=100)

  # Generate an object image given an object type ('scalar' or
  # 'rootobj'), the run number, dataset path, object name and render
  # options.  See ROOTImage for details about image generation.
  def plot(self, runnr, dsP, dsW, dsT, *path, **options):
    return self._plot(int(runnr), "/".join(('', dsP, dsW, dsT)),
		      "/".join(path), options)

# --------------------------------------------------------------------
# .sessiondef
# .gui
# .rank
# .category
# .name
# .match
# .warnings         Recent warnings printed to the logs.

class DQMWorkspace:
  sessiondef = { 'dqm.sample.vary':    "any",
                 'dqm.sample.order':   "dataset",
                 'dqm.sample.prevws':  "",
                 'dqm.sample.pattern': "",
                 'dqm.sample.dynsearch': "yes",
                 #'dqm.sample.type':    0,
                 #'dqm.sample.runnr':   0,
                 #'dqm.sample.dataset': "",
                 'dqm.play.prevws':    "",
                 'dqm.play.interval':  5,
                 'dqm.play.pos':       0,
                 #'dqm.panel.tools.show': 1,
                 #'dqm.panel.tools.x':  -1,
                 #'dqm.panel.tools.y':  -1,
                 #'dqm.panel.help.show': 1,
                 #'dqm.panel.help.x':  -1,
                 #'dqm.panel.help.y':  -1,
                 #'dqm.panel.custom.show': 1,
                 #'dqm.panel.custom.x':  -1,
                 #'dqm.panel.custom.y':  -1,
		 'dqm.qplot': 	       "",
                 'dqm.search':         "",
                 'dqm.filter':         "all",
                 'dqm.strip.type':     "object",
                 'dqm.strip.omit':     "none",
                 'dqm.strip.axis':     "run",
                 'dqm.strip.n':        "",
                 'dqm.reference':      {'show': "customise", 'position': "overlay", 'param':
					[{'type': "refobj", 'run': "", 'dataset': ""},
					 {'type': "none",   'run': "", 'dataset': ""},
					 {'type': "none",   'run': "", 'dataset': ""},
					 {'type': "none",   'run': "", 'dataset': ""}]},
                 'dqm.submenu':	       "data",
                 'dqm.size':           "M",
                 'dqm.root':           {},
                 'dqm.focus':          {},
                 'dqm.drawopts':       {},
                 'dqm.myobjs':         {} }

  def __init__(self, gui, rank, category, name):
    #gui._addCSSFragment("%s/fonts/fonts.css" % gui._yui)
    gui._addCSSFragment("%s/resources/css/ext-all.css" % gui._extjs)
    gui._addCSSFragment("%s/container/assets/skins/sam/container.css" % gui._yui)
    gui._addCSSFragment("%s/resize/assets/skins/sam/resize.css" % gui._yui)
    gui._addJSFragment("%s/adapter/ext/ext-base.js" % gui._extjs, False)
    gui._addJSFragment("%s/ext-all.js" % gui._extjs, False)
    gui._addJSFragment("%s/utilities/utilities.js" % gui._yui, False)
    gui._addJSFragment("%s/container/container-min.js" % gui._yui, False)
    gui._addJSFragment("%s/resize/resize-min.js" % gui._yui, False)

    gui.css = [x for x in gui.css if x[0] != '%s/css/Core/style.css' % gui.contentpath]
    gui._addCSSFragment("%s/css/DQM/style.css" % gui.contentpath)
    for p in ("Canvas", "Header", "Quality", "Sample", "Summary", "Play"):
      gui._addJSFragment("%s/javascript/DQM/%s.js" % (gui.contentpath, p))

    self.gui       = gui
    self.rank      = rank
    self.category  = category
    self.name      = name
    self.warnings  = {}

  # Customise server on start-up. Force hidden internal workspaces.
  def customise(self):
    if "Sample" not in (w.name for w in self.gui.workspaces):
      self.gui.workspaces.append(DQMSampleWorkspace(self.gui))

    if "Play" not in (w.name for w in self.gui.workspaces):
      self.gui.workspaces.append(DQMPlayWorkspace(self.gui))

  # Initialise a new session.
  def initialiseSession(self, session):
    for (var, value) in self.sessiondef.iteritems():
      if var not in session:
        session[var] = deepcopy(value)

  # Make sure the accessed object name is valid.
  def _checkObjectName(self, o):
    if not re.match(r"^[-+=_()/# A-Za-z0-9]*$", o):
      if o not in self.warnings:
        _logwarn("attempt to access unsafe object or version '%s'" % o)
	self.warnings[o] = 1
      raise HTTPError(500, "Attempt to access unsafe object")
    return o

  # Set session parameters, checking them for validity.
  def _set(self, session,
           samplevary = None,
           sampleorder = None,
           sampleprevws = None,
           samplepat = None,
           sampledynsearch = None,
           sampletype = None,
           dataset = None,
           runnr = None,
	   qplot = None,
           filter = None,
           referencepos  = None,
           referenceshow = None,
           referenceobj1 = None,
           referenceobj2 = None,
           referenceobj3 = None,
           referenceobj4 = None,
           striptype = None,
           stripruns = None,
           stripaxis = None,
           stripomit = None,
           search = None,
           submenu = None,
           size = None,
           playprevws = None,
           playinterval = None,
           playpos = None,
           root = None,
           focus = None,
           reset = None,
           add = None,
           zoom = None,
           **kwargs):

    if samplevary != None:
      if not isinstance(samplevary, str) \
         or samplevary not in ("any", "run", "dataset"):
        raise HTTPError(500, "Incorrect sample vary parameter")
      session['dqm.sample.vary'] = samplevary

    if sampleorder != None:
      if not isinstance(sampleorder, str) \
         or sampleorder not in ("run", "dataset"):
        raise HTTPError(500, "Incorrect sample order parameter")
      session['dqm.sample.order'] = sampleorder

    if sampleprevws != None:
      if not isinstance(sampleprevws, str) \
         or sampleprevws not in (x.name for x in self.gui.workspaces):
        raise HTTPError(500, "Incorrect sample previous workspace parameter")
      session['dqm.sample.prevws'] = sampleprevws

    if samplepat != None:
      if not isinstance(samplepat, str):
        raise HTTPError(500, "Incorrect sample pattern parameter")
      session['dqm.sample.pattern'] = samplepat

    if sampledynsearch != None:
      if not isinstance(sampledynsearch, str) \
             or sampledynsearch not in ("yes", "no"):
        raise HTTPError(500, "Incorrect sample dynamic search parameter")
      session['dqm.sample.dynsearch'] = sampledynsearch

    if sampletype != None:
      if not isinstance(sampletype, str):
        raise HTTPError(500, "Incorrect sample type parameter")
      elif hasattr(Accelerator.sampletype, sampletype):
        session['dqm.sample.type'] = int(getattr(Accelerator.sampletype, sampletype))
      else:
        raise HTTPError(500, "Incorrect sample type parameter")

    if dataset != None:
      if not isinstance(dataset, str):
        raise HTTPError(500, "Incorrect dataset parameter")
      session['dqm.sample.dataset'] = dataset

    if runnr != None:
      if not isinstance(runnr, str):
        raise HTTPError(500, "Incorrect run parameter")
      elif not re.match(r"^\d+$", runnr):
        raise HTTPError(500, "Incorrect run parameter")
      else:
        session['dqm.sample.runnr'] = int(runnr)

    if qplot != None:
      if not isinstance(qplot, str):
        raise HTTPError(500, "Incorrect quality plot parameter")
      if not re.match(r"^[A-Za-z.\d]*$", qplot):
        raise HTTPError(500, "Invalid quality plot parameter")
      session['dqm.qplot'] = qplot

    if filter != None:
      if not isinstance(filter, str) \
         or filter not in ("all", "alarms", "nonalarms"):
        raise HTTPError(500, "Incorrect filter parameter")
      session['dqm.filter'] = filter

    if referenceshow != None:
      if not isinstance(referenceshow, str) \
         or referenceshow not in ("all", "none", "customise"):
        raise HTTPError(500, "Incorrect referenceshow parameter")
      session['dqm.reference']['show'] = referenceshow

    if referencepos != None:
      if not isinstance(referencepos, str) \
         or referencepos not in ("overlay", "on-side"):
        raise HTTPError(500, "Incorrect referencepos parameter")
      session['dqm.reference']['position'] = referencepos

    for refidx, refobj in ((0, referenceobj1),
			   (1, referenceobj2),
			   (2, referenceobj3),
			   (3, referenceobj4)):
      param = session['dqm.reference']['param'][refidx]
      if refobj != None:
        if not isinstance(refobj, str):
          raise HTTPError(500, "Incorrect referenceobj parameter")
        m = re.match(r"^other:(\d*):([-/A-Za-z0-9_]*)$", refobj)
        if refobj == "refobj" or refobj == "none":
          param['type'] = refobj
          param['run'] = ""
          param['dataset'] = ""
        elif m:
          param['type'] = "other"
          param['run'] = m.group(1)
          param['dataset'] = m.group(2)
	else:
          raise HTTPError(500, "Incorrect referenceobj parameter")

    if striptype != None:
      if not isinstance(striptype, str) \
         or striptype not in \
           ("object", "num-entries", "num-bins", "num-bytes",
            "x-mean", "x-min", "x-max", "x-mean-rms", "x-mean-min-max", "x-bins",
            "y-mean", "y-min", "y-max", "y-mean-rms", "y-mean-min-max", "y-bins",
            "z-mean", "z-min", "z-max", "z-mean-rms", "z-mean-min-max", "z-bins"):
        raise HTTPError(500, "Incorrect strip type parameter")
      session['dqm.strip.type'] = striptype

    if stripruns != None:
      if not isinstance(stripruns, str):
        raise HTTPError(500, "Incorrect strip runs parameter")
      elif not re.match(r"^\d*$", stripruns):
        raise HTTPError(500, "Incorrect strip runs parameter")
      else:
        session['dqm.strip.n'] = stripruns

    if stripomit != None:
      if not isinstance(stripomit, str) or stripomit not in ("empty", "none"):
        raise HTTPError(500, "Incorrect strip omit parameter")
      session['dqm.strip.omit'] = stripomit

    if stripaxis != None:
      if not isinstance(stripaxis, str) or stripaxis not in ("run", "time"):
        raise HTTPError(500, "Incorrect strip axis parameter")
      session['dqm.strip.axis'] = stripaxis

    if search != None:
      if not isinstance(search, str):
        raise HTTPError(500, "Incorrect search parameter")
      session['dqm.search'] = search

    if submenu != None:
      if not isinstance(submenu, str) \
         or submenu not in ("data", "service", "workspace"):
        raise HTTPError(500, "Incorrect submenu parameter")
      session['dqm.submenu'] = submenu

    if size != None:
      if not isinstance(size, str) \
         or size not in ("XS", "S", "M", "L", "XL"):
        raise HTTPError(500, "Incorrect size parameter")
      session['dqm.size'] = size

    if playprevws != None:
      if not isinstance(playprevws, str) \
         or playprevws not in (x.name for x in self.gui.workspaces):
        raise HTTPError(500, "Incorrect play previous workspace parameter")
      session['dqm.play.prevws'] = playprevws

    if playinterval != None:
      if not isinstance(playinterval, str) \
         or not re.match(r"^\d+$", playinterval):
        raise HTTPError(500, "Incorrect play interval parameter")
      playinterval = int(playinterval)
      if playinterval == 0:
        playinterval = self.sessiondef['dqm.play.interval']
      elif playinterval < 2:
        playinterval = 2
      elif playinterval > 99:
        playinterval = 99
      session['dqm.play.interval'] = playinterval

    if playpos != None:
      if not isinstance(playpos, str) \
         or not re.match(r"^\d+$", playpos):
        raise HTTPError(500, "Incorrect play position parameter")
      session['dqm.play.pos'] = int(playpos)

    if root != None:
      if not isinstance(root, str):
        raise HTTPError(500, "Incorrect root parameter")
      session['dqm.root'][self.name] = self._checkObjectName(root)

    if focus != None:
      if not isinstance(focus, str):
        raise HTTPError(500, "Incorrect focus parameter")
      elif focus == "":
        if self.name in session['dqm.focus']:
          del session['dqm.focus'][self.name]
      else:
        session['dqm.focus'][self.name] = self._checkObjectName(focus)

    if reset == "yes":
      session['dqm.myobjs'][self.name] = []

    if add != None:
      session['dqm.myobjs'][self.name].extend \
        ([self._checkObjectName(x)
          for x in ((type(add) == str and [ add ]) or add)])

    if zoom != None:
      if not isinstance(zoom, str) or zoom not in ("yes", "no"):
        raise HTTPError(500, "Invalid Zoom show parameter")
      session['dqm.zoom.show'] = (zoom == "yes")


  # -----------------------------------------------------------------
  # Initialise a new session with the "start" URL.
  def start(self, session, **kwargs):
    self._set(session, **kwargs)

  # -----------------------------------------------------------------
  # Return DQM-specific index page.
  def sessionIndex(self, session, *args, **kwargs):
    return self.gui._templatePage("index-dqm", {
        'TITLE'      : self.gui.title,
        'SESSION_ID' : session['core.name'],
        'USER'       : session['core.user'],
        'HOSTNAME'   : socket.gethostname(),
        'ROOTPATH'   : self.gui.baseUrl
      })

  # -----------------------------------------------------------------
  # Switch session to another workspace.  Resets to data sub-menu
  # then re-uses GUI core workspace method.
  def sessionWorkspace(self, session, *args, **kwargs):
    self._set(session, submenu = "data")
    return self.gui.sessionWorkspace(session, *args, **kwargs)

  # Set quality plot selection.  For notification only.
  def sessionSetQualityPlot(self, session, *args, **kwargs):
    self._set(session, qplot = kwargs.get("n", ""))
    self.gui._saveSession(session)
    return "Ta."

  # Start choosing another sample.  This switches to a hidden internal
  # auto-generated workspace, remembering where the user was previously.
  def sessionChooseSample(self, session, *args, **kwargs):
    self._set(session,
              samplevary = kwargs.get("vary", "any"),
              sampleorder = kwargs.get("order", "dataset"),
              sampledynsearch = kwargs.get("dynsearch", "yes"),
              sampleprevws = session['core.workspace'])
    return self.gui.sessionWorkspace(session, name = "sample")

  # Set all/alarm/non-alarm filter.
  def sessionSetFilter(self, session, *args, **kwargs):
    self._set(session, filter = kwargs.get("n", "all"))
    self.gui._saveSession(session)
    return self._state(session)

  # Set global reference display override.
  def sessionSetReference(self, session, *args, **kwargs):
    self._set(session,
	      referencepos  = kwargs.get("position", None),
	      referenceshow = kwargs.get("show", None),
	      referenceobj1 = kwargs.get("r1", None),
	      referenceobj2 = kwargs.get("r2", None),
	      referenceobj3 = kwargs.get("r3", None),
	      referenceobj4 = kwargs.get("r4", None))
    self.gui._saveSession(session)
    return self._state(session)

  # Set global reference display override.
  def sessionSetStripChart(self, session, *args, **kwargs):
    self._set(session,
	      striptype = kwargs.get("type", None),
	      stripruns = kwargs.get("n", None),
              stripaxis = kwargs.get("axis", None),
              stripomit = kwargs.get("omit", None))
    self.gui._saveSession(session)
    return self._state(session)

  # Set sub-menu.  For notification only.
  def sessionSetSubMenu(self, session, *args, **kwargs):
    self._set(session, submenu = kwargs.get("n", "data"))
    self.gui._saveSession(session)
    return "Ta."

  # Set content search pattern.
  def sessionSetSearch(self, session, *args, **kwargs):
    self._set(session, search = kwargs.get("rx", ""))
    self.gui._saveSession(session)
    return self._state(session)

  # Set the size of canvas objects.
  def sessionSetSize(self, session, *args, **kwargs):
    self._set(session, size = kwargs.get("sz", None))
    self.gui._saveSession(session)
    return self._state(session)

  # Turn play mode on or off.
  def sessionPlay(self, session, *args, **kwargs):
    self._set(session, playprevws = session['core.workspace'])
    return self.gui.sessionWorkspace(session, name = "play")

  # Change the root shown in the selector menu.
  def sessionSetRoot(self, session, *args, **kwargs):
    self._set(session, root = kwargs.get("n", ""))
    if self.name in session['dqm.focus']:
      del session['dqm.focus'][self.name]
    self.gui._saveSession(session)
    return self._state(session)

  # Change the selected object.
  def sessionSetFocus(self, session, *args, **kwargs):
    name = kwargs.get("n", None)
    if name == None:
      if self.name in session['dqm.focus']:
        del session['dqm.focus'][self.name]
    else:
      self._set(session, focus = name)
    self.gui._saveSession(session)
    return self._state(session)

  # Change panel display parameters.
  def sessionPanel(self, session, *args, **kwargs):
    name = kwargs.get('n')
    show = kwargs.get('show', None)
    x = kwargs.get('x', None)
    y = kwargs.get('y', None)

    if not isinstance(name, str) or name not in ("help", "custom", "tools"):
      raise HTTPError(500, "Panel operation without a valid panel name")

    if show != None:
      if not isinstance(show, str) or show not in ("yes", "no"):
        raise HTTPError(500, "Invalid panel show parameter")
      session['dqm.panel.%s.show' % name] = show == "yes"

    if x != None:
      if not isinstance(x, str) or not re.match(r"^\d+$", x):
        raise HTTPError(500, "Invalid panel position parameter")
      session['dqm.panel.%s.x' % name] = int(x)

    if y != None:
      if not isinstance(y, str) or not re.match(r"^\d+$", y):
        raise HTTPError(500, "Invalid panel position parameter")
      session['dqm.panel.%s.y' % name] = int(y)

    self.gui._saveSession(session)
    return self._state(session)

  # Change Zoom window parameters.
  def sessionSetZoom(self, session, *args, **kwargs):
    show = kwargs.get('show', None)
    x = kwargs.get('x', None)
    y = kwargs.get('y', None)
    w = kwargs.get('w', None)
    h = kwargs.get('h', None)

    if show != None:
      if not isinstance(show, str) or show not in ("yes", "no"):
        raise HTTPError(500, "Invalid Zoom show parameter")
      session['dqm.zoom.show'] = (show == "yes")

    if x != None:
      if not isinstance(x, str) or not re.match(r"^\d+$", x):
        raise HTTPError(500, "Invalid Zoom position parameter")
      session['dqm.zoom.x'] = int(x)

    if y != None:
      if not isinstance(y, str) or not re.match(r"^\d+$", y):
        raise HTTPError(500, "Invalid Zoom position parameter")
      session['dqm.zoom.y'] = int(y)

    if w != None:
      if not isinstance(w, str) or not re.match(r"^\d+$", w):
        raise HTTPError(500, "Invalid Zoom width parameter")
      session['dqm.zoom.w'] = int(w)

    if h != None:
      if not isinstance(h, str) or not re.match(r"^\d+$", h):
        raise HTTPError(500, "Invalid Zoom height parameter")
      session['dqm.zoom.h'] = int(h)

    self.gui._saveSession(session)
#    return "Ma."
    return self._state(session)

# --------------------------------------------------------------------
class DQMSummaryWorkspace(Accelerator.DQMSummaryWorkspace, DQMWorkspace):
  def __init__(self, gui, rank, category, name):
    Accelerator.DQMSummaryWorkspace.__init__(self, gui, name)
    DQMWorkspace.__init__(self, gui, rank, category, name)

  # Return JSON object for the current session state.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

# --------------------------------------------------------------------
class DQMQualityWorkspace(Accelerator.DQMQualityWorkspace, DQMWorkspace):
  def __init__(self, gui, rank, category, name):
    Accelerator.DQMQualityWorkspace.__init__(self, gui, name)
    DQMWorkspace.__init__(self, gui, rank, category, name)

  # Return JSON object for the current session state.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

# --------------------------------------------------------------------
class DQMContentWorkspace(Accelerator.DQMContentWorkspace, DQMWorkspace):
  def __init__(self, gui, rank, category, name, match, *content):
    Accelerator.DQMContentWorkspace.__init__(self, gui, name, match)
    DQMWorkspace.__init__(self, gui, rank, category, name)
    self.content = content

  # Initialise a new session.
  def initialiseSession(self, session):
    DQMWorkspace.initialiseSession(self, session)
    if self.name not in session['dqm.myobjs'] and len(self.content):
      session['dqm.myobjs'][self.name] = deepcopy(self.content)
      session['dqm.root'][self.name] = "Quick collection"

  # Return JSON object for the current session state.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

  # -----------------------------------------------------------------
  # Reset the canvas back to the workspace default.
  def sessionResetWorkspace(self, session, *args, **kwargs):
    session['dqm.play.prevws'] = ""
    session['dqm.play.pos'] = 0
    session['dqm.size'] = self.sessiondef['dqm.size']
    for key in ('dqm.focus', 'dqm.drawopts', 'dqm.myobjs'):
      if self.name in session[key]:
        del session[key][self.name]
    self.initialiseSession(session)
    self.gui._saveSession(session)
    return self._state(session)

  # -----------------------------------------------------------------
  # Change image parameters.
  def sessionAlterImage(self, session, *args, **kwargs):
    obj = kwargs['o']
    opt = kwargs['opt']
    val = kwargs['value']
    if not isinstance(obj, str):
      raise HTTPError(500, "Image operation without an image")
    if opt == "withref":
      if val == "":
        val == "def"
      if val not in ("def", "yes", "no"):
        raise HTTPError(500, "Unrecognised reference setting")
    elif opt == "drawopts":
      if not re.match(r"^[A-Za-z,]*$", val):
        raise HTTPError(500, "Unknown draw options")
    elif opt in ("xtype", "ytype", "ztype"):
      if val not in ("def", "lin", "log"):
        raise HTTPError(500, "Unrecognised axis type")
    elif opt in ("xmin", "xmax", "ymin", "ymax", "zmin", "zmax"):
      if not re.match(r"^([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)?$", val):
        raise HTTPError(500, "Unrecognised axis limit")
    else:
      raise HTTPError(500, "Attempt to set unknown image option")

    opts = session['dqm.drawopts'].setdefault(self.name, {}).setdefault(obj, {})
    if val == '':
      if opt in opts:
        del opts[opt]
      if len(opts) == 0:
        del session['dqm.drawopts'][self.name][obj]
      if len(session['dqm.drawopts'][self.name]) == 0:
        del session['dqm.drawopts'][self.name]
    else:
      opts[opt] = val

    self.gui._saveSession(session)
    return self._state(session)

# --------------------------------------------------------------------
class DQMShiftWorkspace(DQMContentWorkspace):
  SHIFT_ROOT = "00 Shift"

  def __init__(self, gui, rank, category, name):
    DQMContentWorkspace.__init__(self, gui, rank, category, name, "^")

  # Initialise a new session.
  def initialiseSession(self, session):
    DQMContentWorkspace.initialiseSession(self, session)
    session['dqm.root'][self.name] = self.SHIFT_ROOT

  # Override session state to fix root.
  def sessionState(self, session, *args, **kwargs):
    if not session['dqm.root'].get(self.name, "").startswith(self.SHIFT_ROOT):
      session['dqm.root'][self.name] = self.SHIFT_ROOT
      self.gui._saveSession(session)
    return DQMContentWorkspace.sessionState(self, session)

  # Change the root shown in the selector menu.
  def sessionSetRoot(self, session, *args, **kwargs):
    if kwargs.get("n", "").startswith(self.SHIFT_ROOT):
      return DQMContentWorkspace.sessionSetRoot(self, session, *args, **kwargs)
    else:
      return self._state(session)

# --------------------------------------------------------------------
class DQMSampleWorkspace(Accelerator.DQMSampleWorkspace, DQMWorkspace):
  def __init__(self, gui, rank = -1, category = "Internal", name = "Sample"):
    Accelerator.DQMSampleWorkspace.__init__(self, gui, name)
    DQMWorkspace.__init__(self, gui, rank, category, name)

  # Return JSON object for the current session state.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

  # Return back from sample selection without making changes.
  def sessionReturn(self, session, *args, **kwargs):
    prevws = session['dqm.sample.prevws']
    session['dqm.sample.prevws'] = ""
    return self.gui.sessionWorkspace(session, name = prevws)

  # Set the search pattern, order or parameter to vary.
  def sessionModify(self, session, *args, **kwargs):
    self._set(session,
	      samplevary = kwargs.get("vary", None),
	      sampleorder = kwargs.get("order", None),
	      samplepat = kwargs.get("pat", None),
	      sampledynsearch = kwargs.get("dynsearch", None));
    self.gui._saveSession(session)
    return self._state(session)

  # Select another sample.
  def sessionSelect(self, session, *args, **kwargs):
    self._set(session,
              sampletype = kwargs.get("type"),
              dataset = kwargs.get("dataset"),
              runnr = kwargs.get("runnr"));
    return self.sessionReturn(session)

# --------------------------------------------------------------------
class DQMPlayWorkspace(Accelerator.DQMPlayWorkspace, DQMWorkspace):
  def __init__(self, gui, rank = -1, category = "Internal", name = "Play"):
    Accelerator.DQMPlayWorkspace.__init__(self, gui, name)
    DQMWorkspace.__init__(self, gui, rank, category, name)

  # Return JSON object for the current session state.
  def sessionState(self, session, *args, **kwargs):
    return self._state(session)

  # Return back from sample selection without making changes.
  def sessionStop(self, session, *args, **kwargs):
    prevws = session['dqm.play.prevws']
    session['dqm.play.prevws'] = ""
    return self.gui.sessionWorkspace(session, name = prevws)

  # Set the search pattern.
  def sessionInterval(self, session, *args, **kwargs):
    self._set(session, playinterval = kwargs.get("v", self.sessiondef['dqm.play.interval']))
    self.gui._saveSession(session)
    return self._state(session)

  # Select another sample.
  def sessionStep(self, session, *args, **kwargs):
    self._set(session, playpos = kwargs.get("pos", 0))
    if kwargs.get("move", "no") == "yes":
      return self._state(session)
    else:
      return "Ta."
