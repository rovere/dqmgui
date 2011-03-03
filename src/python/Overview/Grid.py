from Monitoring.Overview.GUI import CompWorkspace

class CompGridWorkspace(CompWorkspace):
  class Summary:
    plugin = 'CompGrid'
    name = 'Distributed Grid Sites'

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'grid', name, [self.Summary()])
    gui._addJSFragment('%s/javascript/Overview/Grid.js' % gui.contentpath)

  def _state(self, session):
    return self._dostate(session, "Distributed_Grid_sites_monitorin")
