import sys, os, os.path, hashlib
from socket import gethostname
from glob import glob
DEFAULT_PORT = 8030
DEFAULT_HOST = gethostname()

class ServerDef:
  """FIXME: Server configuration."""
  def __init__(self):
    self.instrument  = None
    self.port        = None
    self.localBase   = None
    self.serverDir   = None
    self.logFile     = None
    self.baseUrl     = None
    self.title       = None
    self.plugins     = []
    self.services    = []
    self.serviceName = None
    self.workspaces  = []
    self.sources     = []
    self.extensions  = []

  def service(self, name, url):
    self.services.append((name, url))

  def workspace(self, type, *args):
    self.workspaces.append((type, args))

  def source(self, type, *args, **kwargs):
    self.sources.append((type, kwargs.get('label', type.lower()), args))

  def extend(self, type, *args):
    self.extensions.append((type, args))

  def plugin(self, name, *patterns):
    self.plugins.append(self._makePlugin(name, *patterns))

  def pathOfPlugin(self, name):
    for p in self.plugins:
      if p['name'] == name:
	return p['binary']
    raise "No such plug-in '%s' registered" % name

  def validate(self):
    if self.port == None:
      self.port = DEFAULT_PORT
    if self.localBase == None:
      self.localBase = '%s:%d' % (DEFAULT_HOST, self.port)
    elif self.localBase.find(':') == -1:
      self.localBase = '%s:%d' % (self.localBase, self.port)
    if self.title == None:
      self.title = "CMS monitoring"
    if self.logFile == None:
       self.logFile = "%s/log-%%Y%%m%%d.log" % self.serverDir

    if self.serverDir == None:
      return "server.serverDir not set"
    if self.baseUrl == None:
      return "server.baseUrl not set"
    if len(self.services) == 0:
      return "no services added with server.service()"
    if self.serviceName == None:
      return "server.serviceName not set"
    if len(self.workspaces) == 0:
      return "no workspaces added with server.workspace()"
    if len(self.sources) == 0:
      return "no sources added with server.source()"
    return None

  def _makePlugin(self, name, *patterns):
    # Create initial plug-in object place-holder.
    p = { 'name': name, 'dir': None, 'data': None, 'id': None,
	  'binary': None, 'regenerate': True, 'sources': [], 'old': [] }

    # Create a sorted list of file names matching the given patterns.
    for pat in patterns:
      l = glob(pat)
      if len(l) == 0:
	print >> sys.stderr, "%s:%s: warning: glob pattern did not match any files" % (name, pat)
      p['sources'].extend([dict(path=f) for f in l])
    p['sources'].sort()

    # Gather source finger print information.
    for f in p['sources']:
      path = f['path']
      if not os.path.exists(path) or not os.access(path, os.R_OK):
	print >> sys.stderr, "%s:%s: warning: file not found or not readable" % (name, path)
	continue
      s = os.stat(path)
      f['mtime'] = s.st_mtime
      f['size'] = s.st_size
      f['cksum'] = hashlib.md5(file(path, "r").read()).hexdigest()

    # Create plug-in finger print and a path to the binary to load.
    p['data'] = "\n".join(["%s:%s:%s:%s" % (f['path'], f['mtime'], f['size'], f['cksum']) for f in p['sources']])
    p['id'] = hashlib.md5(p['data']).hexdigest()
    p['dir'] = '%s/%s' % (self.serverDir, p['name'])
    p['binary'] = '%s/%s.ext' % (p['dir'], p['id'])

    # Check for existing plug-ins. If we find one with a different finger print,
    # mark it as old for removal at server start-up. If we find one with the right
    # finger print, remember that the plug-in doesn't need to be regenerated now.
    for cand in glob(p['dir'] + "/*.ext*"):
      if cand == p['binary']:
	p['regenerate'] = False
      else:
	p['old'].append(cand)

    # Return this plug-in.
    return p
