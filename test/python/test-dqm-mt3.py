import time, os, time, re, md5, thread
import libVisDQMServer as Native
from threading import Thread

thread.stack_size(128*1024)

class Gui:
  stamp = time.time()
  serviceName = "Online"
  services = [("Online", "http://cmsdoc.cern.ch/dqm/online")]
  extensions = []
  sources = []
  workspaces = []
  def _noResponseCaching(self): pass

class ArchiveSource(Native.DQMArchiveSource):
  def __init__(self, gui, opts):
    Native.DQMArchiveSource.__init__(self, gui, opts)

class RenderLink(Native.DQMRenderLink):
  def __init__(self, gui):
    Native.DQMRenderLink.__init__(self, ".", "", 5, False)

class StripSource(Native.DQMStripChartSource):
  def __init__(self, gui):
    Native.DQMStripChartSource.__init__(self)
    self.sources = gui.sources
  def plot(self, *path, **options):
    sources = dict((s.plothook, s) for s in self.sources
		   if getattr(s, 'plothook', None))
    (srcname, runnr, dataset) = options.get("current").split("/", 2)
    info = (sources[srcname], int(runnr), "/" + dataset)
    return self._plot(sources.values(), info, "/".join(path), options)

wsnames = []
wsobj = []
gui = Gui()
gui.extensions.append(RenderLink(gui))
gui.sources.append(StripSource(gui))
gui.sources.append(ArchiveSource(gui, {
  "index": "/build/lat/dqmgui/ix",
  "rxonline": "^/Global/" }))

ws = Native.DQMQualityWorkspace(gui, "Summary")
ws.category = "Summary"
ws.name = "Summary"
ws.rank = 0
gui.workspaces.append(ws)
wsnames.append(ws.name)
wsobj.append(ws)

ws = Native.DQMSummaryWorkspace(gui, "Reports")
ws.category = "Summary"
ws.name = "Reports"
ws.rank = 0
gui.workspaces.append(ws)
wsnames.append(ws.name)
wsobj.append(ws)

ws = Native.DQMContentWorkspace(gui, "Everything", "^")
ws.category = "Summary"
ws.name = "Everything"
ws.rank = 0
gui.workspaces.append(ws)
wsnames.append(ws.name)
wsobj.append(ws)

ws = Native.DQMSampleWorkspace(gui, "Sample")
ws.category = "Internal"
ws.name = "Sample"
ws.rank = -1
gui.workspaces.append(ws)
wsnames.append(ws.name)
wsobj.append(ws)

ws = Native.DQMPlayWorkspace(gui, "Play")
ws.category = "Internal"
ws.name = "Play"
ws.rank = -1
gui.workspaces.append(ws)
wsnames.append(ws.name)
wsobj.append(ws)

class PollThread(Thread):
  def __init__(self, num):
    Thread.__init__(self)
    roots = ["CaloTowersV", "DT", "EcalBarrel", "EcalDigisV", "EcalEndcap",
             "EcalHitsV", "EcalRecHitsV", "GlobalDigisV", "GlobalHitsV",
             "GlobalRecHitsV", "HLT", "Hcal", "HcalHitsV", "HcalRecHitsV",
             "L1T", "MuonCSCDigisV", "MuonDTDigisV", "MuonDTHitsV",
             "Muons", "Pixel", "RPCDigisV", "RecoMuonV", "RecoTrackV",
             "SiStrip", "TrackerDigisV", "TrackerHitsV", "TrackerRecHitsV",
             "TrackingMCTruthV"]
    roots = [ "00 Shift", "AlCaReco", "CSC", "Castor", "DT", "EcalBarrel",
              "EcalEndcap", "EcalPreshower", "Egamma", "FED", "HLT", "Hcal",
              "JetMET", "L1T", "L1TEMU", "MessageLogger", "Muons", "Physics",
              "Pixel", "RPC", "SiStrip", "Tracking" ]
    self.num = num
    self.session = {
      'core.clientid'   : '127.0.0.1',
      'core.name'       : 'TESTME%03d' % num,
      'core.public'     : False,
      'core.stamp'      : time.time(),
      'core.user'       : "Tester",
      'core.workspace'  : wsnames[num % len(wsnames)],
      'dqm.drawopts'    : {},
      'dqm.filter'      : 'all',
      'dqm.focus'       : {},
      'dqm.myobjs'      : {},
      'dqm.panel.help.show' : False,
      'dqm.play.interval' : 5,
      'dqm.play.pos'     : 0,
      'dqm.play.prevws' : 'Everything',
      'dqm.qplot'       : '',
      'dqm.reference'   : {'position': 'overlay', 'show': 'customise', 'param': [
			    {'type': 'refobj', 'run': '', 'dataset': ''},
			    {'type': 'none', 'run': '', 'dataset': ''},
			    {'type': 'none', 'run': '', 'dataset': ''},
			    {'type': 'none', 'run': '', 'dataset': ''}]},
      'dqm.root'        : {"Everything": roots[num % len(roots)]},
      'dqm.sample.dataset' : "/Cosmics/Commissioning09-PromptReco-v9/RECO", #'/RelValCosmics/CMSSW_2_2_4_STARTUP_V8_v1/GEN-SIM-RECO', #'/RelValCosmics/CMSSW_2_2_0_STARTUP_V7_v1/GEN-SIM-RECO',
      'dqm.sample.order' : 'dataset',
      'dqm.sample.pattern': '',
      'dqm.sample.prevws': 'Everything',
      'dqm.sample.runnr': 119228, # 0
      'dqm.sample.type' : 2,
      'dqm.sample.vary' : 'run',
      'dqm.search'      : '',
      'dqm.size'        : 'M',
      'dqm.strip.axis'  : 'run',
      'dqm.strip.n'     : '',
      'dqm.strip.omit'  : 'none',
      'dqm.strip.type'  : 'object',
      'dqm.submenu'     : 'data'
    }

  def run(self):
    for z in xrange(1, 101):
      for i in xrange(1, 101):
        ws = wsobj[i % len(wsobj)]
        data = eval(ws._state(self.session))
        print self.num, z, i, len([x for x in data if x['kind'] not in ('AutoUpdate', 'DQMHeaderRow')])
        print md5.new(gui.sources[0].plot("L1TEMU", "DTTPG", "DTPErrorFlag", trend="x-mean-rms", n=("%d" % (2*i+z)), omit="none", axis="run", current="archive/119228/Cosmics/Commissioning09-PromptReco-v9/RECO", w=("%d" % (200+i)), h=("%d" % (200+z)))[1]).hexdigest()
        #time.sleep(.1)

for i in xrange(1, 50):
  PollThread(i).start()
