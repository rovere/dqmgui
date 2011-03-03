from Monitoring.Overview.GUI import ImageScraperSource, CompWorkspace, lemonimg, lrfimg, slsimg
from threading import Thread, Lock, Condition
from cherrypy import engine
import time, urllib2, json, re

class CAFDataProvider(Thread):
  DATAURL = "http://cmsweb.cern.ch/phedex/datasvc/json/prod/blockreplicas?node=T2_CH_CAF"
  DISKURL = "http://sls.cern.ch/sls/service.php?id=CASTORCMS_CMSCAF"
  def __init__(self, *rxlist):
    Thread.__init__(self, name="CAFDataProvider")
    self.rxlist = [(label, re.compile(rx, re.I)) for label, rx in rxlist]
    self.rxtotal = re.compile(r"Free/Total Space ([0-9.]+)/([0-9.]+) \([0-9.]+ %\)")
    self.stopme = False
    self.lock = Lock()
    self.wakeup = Condition(self.lock)
    self.data = {}
    self.total = 0

  # Tell thread to stop.
  def stop(self):
    self.lock.acquire()
    self.stopme = True
    self.wakeup.notifyAll()
    self.lock.release()

  # Retrieve currently cached values.
  def values(self):
    self.lock.acquire()
    (data, total) = (self.data, self.total)
    self.lock.release()
    return (data, total)

  # Refresh the current set of values.
  def refresh(self):
      # Let lock go first.
      newdata = None
      newtotal = None
      self.lock.release()

      # Fetch new data.  Ignore failures
      try: newdata = json.loads(urllib2.urlopen(self.DATAURL).read())
      except: pass

      try: newtotal = urllib2.urlopen(self.DISKURL).readlines()
      except: pass

      # Reacquire lock to re-evaluate information.
      self.lock.acquire()

      # Parse PhEDEx json data. Find datasets from blocknames, and
      # accumulate statistics on total size and creation time.
      # Assign each dataset to the first label in rxlist which
      # matches the dataset name.
      if newdata:
        self.data = {}
        for block in newdata['phedex']['block']:
	  dsname = block['name'].split("#")[0]
	  size = float(block['replica'][0]['bytes'])
          ctime = float(block['replica'][0]['time_create'])
	  label = "other"
	  for l, rx in self.rxlist:
	    if re.search(rx, dsname):
	      label = l
	      break

	  if label not in self.data:
	    self.data[label] = {}

	  if dsname not in self.data[label]:
	    self.data[label][dsname] = { 'size': size, 'ctime': ctime }
	  else:
	    self.data[label][dsname]['size'] += size
            if ctime < self.data[label][dsname]['ctime']:
              self.data[label][dsname]['ctime'] = ctime

      # Parse the SLS disk space information. Locate the total
      # space information field and extract total CAF disk size
      # from it.  Note that SLS reports volume in metric, while
      # we use powers of two for the rest.  Only 95% of the
      # reported space is actually valid for use.
      if newtotal:
	for line in newtotal:
	  m = re.search(self.rxtotal, line)
	  if m:
	    self.total = 0.95 * float(m.group(2))
            self.total *= 1000.**4 / 1024**4


  # Run forever retrieving new CAF usage statistics every 30 minutes.
  # Exit early on server shutdown.
  def run(self):
    self.lock.acquire()
    while not self.stopme:
      self.refresh()
      self.wakeup.wait(1800)
    self.lock.release()

# Source to print CAF status text files.
class CAFDiskSource:
  plothook = 'caf-disk'
  scales = (('TB', 1024**4), ('GB', 1024**3), ('MB', 1024**2), ('kB', 1024), ('B', 0))
  def __init__(self, server, statedir, *rxlist):
    self.thread = CAFDataProvider(*rxlist)
    engine.subscribe('exit', self.thread.stop, priority=100)
    self.thread.start()

  # Utility method to format dataset line.  Converts size to human
  # scale number, epoch time to a YYYY-MM-DD date, and shows the
  # lot as a reasonably nicely tabulated data.
  def format(self, name, size, ctime):
    for scale in self.scales:
      if size >= scale[1]:
        if scale[1]:
          hsize = "%.2f %s" % (size/scale[1], scale[0])
        else:
	  hsize = "%.2f %s" % (size, scale[0])
	return "%-14s %-14s %s" % \
	  (hsize, time.strftime("%Y-%m-%d", time.gmtime(ctime)), name)
    assert(False)

  # Utility method to get dataset list sorted by size or time for a
  # given regexp label.  Note that basically the code below knows the
  # labels the configuration will give, but not the actual regexps.
  # Oh well. Returns content type and formatted content (text/plain).
  def getdata(self, label, key):
    (data, total) = self.thread.values()
    data = [(ds, data[label][ds]) for ds in data.get(label, {})]
    data.sort(lambda a, b: cmp(b[1][key], a[1][key]))
    return 'text/plain', "\n".join(self.format(ds[0], **ds[1]) for ds in data)

  # Utility method to get fraction of disk space for a given regexp
  # label.  Note that basically the code below knows the labels the
  # configuration will give, but not the actual regexps.  Oh well.
  # Returns content type and formatted content (text/plain).
  def getfrac(self, label):
    (data, total) = self.thread.values()
    size = sum(data[label][ds]['size'] for ds in data.get(label, {}))
    size /= self.scales[0][1]
    return 'text/plain', "%s %% (%.2f TB)" % \
	((total and ("%.2f" % (100. * size / total))) or "?.??", size)

  # Plot routine. Just a simple hook to show current data values.
  def plot(self, group, *args, **kwargs):
    if group == 'alcabydate': return self.getdata('alca', 'ctime')
    elif group == 'alcabysize': return self.getdata('alca', 'size')
    elif group == 'commbydate': return self.getdata('comm', 'ctime')
    elif group == 'commbysize': return self.getdata('comm', 'size')
    elif group == 'physbydate': return self.getdata('phys', 'ctime')
    elif group == 'physbysize': return self.getdata('phys', 'size')
    elif group == 'otherbydate': return self.getdata('other', 'ctime')
    elif group == 'otherbysize': return self.getdata('other', 'size')
    elif group == 'alcapercent': return self.getfrac('alca')
    elif group == 'commpercent': return self.getfrac('comm')
    elif group == 'physpercent': return self.getfrac('phys')
    elif group == 'otherpercent': return self.getfrac('other')
    else: return None, None

class CAFPlotSource(ImageScraperSource):
  def __init__(self, server, dir, *args):
    ImageScraperSource.__init__(self, 'caf-plot', [
      lemonimg("web", "info", ["lxbatch-cmscaf-cpu", "lxbatch-cmscaf-net"], "/lemon-web/images/", entity="lxbatch/cmscaf", time=0, offset=0),
      lemonimg("web", "info", ["lxbatch-cmsexpress-cpu", "lxbatch-cmsexpress-net"], "/lemon-web/images/", entity="lxbatch/cmsexpress", time=0, offset=0),
      lemonimg("web", "info", ["lxbatch-cmsinter-cpu", "lxbatch-cmsinter-net"], "/lemon-web/images/", entity="lxbatch/cmsinter", time=0, offset=0),
      lemonimg("web", "metric_distribution", ["lxbatch-cmscaf-loadavg"], "cache/", entity="lxbatch/cmscaf", metric="LoadAvg", field="LoadAvg"),
      lemonimg("web", "metric_distribution", ["lxbatch-cmsexpress-loadavg"], "cache/", entity="lxbatch/cmsexpress", metric="LoadAvg", field="LoadAvg"),
      lemonimg("web", "metric_distribution", ["lxbatch-cmsinter-loadavg"], "cache/", entity="lxbatch/cmsinter", metric="LoadAvg", field="LoadAvg"),
      lemonimg("status", "info", ["c2cms-cmscaf-cpu", "c2cms-cmscaf-net"], "/lemon-status/images/", entity="c2cms/cmscaf", time="0.0.5", offset=0),
      lemonimg("status", "info", ["c2cms-cmscafuser-cpu", "c2cms-cmscafuser-net"], "/lemon-status/images/", entity="c2cms/cmscafuser", time="0.0.5", offset=0),
      lemonimg("status", "metric_distribution", ["c2cms-cmscaf-loadavg"], "cache/", entity="c2cms/cmscaf", metric="LoadAvg", field="LoadAvg"),
      lemonimg("status", "metric_distribution", ["c2cms-cmscafuser-loadavg"], "cache/", entity="c2cms/cmscafuser", metric="LoadAvg", field="LoadAvg"),
      slsimg(["avail"], "graph.php", id="CASTORCMS_CMSCAF", more="availability", period="day"),
      slsimg(["disk-free"], "graph.php", id="CASTORCMS_CMSCAF", more="nv:Percentage Free Space", period="week"),
      slsimg(["diskuser-free"], "graph.php", id="CASTORCMS_CMSCAFUSER", more="nv:Percentage Free Space", period="week"),
      lrfimg(["job-queue-caf1nh"], "rrd_RUN_d.gif", queue="cmscaf1nh"),
      lrfimg(["job-queue-caf1nd"], "rrd_RUN_d.gif", queue="cmscaf1nd"),
      lrfimg(["job-queue-caf1nw"], "rrd_RUN_d.gif", queue="cmscaf1nw"),
      lrfimg(["job-queue-cmsexpress"], "rrd_RUN_d.gif", queue="cmsexpress")])

class CompCAFWorkspace(CompWorkspace):
  class StorageSummary:
    name = "Storage pools performance"
    def state(self, session):
      return "{kind: 'CompCAFCastor', version: %d}" % time.time()

  class BatchSummary:
    name = "LSF performance"
    def state(self, session):
      return "{kind: 'CompCAFBatch', version: %d}" % time.time()

  class JobSummary:
    name = "Job summary"
    def state(self, session):
      return "{kind: 'CompCAFJobs', version: %d}" % time.time()

  class DiskMonitor:
    name = "Disk monitoring"
    def state(self, session):
      return "{kind: 'CompCAFDisk', version: %d}" % time.time()

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'caf', name,
			   [self.StorageSummary(), self.BatchSummary(),
			    self.JobSummary(), self.DiskMonitor()])
    gui._addJSFragment("%s/javascript/Overview/CAF.js" % gui.contentpath)

  def _state(self, session):
    return self._dostate(session, "CAF_workflows_monitoring")
