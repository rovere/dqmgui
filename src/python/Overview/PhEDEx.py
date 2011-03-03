import re, time
from Monitoring.Core.Plot import *
from Monitoring.Core.Utils import _logwarn, timeseries
from Monitoring.Overview.GUI import CompWorkspace
from cherrypy import HTTPError
from threading import Lock
from calendar import timegm
from time import gmtime

# --------------------------------------------------------------------
class Stuff: pass
def time2gm(year = 1970, month = 1, day = 1, hour = 0, minute = 0, second = 0):
  return timegm((year, month, day, hour, minute, second, 0, 0, 0))

# --------------------------------------------------------------------
def groupByNode(name):
  return name

def groupBySite(name):
  return re.sub(r"_(Buffer|Disk|Export|MSS)$", "", name)

def groupByCountry(name):
  return re.sub(r"^[^_]+_([^_]+)_.*", r"\1", name)

def groupByTier(name):
  return re.sub(r"^([^_]+)_.*", r"\1", name)

def groupByRegion(name):
  if len(name) < 6 \
     or name[0] != 'T' \
     or name[1] not in ('0', '1', '2', '3') \
     or name[2] != '_' \
     or name[5] != '_':
    return "Other"
  elif name[0:2] == 'T0': return "T0"
  elif name == 'T2_CH_CAF': return "CAF"
  elif name[3:5] in ('TW', 'KR'): return "ASGC"
  elif name[3:5] in ('RU', 'IN', 'GR') or name[3:10] == 'CH_CERN': return "CERN"
  elif name[3:5] in ('IT', 'AT', 'HU'): return "CNAF"
  elif name[3:5] in ('US', 'BR'): return "FNAL"
  elif name[3:5] in ('DE', 'PL') or name == 'T2_CH_CSCS': return "FZK"
  elif name[3:5] in ('FR', 'BE', 'CN'): return "IN2P3"
  elif name[3:5] in ('ES', 'PT'): return "PIC"
  elif name[3:5] in ('UK', 'EE', 'FI'): return "RAL"
  else: return "Other"

GROUP_BY = { 'node': groupByNode,
	     'site': groupBySite,
	     'country': groupByCountry,
	     'region': groupByRegion,
	     'tier': groupByTier }

# --------------------------------------------------------------------
# PhEDEx data transfer source.
class PhEDExSource:
  plothook = "phedex"
  def __init__(self, server, statedir, dbcfg, *args):
    self.lock = Lock()
    self.imgcache = {}
    self.dbparam = []
    self.db = {}
    execfile(dbcfg, globals(), { 'dbparam': self.dbparam })
    for i in self.dbparam:
      self.db[i['label']] = { 'db': None, 'def': i, 'cache': {} }
    self._databases(i for i in self.db)

  def _queryPruneCache(self, db):
    now = time.time()
    for (label, cache) in db['cache'].iteritems():
      for old in [q for q in cache if cache[q]['valid'] < now]:
	del cache[old]

  def _queryData(self, db, label, validfor, q, *args):
    cache = db['cache'].setdefault(label, {})
    key = ":".join(str(i) for i in args)
    now = time.time()
    if key not in cache or now > cache[key]['valid']:
      try:
	if not db['db']: db['db'] = db['def']['connect']()
	c = db['db'].cursor(); c.arraysize = 10000
	cache[key] = {'valid': now + validfor, 'data': q(c, *args) }
      except Exception, e:
	_logwarn("failed to execute query %s for '%s' in %s: %s; disconnecting"
		 % (label, key, db['def']['label'], str(e)))
	db['db'] = None
	raise
    return cache[key]['data']

  def _queryTimeSeries(self, db, label, validfor, q, *args):
    cache = db['cache'].setdefault(label, {})
    (bin, start, end) = args[0:3]
    found = None
    for key in cache:
      if key[0] == bin and key[1] <= start and key[2] >= end:
	if not found or found[1] < key[1] or found[2] > key[2]:
	  found = key

    now = time.time()
    key = (found or (bin, start, end))
    if key not in cache or now > cache[key]['valid']:
      try:
	if not db['db']: db['db'] = db['def']['connect']()
	c = db['db'].cursor(); c.arraysize = 10000
        cache[key] = { 'valid': now + validfor, 'data': q(c, *args) }
      except Exception, e:
	_logwarn("failed to execute query %s for '%s' in %s: %s; disconnecting"
		 % (label, key, db['def']['label'], str(e)))
	db['db'] = None
        raise
    return cache[key]['data']

  def _fetchLinkEvents(self, c, span, start, end):
    c.execute("""
      select
	trunc(timebin/:span)*:span, t.name, f.name,
        nvl(sum(h.done_bytes),0),
        nvl(sum(h.done_files),0),
        nvl(sum(h.fail_files),0),
	nvl(sum(h.try_files),0)
      from t_history_link_events h
        join t_adm_node f on f.id = h.from_node
	join t_adm_node t on t.id = h.to_node
      where timebin >= :starttime
        and timebin < :endtime
      group by trunc(timebin/:span)*:span, f.name, t.name
      """, span=span, starttime=start, endtime=end)
    return c.fetchall()

  def _fetchLinkStats(self, c, span, start, end):
    c.execute("""
      select
	trunc(timebin/:span)*:span, t.name, f.name,
	nvl(sum(pend_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(wait_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(cool_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(ready_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(xfer_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(confirm_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(param_rate) keep (dense_rank last order by timebin asc),0),
	nvl(sum(param_latency) keep (dense_rank last order by timebin asc),0)
      from t_history_link_events h
        join t_adm_node f on f.id = h.from_node
	join t_adm_node t on t.id = h.to_node
      where timebin >= :starttime
        and timebin < :endtime
      group by trunc(timebin/:span)*:span, f.name, t.name
      """, span=span, starttime=start, endtime=end)
    return c.fetchall()

  def _fetchDestStats(self, c, span, start, end):
    c.execute("""
      select
	trunc(timebin/:span)*:span, t.name, f.name,
	nvl(sum(dest_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(node_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(request_bytes) keep (dense_rank last order by timebin asc),0),
	nvl(sum(idle_bytes) keep (dense_rank last order by timebin asc),0)
      from t_history_dest h
        join t_adm_node f on f.id = h.from_node
	join t_adm_node t on t.id = h.to_node
      where timebin >= :starttime
        and timebin < :endtime
      group by trunc(timebin/:span)*:span, f.name, t.name
      """, span=span, starttime=start, endtime=end)
    return c.fetchall()

  def _fetchBlockLatency(self, c, start, end):
    c.execute("""
      select l.last_replica, n.name, l.bytes, l.latency
      from t_log_block_latency l
        join t_adm_node n on n.id = l.destination
      where l.last_replica >= :starttime
        and l.last_replica < :endtime
        and l.last_replica is not null
        and l.last_replica > l.first_replica
      """, starttime=start, endtime=end)
    return c.fetchall()


  def _fetchNodes(self, c):
    c.execute("select name from t_adm_node")
    return c.fetchall()

  def _queryTimeBounds(self, attrs):
    d = Stuff()

    # Determine query time span and granularity.
    d.span = attrs.get('span', 'hour')
    if d.span not in ('hour', 'day', 'week', 'month', 'year'):
      d.span = 'hour'

    d.end = (attrs.get('end', '').isdigit() and int(attrs['end'])) or time.time()
    range = timeseries(d.span, d.end, d.end)[0]
    if d.end != range[0]:
      d.end = range[1]

    if 'start' in attrs and attrs['start'].isdigit():
      d.start = int(attrs['start'])
      if d.span == 'hour' and d.start < d.end-336*3600:
	d.start = d.end - 336*3600
      elif d.span == 'day' and d.start < d.end-365*86400:
	d.start = d.end - 365*86400
      elif d.span == 'week' and d.start < d.end-312*7*86400:
	d.start = d.end - 312*7*86400
    elif d.span == 'hour':
      d.start = d.end - 96*3600
    elif d.span == 'day':
      d.start = d.end - 90*86400
    elif d.span == 'week':
      d.start = d.end - 52*7*86400
    else:
      d.start = 0

    d.start = max(d.start, time2gm(year=2004, month=6, day=1))
    d.start = timeseries(d.span, d.start, d.start)[0][0]
    d.querybin = ((d.span == 'hour' and 3600) or 86400)
    return d

  def _linkOptions(self, attrs):
    d = self._queryTimeBounds(attrs)

    # Determine other attributes.
    d.qty = attrs.get('qty', None)
    if d.qty not in ('source', 'destination', 'link'):
      d.qty = 'destination'

    d.links = attrs.get('links', None)
    if d.links not in ('mss', 'nonmss', 'all'):
      d.links = 'nonmss'

    d.grouping = attrs.get('grouping', None)
    if d.grouping not in ('db', 'node', 'site', 'country', 'region', 'tier'):
      d.grouping = 'node'

    d.srcgrouping = attrs.get('src-grouping', None)
    if d.srcgrouping not in ('same', 'db', 'node', 'site', 'country', 'region', 'tier'):
      d.srcgrouping = 'same'

    d.filterTitle = None
    d.srcfilter = attrs.get('from', None)
    if d.srcfilter:
      d.filterTitle = "Sources matching '%s'" % d.srcfilter
      d.srcfilter = re.compile(d.srcfilter)

    d.destfilter = attrs.get('to', None)
    if d.destfilter:
      d.filterTitle = "%sestinations matching '%s'" % \
	(((d.srcfilter and d.filterTitle + " and d") or "D"), d.destfilter)
      d.destfilter = re.compile(d.destfilter)

    if d.span == 'day':
      d.spanTitle = 'Daily'
    else:
      d.spanTitle = d.span[0].upper() + d.span[1:] + 'ly'

    d.dataTitle = "By "
    if d.qty == 'link':
      if d.srcgrouping == 'db' or d.grouping == 'db':
	d.dataTitle = 'database instance'
      elif d.srcgrouping == 'same' or d.srcgrouping == d.grouping:
        if d.grouping == 'node':    d.dataTitle += 'transfer link'
        elif d.grouping == 'site':    d.dataTitle += 'site links'
        elif d.grouping == 'country': d.dataTitle += 'country links'
        elif d.grouping == 'region':  d.dataTitle += 'region links'
        elif d.grouping == 'tier':    d.dataTitle += 'tier-level links'
      else:
        d.dataTitle = d.srcgrouping + ' to ' + d.grouping + ' links'
    else:
      if d.grouping == 'db':        d.dataTitle += 'database instance'
      elif d.grouping == 'node':    d.dataTitle += d.qty + ' storage node'
      elif d.grouping == 'site':    d.dataTitle += d.qty + ' site'
      elif d.grouping == 'country': d.dataTitle += d.qty + ' country'
      elif d.grouping == 'region':  d.dataTitle += d.qty + ' region'
      elif d.grouping == 'tier':    d.dataTitle += d.qty + ' tier level'

    if d.links == 'mss':
      d.dataTitle += " for tape storage only"
    elif d.links == 'nonmss':
      d.dataTitle += " for non-tape storage only"
    else:
      d.dataTitle += " for both tape and non-tape storage"

    # Determine data grouping, by source, destination or link.
    if d.srcgrouping == 'same':
      d.srcgrouping = d.grouping

    if d.qty == 'link':
      if d.srcgrouping == 'db' or d.grouping == 'db':
        def keyfunc(db, dest, src):
	  return (db, db)
      else:
        def keyfunc(db, dest, src):
	  dgroup = GROUP_BY[d.grouping](dest)
          sgroup = GROUP_BY[d.srcgrouping](src)
	  return ((dgroup, sgroup), "%s -> %s" % (sgroup, dgroup))
    elif d.grouping == 'db':
      def keyfunc(db, dest, src):
	return (db, db)
    elif d.qty == 'source':
      def keyfunc(db, dest, src):
	sgroup = GROUP_BY[d.grouping](src)
	return (sgroup, sgroup)
    elif d.qty == 'destination':
      def keyfunc(db, dest, src):
	dgroup = GROUP_BY[d.grouping](dest)
	return (dgroup, dgroup)

    d.keyfunc = keyfunc
    return d

  def _makeLegend(self, d):
    # Compute legend.  For links use data, otherwise fixed colouring.
    # Apply drawing style options based on node (or link) type.
    # FIXME: Cache legend definitions?
    colors = [ "#7D818C", "#007092", "#75B3CE", "#B40019", "#FFDC8C", "#F79025",
	       "#BBBEC5", "#231F20", "#e66266", "#fff8a9", "#7bea81", "#8d4dff",
	       "#ffbc71", "#a57e81", "#baceac", "#00ccff", "#ccffff", "#ff99cc",
	       "#cc99ff", "#ffcc99", "#3366ff", "#33cccc", "#ffffff", "#bbbbbb",
	       "#777777", "#333333" ]

    if d.qty == 'link':
      d.legend = [{ 'key': x, 'name': d.allkeys[x],
		    'attrs': { 'facecolor': colors[i % len(colors)] } }
	          for x, i in zip(sorted(d.allkeys.keys()),
				  xrange(0, len(d.allkeys)))]
    else:
      if d.grouping == 'db':
	keys = [ db['name'] for db in self.dbparam ]
      else:
        keys = sorted(dict((n, 1) for n in map(GROUP_BY[d.grouping], d.allnodes.keys())).keys())
      d.legend = [{ 'key': x, 'name': x, 'attrs': { 'facecolor': colors[i % len(colors)] } }
	          for x, i in zip(keys, xrange(0, len(keys)))]

    d.legend.append({ 'key': None, 'name': None, 'attrs': { 'facecolor': '#000000' }})

  def _linkData(self, dbs, attrs):
    d = self._linkOptions(attrs)

    # Define the binning.
    series = timeseries(d.span, d.start, d.end)
    data = [{} for t in series]
    bins = numpy.zeros(len(series)+2)
    for i in xrange(0, len(series)):
      bins[i+1] = series[i][0]
    bins[-1] = series[-1][1]

    # Group raw database data as per request.
    d.allkeys = {}
    d.allnodes = {}
    self.lock.acquire()
    try:
      for db in dbs:
        for name, in self._queryData(db, 'nodes', 300, self._fetchNodes):
	  d.allnodes.setdefault(name, {}).setdefault(db['def']['name'], 1)
	rows = self._queryTimeSeries(db, 'link-events', 300,
				     self._fetchLinkEvents,
				     d.querybin, d.start, d.end)
        if len(rows):
          binning = numpy.digitize([r[0] for r in rows], bins)
        for i in xrange(0, len(rows)):
          binidx = binning[i]
	  if binidx == 1 or binidx == len(data)+2:
	    continue

          row = rows[i]
          bin = row[0]
          dest = row[1]
          src = row[2]

          destmss = (dest.find("_MSS") >= 0)
          srcmss = (src.find("_MSS") >= 0)
          if d.links == 'mss':
	    if d.qty == 'destination' and not destmss: continue
	    if d.qty == 'source' and not srcmss: continue
	    if d.qty == 'link' and not (srcmss or destmss): continue
          elif d.links == 'nonmss' and (srcmss or destmss): continue
	  if d.srcfilter and re.search(d.srcfilter, src) < 0: continue
	  if d.destfilter and re.search(d.destfilter, dest) < 0: continue

	  (key, label) = d.keyfunc(db['def']['name'], dest, src)
	  d.allkeys[key] = label

	  bindata = data[binidx-2]
	  if key in bindata:
	    bindata[key] += row[3:]
          else:
	    bindata[key] = numpy.array([float(x) for x in row[3:]])
      for db in dbs:
        self._queryPruneCache(db)
	db['db'] = None
    finally:
      self.lock.release()

    self._makeLegend(d)
    return (d, series, data)

  def _blockLatencyData(self, dbs, attrs):
    d = self._linkOptions(attrs)

    # Define the binning.
    series = timeseries(d.span, d.start, d.end)
    data = [{} for t in series]
    bins = numpy.zeros(len(series)+2)
    for i in xrange(0, len(series)):
      bins[i+1] = series[i][0]
    bins[-1] = series[-1][1]

    # Group raw database data as per request.
    d.allkeys = {}
    d.allnodes = {}
    self.lock.acquire()
    try:
      for db in dbs:
	d.allnodes.update((name, 1) for (name,) in
			  self._queryData(db, 'nodes', 300, self._fetchNodes))
	rows = self._queryData(db, 'block-latency', 300,
			       self._fetchBlockLatency,
			       d.start, d.end)
        if len(rows):
          binning = numpy.digitize([r[0] for r in rows], bins)
        for i in xrange(0, len(rows)):
          binidx = binning[i]
	  if binidx == 1 or binidx == len(data)+2:
	    continue

          row = rows[i]
          tdone = row[0]
          dest = row[1]
          nbytes = row[2]
          latency = row[3]

          destmss = (dest.find("_MSS") >= 0)
          if d.links == 'mss' and not destmss: continue
          elif d.links == 'nonmss' and destmss: continue
	  if d.destfilter and re.search(d.destfilter, dest) < 0: continue

	  (key, label) = d.keyfunc(db['def']['name'], dest, dest)
	  d.allkeys[key] = label

	  data[binidx-2].setdefault(key, []).append([float(nbytes), float(latency)])
      for db in dbs:
        self._queryPruneCache(db)
	db['db'] = None
    finally:
      self.lock.release()

    self._makeLegend(d)
    return (d, series, data)

  def _databases(self, names):
    self.lock.acquire()
    try:
      instances = {}
      for namegroup in names:
        if namegroup in self.db:
          instances[namegroup] = self.db[namegroup]
        else:
          for (x, db) in self.db.iteritems():
            if db['def']['class'] == namegroup:
              instances[x] = db

      valid = []
      for i in instances.values():
        (db, spec, ok) = (i['db'], i['def'], False)
        if db:
          try:
            c = db.cursor()
            c.execute("select sysdate from dual")
            c.fetchall()
	    ok = True
          except Exception, e:
            _logwarn("stale database connection %s, error was %s"
		     % (spec['label'], str(e)))
	    i['db'] = None

        if not ok:
          try:
            # _loginfo("(re)connecting to database %s" % spec['label'])
	    # db = spec['connect']()
	    # c = db.cursor()
	    # c.execute("select sysdate from dual")
	    # c.fetchall()
	    # i['db'] = db
	    ok = True
	  except Exception, e:
            _logwarn("disabling database %s due to connection error %s"
		     % (spec['label'], str(e)))

        if ok:
	  valid.append(i)

      return valid
    finally:
      self.lock.release()

  def plot(self, where, what, **attrs):
    # Collect the database instances to aggregate over.
    instances = self._databases(where.split("+"))
    types = { 'pdf':  'application/pdf',
              'ps':   'application/postscript',
              'eps':  'application/postscript',
	      'png':  'image/png',
	      'svg':  'image/svg+xml',
              'json': 'text/plain' }
    type = attrs.get('type', 'png')
    if type not in types:
      type = 'png'

    what, part = re.match(r"([^:]+)(?::(.*))?", what).groups()
    if part not in ('plot', 'legend', 'info', 'plot+legend'):
      part = 'plot'

    w, h, dpi = 800, 600, 72
    if 'width' in attrs and attrs['width'].isdigit():
      w = int(attrs['width'])
    if 'height' in attrs and attrs['height'].isdigit():
      h = int(attrs['height'])
    if 'dpi' in attrs and attrs['dpi'].isdigit():
      dpi = int(attrs['dpi'])

    # Compute image cache key for this request.
    now = time.time()
    imgkey = ":".join([where, what, ''])
    imgkey += ":".join("%s=%s" % (k, attrs[k])
		       for k in sorted(attrs.keys())
		       if k not in ('session', 'version'))

    # Check if we have a cached image part.  We need to protect access
    # to the cache with a lock, and also lock the individual cache
    # entry so that concurrent accesses will access the same data.
    self.lock.acquire()
    for old in [k for k, v in self.imgcache.iteritems() if v['valid'] < now]:
      del self.imgcache[old]
    if imgkey not in self.imgcache:
      cached = self.imgcache[imgkey] = { 'lock': Lock(), 'valid': 0 }
    else:
      cached = self.imgcache[imgkey]
    self.lock.release()

    try:
      cached['lock'].acquire()
      imgtype, imgdata = cached.get(part, (None, None))
      plot = json = None

      # Draw the plot
      if not imgtype and what == 'link-bytes':
        (d, series, rawdata) = self._linkData(instances, attrs)
        data = [{} for x in series]
        for x, r in zip(data, rawdata):
          x.update((key, float(val[0])/2**40)
		   for key, val in r.iteritems()
		   if val[0] > 0)

        title = d.spanTitle + ' CMS PhEDEx transfer volume, '
        title += " + ".join(x['def']['name'] for x in instances)
        title += "\n" + d.dataTitle
        title += (d.filterTitle and "\n" + d.filterTitle) or ""
        plot = TimeSeriesByteBarChart(d.span, series, data, d.legend, {
	  'title': title, 'yAxisTitle': 'Terabytes',
	  'width': w, 'height': h, 'dpi': dpi, 'imageType': type })
	json = repr({'series': series, 'data': data, 'title': plot.get('title')})

      elif not imgtype and what == 'link-rate':
        (d, series, rawdata) = self._linkData(instances, attrs)
        data = [{} for x in series]
        for x, r, t in zip(data, rawdata, series):
          mbps = 1. / (2**20 * (t[1] - t[0]))
	  x.update((key, float(val[0])*mbps)
		   for key, val in  r.iteritems()
		   if val[0] > 0)

        title = d.spanTitle + ' CMS PhEDEx transfer rate, '
        title += " + ".join(x['def']['name'] for x in instances)
        title += "\n" + d.dataTitle
        title += (d.filterTitle and "\n" + d.filterTitle) or ""
        plot = TimeSeriesBarChart(d.span, series, data, d.legend, {
	  'title': title, 'yAxisTitle': 'MB/s',
	  'width': w, 'height': h, 'dpi': dpi, 'imageType': type })
	json = repr({'series': series, 'data': data, 'title': plot.get('title')})

      elif not imgtype and what == 'rate-vs-quality':
        (d, series, rawdata) = self._linkData(instances, attrs)
        sumdata = {}
        for r in rawdata:
	  for key, val in r.iteritems():
	    if key not in sumdata:
	      sumdata[key] = val
	    else:
	      sumdata[key] += val

        data = {}
        mbps = 1. / (2**20 * (series[-1][1] - series[0][0]))
        for key, val in sumdata.iteritems():
          if val[1]:
	    rate = float(val[0])*mbps
	    qval = val[1]/(val[1]+val[2])
	    data[key] = (rate, qval, val[1])

        bubbles = (attrs.get('bubbles', 'yes') == 'yes')
        letters = (attrs.get('letters', 'yes') == 'yes')
        title = 'CMS PhEDEx transfer rate vs. quality, '
        title += " + ".join(x['def']['name'] for x in instances)
        title += "\n" + d.dataTitle
        title += (d.filterTitle and "\n" + d.filterTitle) or ""
        plot = RateQualityScatterChart(d.span, series, data, d.legend, {
	  'title': title, 'xAxisTitle': 'Quality', 'yAxisTitle': 'MB/s',
	  'width': w, 'height': h, 'dpi': dpi, 'imageType': type,
	  'bubbles': bubbles, 'letters': letters })
	json = repr({'series': series, 'data': data, 'title': plot.get('title')})

      elif not imgtype and what == 'region-cross-rate':
        regions = ["ASGC", "CERN", "CNAF", "FNAL",
		   "FZK", "IN2P3", "PIC", "RAL",
		   "T0", "CAF", "Other"]
        attrs['qty'] = 'link'
        attrs['grouping'] = 'region'
        (d, series, rawdata) = self._linkData(instances, attrs)
        table = dict((r1, dict((r2, 0.) for r2 in regions)) for r1 in regions)
        mbps = 1. / (2**20 * (series[-1][1] - series[0][0]))
        for r in rawdata:
	  for key, val in r.iteritems():
	    table[key[0]][key[1]] += val[0]*mbps

        info = "({ regions: %s, table: %s })" % (repr(regions), repr(table))
	cached['valid'] = now + 60
	cached['plot'] = (None, None)
	cached['legend'] = (None, None)
	cached['info'] = ('text/plain', info)
	imgtype, imgdata = cached['info']

      elif not imgtype and what == 'completed-block-latency-summary':
        attrs['qty'] = 'destination'
        (d, series, rawdata) = self._blockLatencyData(instances, attrs)

        nbins = 100
        if 'bins' in attrs and attrs['bins'].isdigit():
	  nbins = max(1, int(attrs['bins']))

        logtime = True
        if 'timerep' in attrs and attrs['timerep'] in ('log', 'lin'):
	  logtime = (attrs['timerep'] == 'log')

        unit = 'time'
        if 'unit' in attrs and attrs['unit'] in ('time', 'time-vs-size'):
	  unit = attrs['unit']

        if unit == 'time-vs-size':
	  ytitle = "Hours per terabyte"
	  value = lambda bytes, secs: (secs/3600) / (bytes/2**40)
        else:
	  ytitle = "Hours"
	  value = lambda bytes, secs: (secs/3600)

        hmin = hmax = 0
        data = {}
        for r in rawdata:
	  for key, blocks in r.iteritems():
            if key not in data: data[key] = {}
	    for b in blocks:
	      b.append(value(b[0], b[1]))
	      hmax = max(hmax, b[2])
	      hmin = min(hmin, b[2])

        if 'ymin' in attrs and attrs['ymin'].isdigit():
	  hmin = float(attrs['ymin'])
	hmin = max(1, hmin)

        if 'ymax' in attrs and attrs['ymax'].isdigit():
	  hmax = float(attrs['ymax'])
	hmax = max(10, hmax)

        if logtime:
          hstep = 1./nbins * math.log(hmax/hmin, 10)
          def hbinf(value):
	    return int(math.log(value/hmin, 10) / hstep)
	  def hvaluef(bin):
            return hmin * 10**(bin*hstep)
        else:
	  hstep = (hmax-hmin)/nbins
          def hbinf(value):
            return int((value-hmin) / hstep)
	  def hvaluef(bin):
            return hmin + bin*hstep

        for r in rawdata:
	  for key, blocks in r.iteritems():
	    for b in blocks:
	      if b[2] < hmin or b[2] > hmax:
		continue
	      hbin = hbinf(b[2])
	      if hbin not in data[key]:
		data[key][hbin] = 0
	      data[key][hbin] += 1

        title = 'CMS PhEDEx block completion latency, '
        title += " + ".join(x['def']['name'] for x in instances)
        title += "\n" + d.dataTitle
        title += (d.filterTitle and "\n" + d.filterTitle) or ""
        plot = LatencyDensityMap(d.span, series, data, d.legend, {
	  'title': title, 'xAxisTitle': '', 'yAxisTitle': ytitle,
	  'yScaleAxisTitle': 'Number of blocks',
	  'yAxisScale': (logtime and 'log') or 'lin',
	  'yAxisValue': hvaluef, 'yAxisBins': nbins,
	  'width': w, 'height': h, 'dpi': dpi, 'imageType': type })
	json = repr({'params': { 'min': hmin, 'max': hmax,
				 'step': hstep, 'islog': logtime },
		     'data': data, 'title': plot.get('title')})

      # Actually generate the image.
      if not imgtype and plot:
        cached['valid'] = now + 60
        if type != "json":
          cached['plot'] = (types[type], plot.draw().read())
          cached['legend'] = (types[type], plot.legend().read())
        else:
          cached['plot'] = (types[type], json)
          cached['legend'] = (types[type], repr(d.legend))
        cached['info'] = ('text/plain', repr(plot.details()))
        if part=='plot+legend':
          imgtype, imgdata = cached['plot'][0],stitchPlotAndLegend(cached['plot'][1],cached['legend'][1])
        else:
          imgtype, imgdata = cached[part]
    finally:
      cached['lock'].release()

    return imgtype, imgdata

class CompPhEDExWorkspace(CompWorkspace):
  sessiondef = { 'phedex.db.prod': 'yes', 'phedex.db.debug': 'yes' }
  class View:
    def state(self, session):
      start = ''
      end = session.get('phedex.period.end', time.time())
      span = session.get('phedex.period.span', 'hour')
      ntime = session.get('phedex.period.time', '')
      end = timeseries(span, end, end)[0][1]
      if ntime != '':
        if span == 'hour':
          start = end - 3600*ntime
        elif span == 'day':
          start = end - 86400*ntime
        elif span == 'week':
          start = end - 7*86400*ntime
        else:
          start = 0
        ts = timeseries(span, start, end)
        start = (ntime <= len(ts) and ts[-ntime][0]) or ts[0][0]
        end = ts[-1][1]

      return "{kind: '%s', version: %d, start: %s, end: %s, %s}" \
         % (self.plugin, time.time(), repr(start), repr(end),
	    ",".join("'%s': %s" % (x.replace("phedex.", ""), repr(session[x]))
		     for x in sorted(session.keys())
		     if x.startswith("phedex.") and x != 'phedex.view'))

  class LinkRate(View):
    name = "Transfer rate"
    plugin = "CompPhEDExLinkRate"
  class LinkBytes(View):
    name = "Transfer volume"
    plugin = "CompPhEDExLinkBytes"
  class RateQuality(View):
    name = "Rate vs. quality"
    plugin = "CompPhEDExRateQuality"
  class BlockLatency(View):
    name = "Block latency"
    plugin = "CompPhEDExBlockLatency"
  class RegionCrossRate(View):
    name = "Region cross rates"
    plugin = "CompPhEDExRegionCrossRate"

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'phedex', name,
			   [self.LinkRate(), self.LinkBytes(), self.RateQuality(),
			    self.BlockLatency(), self.RegionCrossRate()])
    gui._addJSFragment("%s/javascript/Overview/PhEDEx.js" % gui.contentpath)

  def _state(self, session):
    return self._dostate(session)

  # -----------------------------------------------------------------
  def sessionAlter(self, session, *args, **kwargs):
    opt = kwargs.get('opt', '')
    val = kwargs.get('val', '')
    if opt == 'size' and re.match(r"^[\d ]+$", val):
      v = [int(x) for x in val.split(' ')]
      if len(v) >= 2:
        session['phedex.size.width'] = v[0]
        session['phedex.size.height'] = v[1]
      if len(v) >= 3:
        session['phedex.size.dpi'] = v[2]
    elif opt == 'qty':
      if val in ('source', 'destination', 'link'):
        session['phedex.qty'] = val
    elif opt == 'grouping':
      if val in ('db', 'node', 'site', 'country', 'region', 'tier'):
        session['phedex.grouping'] = val
    elif opt == 'src-grouping':
      if val in ('same', 'db', 'node', 'site', 'country', 'region', 'tier'):
        session['phedex.src-grouping'] = val
    elif opt == 'links':
      if val in ('nonmss', 'mss', 'all'):
        session['phedex.links'] = val
    elif opt == 'detail':
      if val in ('completed-block-latency-summary',):
	session['phedex.detail'] = val
    elif opt == 'range-unit':
      if val in ('time', 'time-vs-size'):
	session['phedex.range.unit'] = val
    elif opt == 'range-linlog':
      if val in ('lin', 'log'):
	session['phedex.range.linlog'] = val
    elif opt in ('range-bins', 'range-ymin', 'range-ymax'):
      if val == '' or val.isdigit():
        session['phedex.' + opt.replace("-", ".")] = val
    elif opt == 'from' or opt == 'to':
      try:
	re.compile(val)
	session['phedex.filter.' + opt] = val
      except:
	pass
    self.gui._saveSession(session)
    return self._state(session)

  def sessionPeriod(self, session, *args, **kwargs):
    span = kwargs.get('span', 'hour')
    time = kwargs.get('time', '')
    upto = kwargs.get('upto', '')

    for k in ('time', 'span', 'upto', 'end'):
      if 'phedex.period.' + k in session:
        del session['phedex.period.' + k]

    if time.isdigit():
      session['phedex.period.time'] = int(time)

    if span in ('hour', 'day', 'week', 'month', 'year'):
      session['phedex.period.span'] = span

    if upto != '':
      if session.get('phedex.period.span', 'hour') == 'week':
        if re.match(r'^\d{6}$', upto):
	  # January 4th is always in (ISO) week one, so find out what
	  # day of week that is, then go back to Monday of that week,
	  # and advance specified number of weeks plus one.
	  jan4 = time2gm(year=int(upto[0:4]), month=1, day=4,
			 hour=23, minute=59, second=59)
          session['phedex.period.upto'] = upto
          session['phedex.period.end'] = \
	    jan4 + (-gmtime(jan4).tm_wday + 7*(int(upto[4:6])-1))*86400;
      else:
	if re.match(r'^\d{6}$', upto):
          session['phedex.period.upto'] = upto
          session['phedex.period.end'] = \
	    time2gm(year=int(upto[0:4]), month=int(upto[4:6])+1,
		    day=1, hour=23, minute=59, second=59) - 86400
	elif re.match(r'^\d{8}$', upto):
          session['phedex.period.upto'] = upto
          session['phedex.period.end'] = \
	    time2gm(year=int(upto[0:4]), month=int(upto[4:6]),
		    day=int(upto[6:8]), hour=23, minute=59, second=59)
	elif re.match(r'^\d{8}[Z ]\d{4}$', upto):
          session['phedex.period.upto'] = upto
          session['phedex.period.end'] = \
	    time2gm(year=int(upto[0:4]), month=int(upto[4:6]),
		    day=int(upto[6:8]), hour=int(upto[9:11]),
		    minute=59, second=59)

    self.gui._saveSession(session)
    return self._state(session)

  def sessionDatabases(self, session, *args, **kwargs):
    for label in kwargs.keys():
      if not re.match(r"^[a-z\d]+$", label):
	continue
      on = kwargs.get(label, 'no')
      if on not in ('yes', 'no'):
	on = 'no'
      session['phedex.db.' + label] = on
    self.gui._saveSession(session)
    return self._state(session)
