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
gui.sources.append(ArchiveSource(gui, { "index": "/build/lat/CMSSW_2_2_10/src/VisMonitoring/DQMServer/foo" }))

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

for i in xrange(1, 2): # 20
  session = {
    'core.clientid'   : '127.0.0.1',
    'core.name'       : 'TESTME001',
    'core.public'     : False,
    'core.workspace'  : wsnames[i % len(wsnames)],
    #'dqm.dataset'     : '/RelValCosmics/CMSSW_2_2_4_STARTUP_V8_v1/GEN-SIM-RECO',
    'dqm.dataset'     : '/RelValCosmics/CMSSW_2_2_0_STARTUP_V7_v1/GEN-SIM-RECO',
    'dqm.drawopts'    : {},
    'dqm.filter'      : 'all',
    'dqm.focus'       : {},
    'dqm.myobjs'      : {},
    'dqm.page'        : {},
    'dqm.perpage'     : 12,
    'dqm.picked'      : {},
    'dqm.playinterval' : 5,
    'dqm.playmode'    : 0,
    'dqm.playpos'     : 0,
    'dqm.qplot'       : '',
    'dqm.root'        : {"Everything": ""},
    'dqm.runnr'       : '0',
    'dqm.search'      : '',
    'dqm.size'        : 'M'
  }
  print i, gui.sources[0]._data("rootobj", "0", session['dqm.dataset'], 'EcalBarrel/EventInfo/reportSummaryMap')
  print i, gui.sources[0]._data("scalar", "0", session['dqm.dataset'], 'EcalBarrel/EventInfo/reportSummary')
