import sys, time, os, time, thread
import libVisDQMServer as Native
from threading import Thread
from copy import deepcopy
from glob import glob
from random import shuffle
thread.stack_size(128*1024)

class Gui:
  stamp = time.time()
  serviceName = "Online"
  services = [("Online", "http://cmsdoc.cern.ch/dqm/online")]
  sources = []
  workspaces = []
  def _noResponseCaching(self): pass

class LiveSource(Native.DQMLiveSource):
  def __init__(self, gui):
    Native.DQMLiveSource.__init__(self, gui, {
     "dataset": "/Global/Online/ALL", "verbose": False,
     "dqmhost": "dqm-integration", "dqmport": 9090 })

gui = Gui()
link = Native.DQMRenderLink("foo", glob("/home/dqm*/gui/render/*.ext")[0], 5, False)
live = LiveSource(gui)
gui.sources.append(live)
ws = Native.DQMContentWorkspace(gui, "Everything", "^")
ws.category = "Summary"
ws.name = "Everything"
ws.rank = 0
gui.workspaces.append(ws)
if getattr(link, "_start", None):
  link._start()

class PollThread(Thread):
  def __init__(self, num):
    Thread.__init__(self)
    self.num = num
    self.params = [{ 'w': "532",  'h': "400" },
		   { 'w': "266",  'h': "200" },
		   { 'w': "1064", 'h': "800" }]
    self.roots = ["CSC", "Castor", "Collisions", "DT", "EcalBarrel",
                  "EcalEndcap", "EcalPreshower", "HLT", "HLX",
                  "Hcal", "HcalCalib", "L1T", "L1TEMU", "Pixel",
                  "RPC", "SiStrip"]
    self.session = {
      'core.clientid'      : '127.0.0.1',
      'core.name'          : 'TESTME%03d' % num,
      'core.public'        : False,
      'core.workspace'     : ws.name,
      'dqm.drawopts'       : {},
      'dqm.filter'         : 'all',
      'dqm.focus'          : {},
      'dqm.myobjs'         : {},
      'dqm.play.interval'  : 5,
      'dqm.play.pos'       : 0,
      'dqm.play.prevws'    : '',
      'dqm.qplot'          : '',
      'dqm.reference'	   : {'position': 'overlay', 'show': 'customise', 'param': [
				{'type': 'refobj', 'run': '', 'dataset': ''},
				{'type': 'none', 'run': '', 'dataset': ''},
				{'type': 'none', 'run': '', 'dataset': ''},
				{'type': 'none', 'run': '', 'dataset': ''} ]},
      'dqm.root'           : {"Everything": ""},
      'dqm.sample.dataset' : '/Global/Online/ALL',
      'dqm.sample.order'   : 'dataset',
      'dqm.sample.runnr'   : 0,
      'dqm.sample.keep'    : '',
      'dqm.sample.pattern' : '',
      'dqm.sample.prevws'  : '',
      'dqm.sample.type'    : 0,
      'dqm.search'         : '',
      'dqm.size'           : 'M',
      'dqm.strip.axis'     : 'run',
      'dqm.strip.n'        : '50',
      'dqm.strip.omit'     : 'none',
      'dqm.strip.type'     : 'object',
      'dqm.submenu'        : 'data'
    }

  def ls(self, s):
    state = eval(ws._state(s))
    dirs = [x['name'] for x in state[-1]['items'] if x.get('dir', None)]
    objs = [x['name'] for x in state[-1]['items'] if not x.get('dir', None) and x['name'].find('Castor Pedestal Value') < 0]
    shuffle(dirs)
    shuffle(objs)
    return (dirs, objs)

  def dump(self, level, s, root):
    if level <= 0: return
    s['dqm.root']['Everything'] = root
    (dirs, objs) = self.ls(s)
    for p in self.params:
      for nd in xrange(0, len(dirs)):
        self.dump(level-1, s, dirs[nd])
      for o in objs:
        t = time.time()
        (type, data) = live._plot(o, p)
        sys.stdout.write("%03d %s (%s) = %s, %d bytes, %d ms\n" %
		         (self.num, o, p, type, (data and len(data) or 0),
			  int((time.time() - t) * 1e6)))

  def run(self):
    s = deepcopy(self.session)
    (dirs, objs) = ((), ())
    while len(dirs) == 0:
      sys.stdout.write("%03d: waiting to connect\n" % self.num)
      (dirs, objs) = self.ls(s)
      time.sleep(.5)
    self.dump(3, s, "")

for i in xrange(1, 10):
  PollThread(i).start()
