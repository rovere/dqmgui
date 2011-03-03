import re, time, calendar, logging
from cherrypy import log

RE_DIGIT_SEQ = re.compile(r'([-+]?\d+)')
RE_THOUSANDS = re.compile(r'(\d)(\d{3}($|\D))')

# Logging methods.
def _logerr(msg):
  log("ERROR: " + msg.replace("\n", " ~~ "),
      severity=logging.ERROR)

def _logwarn(msg):
  log("WARNING: " + msg.replace("\n", " ~~ "),
      severity=logging.WARNING)

def _loginfo(msg):
  log("INFO: " + msg.replace("\n", " ~~ "),
      severity=logging.INFO)

# Format numbers nicely.
def thousands(s):
  while True:
    r = re.sub(RE_THOUSANDS, r"\1'\2", s)
    if r == s:
      return r
    else:
      s = r
  return s

# Sort naturally.
def natconvert(list):
  return [ ((i % 2 == 0 and (list[i],)) or (int(list[i]),))[0]
	   for i in xrange(0, len(list)) ]

def natsort(list):
  temp = [ (natconvert(re.split(RE_DIGIT_SEQ, s)), s) for s in list ]
  temp.sort()
  return [ t[1] for t in temp ]

def natsorted(list):
  temp = [ (natconvert(re.split(RE_DIGIT_SEQ, s)), s) for s in list ]
  temp.sort()
  return [ t[1] for t in temp ]

# Generate time series from START to END using SPAN as the unit.
#
# Returns a list of (LOW, HIGH) tuples where each tuple defines a
# half-open interval [LOW, HIGH).  The HIGH of one tuple will be the
# LOW of the following one.
#
# If SPAN is "hour", generates intervals of hours.  If SPAN is "day",
# generates intervals for days, from midnight to midnight in UTC time.
# If SPAN is "week", returns intervals of ISO weeks, from midnight on
# Monday to the next Monday.  If RANGE is "month", returns intervals of
# calendar months from midnight of the first day of the month to the
# midnight of the first day of the next month.
#
# Time series starts from the beginning of the interval START falls
# in and ends in an interval that contains END.  If START == END,
# then returns a list of a single tuple containing START and END.
def timeseries(span, start, end):
  if span == 'hour':
    # Convert first time to UTC hour, then make a series of hours.
    low = int(start / 3600)
    high = max(low+1, int((end+3599) / 3600))
    return [(t*3600, (t+1)*3600) for t in xrange(low, high)]
  elif span == 'day':
    # Convert first time to UTC day at 00:00, then make a series of days.
    low = int(start / 86400)
    high = max(low+1, int((end+86399) / 86400))
    return [(t*86400, (t+1)*86400) for t in xrange(low, high)]
  elif span == 'week':
    # Convert first time to previous Monday.  Then make a
    # time series of weeks until we pass the end date.
    low = int(start/86400) - time.gmtime(start).tm_wday
    high = max(low+1, int((end+86399) / 86400))
    return [(t*86400, (t+7)*86400) for t in xrange(low, high, 7)]
  elif span == 'month':
    # Create a time series for each first of the month.
    series = []
    limit = int((end+86399)/86400)*86400
    t = time.gmtime(start)
    year = t.tm_year
    month = t.tm_mon
    day = calendar.timegm((year, month, 1, 0, 0, 0, 0, 0, 0))
    while True:
      low = day
      month += 1
      if month > 12:
	month = 1
	year += 1
      day = calendar.timegm((year, month, 1, 0, 0, 0, 0, 0, 0))
      series.append((low, day))
      if day >= limit:
	break
    return series
  elif span == 'year':
    # Create a series of the first of January of each year.
    series = []
    limit = int((end+86399)/86400)*86400
    t = time.gmtime(start)
    year = t.tm_year
    day = calendar.timegm((year, 1, 1, 0, 0, 0, 0, 0, 0))
    while True:
      low = day
      year += 1
      day = calendar.timegm((year, 1, 1, 0, 0, 0, 0, 0, 0))
      series.append((low, day))
      if day >= limit:
	break
    return series

# Format TIME in numerical format as unit of SPAN ("hour", "day", "week" or "month").
def numtimefmt(span, timeval):
  if span == 'hour': return time.strftime('%Y%m%dZ%H00', time.gmtime(int(timeval)))
  elif span == 'day': return time.strftime('%Y%m%d', time.gmtime(int(timeval)))
  elif span == 'week': return time.strftime('%Y%V', time.gmtime(int(timeval)))
  elif span == 'month': return time.strftime('%Y%m', time.gmtime(int(timeval)))
  elif span == 'year': return time.strftime('%Y', time.gmtime(int(timeval)))

# Convert a storage size into a numeric value (as bytes).  Storage
# sizes are a floating point number optionally followed by a letter
# "k", "M", "G", "T", "P" or "E" for kilo-, mega-, giga-, tera-,
# peta- and exabytes, respectively.  A raw number is accepted as
# well, returned as such, i.e. as bytes.
def sizevalue(val):
  m = re.match(r'^([-+\d.Ee]+)([kMGTPE])$', val)
  if m:
    scale = { 'k': 2**10, 'M': 2**20, 'G': 2**30, 'T': 2**40, 'P': 2**50, 'E': 2**60 }
    return float(m.group(1)) * scale[m.group(2)]
  else:
    return float(val)
