import time, os, time
import libVisDQMServer as Native
from threading import Thread

class Gui:
  stamp = time.time()
  serviceName = "Online"
  services = [("Online", "http://cmsdoc.cern.ch/dqm/online")]
  sources = []
  workspaces = []
  def _noResponseCaching(self): pass

class ArchiveSource(Native.DQMArchiveSource):
  def __init__(self, gui, opts):
    Native.DQMArchiveSource.__init__(self, gui, opts)

wsnames = []
wsobj = []
gui = Gui()
gui.sources.append(ArchiveSource(gui, {
  "index": "/build/lat/CMSSW_3_2_1/src/VisMonitoring/DQMServer/foo",
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
    self.num = num
    self.session = {
      'core.clientid'   : '127.0.0.1',
      'core.name'       : 'TESTME%03d' % num,
      'core.public'     : False,
      'core.workspace'  : wsnames[num % len(wsnames)],
      'dqm.drawopts'    : {},
      'dqm.filter'      : 'all',
      'dqm.focus'       : {},
      'dqm.myobjs'      : {},
      'dqm.play.interval' : 5,
      'dqm.play.pos'     : 0,
      'dqm.play.prevws'    : '',
      'dqm.qplot'       : '',
      'dqm.root'        : {"Everything": ""}, #roots[num % len(roots)]},
      #'dqm.sample.dataset'     : '/Cosmics/CRUZET09-PromptReco-v1/RECO',
      'dqm.sample.dataset'     : '/RelValCosmics/CMSSW_2_2_4_STARTUP_V8_v1/GEN-SIM-RECO',
      #'dqm.sample.dataset'     : '/RelValCosmics/CMSSW_2_2_0_STARTUP_V7_v1/GEN-SIM-RECO',
      #'dqm.sample.runnr'       : 98493,
      'dqm.sample.runnr'       : 0,
      'dqm.sample.keep'       : '',
      'dqm.sample.pattern'       : '',
      'dqm.sample.prevws'       : '',
      'dqm.sample.type'       : 3,
      #'dqm.sample.type'       : 2,
      'dqm.search'      : '',
      'dqm.size'        : 'M',
      'dqm.submenu'        : 'data'
    }

  def run(self):
    for x in xrange(1, 20):
      for i in xrange(1, 301):
        print self.num, x, i, len(wsobj[i % len(wsobj)]._state(self.session))

for i in xrange(1, 8):
  PollThread(i).start()
