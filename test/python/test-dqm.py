import time
import libVisDQMServer as Native

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
gui.sources.append(ArchiveSource(gui, { "index": "/build/lat/CMSSW_2_2_10/src/VisMonitoring/DQMServer/foo", "rxonline": "^/Global/" }))

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

for i in xrange(1, 10): # 20
  session = {
    'core.clientid'      : '127.0.0.1',
    'core.name'          : 'TESTME001',
    'core.public'        : False,
    'core.workspace'     : wsnames[i % len(wsnames)],
    'dqm.sample.type'    : 3,
    'dqm.sample.runnr'   : 0,
    #'dqm.sample.dataset' : '/RelValCosmics/CMSSW_2_2_4_STARTUP_V8_v1/GEN-SIM-RECO',
    'dqm.sample.dataset' : '/RelValCosmics/CMSSW_2_2_0_STARTUP_V7_v1/GEN-SIM-RECO',
    'dqm.drawopts'       : {},
    'dqm.filter'         : 'all',
    'dqm.submenu'        : 'data',
    'dqm.focus'          : {},
    'dqm.myobjs'         : {},
    'dqm.picked'         : {},
    'dqm.playinterval'   : 5,
    'dqm.playmode'       : 0,
    'dqm.playpos'        : 0,
    'dqm.qplot'          : '',
    'dqm.root'           : {"Everything": ""},
    'dqm.search'         : '',
    'dqm.size'           : 'M'
  }
  print i, wsobj[i % len(wsobj)]._state(session)

gui.sources[0]._stop()
