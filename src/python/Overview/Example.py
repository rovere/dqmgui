from Monitoring.Overview.GUI import CompWorkspace
import time
from copy import copy,deepcopy
from xml.dom.minidom import parseString
import urllib2

##########################################################
# Start of LHCCooldown example
##########################################################

class LHCSource:
  """Our first example. This should be considered a simple, hello-world type
  exercise. The environment we are creating is entirely non-interactive, just
  serving the LHC cooldown state image to the javascript client page."""

  plothook = 'lhc'
  """Set the plothook so this source appears in /overview/plotfairy/lhc."""

  urls = { "lhc.png": "http://hcc.web.cern.ch/hcc/file/field_lhc.png" }
  """Short URLs for images we scrape from origin server."""

  def __init__(self, gui, path, *args):
    """Create a new LHCSource. A no-op."""
    pass

  def plot(self, *path, **attrs):
    """Serve plot requests for /overview/plotfairy/lhc/image-path?key=value

    In this case we don't really need to worry about arguments since we're only
    expecting one possible value, path=['lhc.png'] and no attributes, but we'll
    check anyway.

    :return: tuple ('image/png', image-data)."""
    # Check for expected number of arguments
    if len(path) == 1:
      # Check it's a short url we know of
      if path[0] in self.urls:
        mimetype = 'image/png' # We'll just assume they're all png for the moment
        url = self.urls[path[0]] # Get the full URL from the dictionary
        data = urllib2.urlopen(url).read() # Read the data from this url
        return mimetype, data # Return the data and mimetype

    # Something went wrong, return an error condition
    return None, None

class LHCWorkspace(CompWorkspace):
  """Workspace for the LHC example. This again is very simple, having only
  a single view and no state updates to worry about. All the necessary
  methods are implemented, and nothing more."""

  sessiondef = {'lhc.view': 'cooldown'}
  """Defaults to be copied into each new session. Set view to 'cooldown'."""

  class CooldownView:
    """Inner class representing the cooldown view. Unnecessary when we have
    only a single view, but the convention of having a class per view can be
    a useful way of handling things."""
    #: The title that will be displayed
    name = 'Cooldown Summary'

    #: The name that will be used when view?name= is called
    label = 'cooldown'

    #: The name of the plugin that will be loaded for this view
    plugin = 'LHCCooldown'

  def __init__(self, gui, rank, category, name, *args):
    """Create a new workspace, and store a reference to the server ('gui')
    object.  Also create the list of views for the workspace."""
    CompWorkspace.__init__(self, gui, rank, category, 'lhc', name,
                           [LHCWorkspace.CooldownView()])
    gui._addJSFragment("%s/javascript/Overview/Example.js" % gui.contentpath)

  def _state(self, session):
    """Return JSON session state."""
    return self._dostate(session)

##########################################################
# End of LHCCooldown example
##########################################################


##########################################################
# Start of TPGQuery example
##########################################################

class TPGWorkspace(CompWorkspace):
  """Workspace for our second example. This is slightly more complex (handles
  interaction with javascript), but large parts are in common with the previous
  example and won't be dwelt on."""
  class TPGView:
    #: The title that will be displayed
    name = 'TPG Query'

    #: The name that will be used when view?name= is called
    label = 'query'

    #: The name of the plugin that will be loaded for this view
    plugin = 'TPGQuery'

    def state(self,session):
      """Return JSON fragment for the view state."""
      return "{'kind': '%s', %s}" \
        % (self.plugin, ",".join("%r:%r" % (key,session[key])
                        for key in session if key.startswith('tpg.')))


  sessiondef =  {
          'tpg.view': 'query',
          'tpg.line': '16662',
          'tpg.direction': 'r',
          'tpg.stop': '1'
          }
  """Defaults to be copied into each new session. Set the defaults:

   * default line: 16662 (bus 56)
   * default direction: r (CERN-Avanchet)
   * default stop: 1 (CERN)."""

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'tpg', name,
                           [TPGWorkspace.TPGView()])
    gui._addJSFragment("%s/javascript/Overview/Example.js" % gui.contentpath)

  def sessionAlter(self, session, *args, **kwargs):
    """Landing function for session-changing calls. When a list value is
    changed, a call is made to here."""
    # Get the field to be altered and the new value from the keywords
    field = kwargs.get('field', '')
    value = kwargs.get('value', '')

    # Validates the field being changed
    if field in ('line', 'direction', 'stop'):
      # For change in line, make the current stop and direction null since
      # they're now undefined. String 'null' is used as a signalling value
      # for javascript here (as opposed to None). We need to check that the
      # values here are roughly what we expect (to guard against XSS attacks,
      # for instance). The value should be an integer ~16000.
      if field == 'line':
	if value.isdigit():
          session['tpg.line'] = value
          session['tpg.direction'] = 'null'
          session['tpg.stop'] = 'null'
        else:
          return None

      # Likewise for direction; value should be a char 'a' or 'r'
      elif field == 'direction':
        if value in ('a', 'r'):
          session['tpg.direction'] = value
          session['tpg.stop'] = 'null'
        else:
          return None

      # And stop; value should be a small positive integer.
      elif field == 'stop':
        if value.isdigit():
          session['tpg.stop'] = value
        else:
          return None

    self.gui._saveSession(session)
    return self._state(session)

  def _state(self,session):
    return self._dostate(session)

class TPGSource:
  """Source for the TPG example. This retrieves data from the backend
  (in this case the TPG AJAX landing page), processes it into an
  appropriate format and serves it."""
  plothook = 'tpg'

  def __init__(self, gui, path, *args):
    pass

  def plot(self, *path, **attrs):
    """Return text or image data. In this case we'll only be serving text
    data. We need to check that it is valid, and then retrieve the
    information from the backend, format it, and serve it.

    Expected queries:
     * ['linelist']
     * ['directionlist'] { 'line': 'linenum' }
     * ['stoplist'] { 'line': 'linenum', 'direction': 'directionchar' }
     * ['times'] { 'line': 'linenum', 'direction': 'directionchar', 'stop': 'stopnum' }"""

    # Check for right number of arguments
    if len(path) == 1:
      path = path[0]

      # Check for known arguments
      if path in ('linelist', 'directionlist', 'stoplist', 'times'):

        # If we just want the list of lines, return it as a json object
        if path == 'linelist':
          return 'text/plain', "(%r)" % (self._fetchLineList())

        # If we want the list of directions, check a line has been supplied
        elif path == 'directionlist':
          if 'line' in attrs:
            return 'text/plain', "(%r)" % (self._fetchDirectionList(attrs['line']))

        # For a list of stops, we need a line and direction
        elif path == 'stoplist':
          if 'line' in attrs and 'direction' in attrs:
            return 'text/plain', "(%r)" % (self._fetchStopList(attrs['line'], attrs['direction']))

        # and for a list of times we need all three
        elif path == 'times':
          if 'line' in attrs and 'direction' in attrs and 'stop' in attrs:
            return 'text/plain', "(%r)" % (self._fetchResult(attrs['line'], attrs['direction'], attrs['stop']))

    # Something went wrong, return nothing (500 Internal Server Error)
    return None, None

  ##################
  # How the following methods work is not really important to the example, but
  # it should be fairly simple to see how they fetch XML and then process it with DOM
  ##################
  def _fetchLineList(self):
    return { '16648': '36', '16649': '41', '16660': '54', '16661': '55',
             '16666': 'd', '16667': 'e', '16664': 'b', '16663': 'a',
             '16640': '27', '16641': '28', '16642': '29', '16643': '31',
             '16644': '32', '16645': '33', '16646': '34', '16647': '35',
             '16672': 's', '16628': '11', '16629': '12', '16626': '9',
             '16627': '10', '16624': '7', '16625': '8', '16622': '5',
             '16623': '6', '16620': '2', '16621': '3', '16677': 'z',
             '16665': 'c', '16676': 'y', '16668': 'f', '16671': 'l',
             '16670': 'k', '16673': 'v', '16669': 'g', '16675': 'x',
             '16674': 'w', '16659': '53', '16658': '52', '16657': '51',
             '16656': '49', '16655': '48', '16654': '46', '16653': '45',
             '16652': '44', '16651': '43', '16650': '42', '16619': '1',
             '16639': '23', '16638': '22', '16662': '56', '16635': '19',
             '16634': '17', '16637': '21', '16636': '20', '16631': '14',
             '16630': '13', '16633': '16', '16632': '15'}

  def _fetchDirectionList(self, line):
    dir = {}
    try:
      data = urllib2.urlopen("http://www.tpg.ch/_services/nextHoraireForm.php"
                             "?todo=ajaxSearch&lineSelector=%s&t=%s"
                             % (line, time.strftime("%H:%M:%S"))).read()
      dom = parseString(data)
      d_blocks = dom.getElementsByTagName("option")

      for b in d_blocks:
        short = b.getAttribute("value").encode('ascii', 'replace')
        if len(short.strip()) > 0:
          desc = b.firstChild.nodeValue.encode('ascii', 'replace')
          dir[short] = desc

      return dir
    except URLError:
      return None

  def _fetchStopList(self, line, direction):
    stops = {}
    try:
      data = urllib2.urlopen("http://www.tpg.ch/_services/nextHoraireForm.php"
                             "?todo=ajaxSearch&lineSelector=%s"
                             "&directionSelector=%s&t=%s"
                             % (line, direction, time.strftime("%H:%M:%S"))).read()
      dom = parseString(data)
      d_blocks = dom.getElementsByTagName("option")

      for b in d_blocks:
        short = b.getAttribute("value").encode('ascii', 'replace')
        if len(short.strip()) > 0:
          desc = b.firstChild.nodeValue.encode('ascii', 'replace')
          stops[short] = desc

      return stops
    except URLError:
      return None

  def _fetchResult(self, line, direction, stop):
    times = []
    try:
      data = urllib2.urlopen("http://www.tpg.ch/_services/nextHoraireForm.php"
                             "?todo=ajaxSearch&lineSelector=%s"
                             "&directionSelector=%s&arretSelector=%s&t=%s"
                             % (line, direction, stop, time.strftime("%H:%M:%S"))).read()
      data = "<all>%s</all>" % data
      dom = parseString(data)
      d_blocks = dom.getElementsByTagName("div")

      for b in d_blocks:
        if b.getAttribute("class") == "nextDepartureItem":
          times.append(b.firstChild.nodeValue.encode('ascii', 'replace'))

      return times
    except URLError:
      return None

##########################################################
# End of TPGQuery example
##########################################################
