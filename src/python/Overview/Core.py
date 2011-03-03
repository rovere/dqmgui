from Monitoring.Overview.GUI import CompWorkspace

class CompCoreWorkspace(CompWorkspace):
  class Summary:
    plugin = 'CompCore'
    name = 'CERN, Core Infrastructure'

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'core', name, [self.Summary()])
    gui._addJSFragment('%s/javascript/Overview/Core.js' % gui.contentpath)

  def _state(self, session):
    return self._dostate(session, "CERN_Core_infrastructure_monitor")
