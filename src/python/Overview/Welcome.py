from Monitoring.Overview.GUI import CompWorkspace

class CompWelcomeWorkspace(CompWorkspace):
  class Summary:
    plugin = 'CompWelcome'
    name = 'Welcome'

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'welcome', name, [self.Summary()])
    gui._addJSFragment('%s/javascript/Overview/Welcome.js' % gui.contentpath)
    gui._addCSSFragment('%s/css/Overview/Welcome.css' % gui.contentpath)

  def _state(self, session):
    return self._dostate(session, "Before_you_proceed")
