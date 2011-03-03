from Monitoring.Overview.GUI import ImageScraperSource, CompWorkspace, lemonimg, lrfimg, slsimg

class CompTier0Source(ImageScraperSource):
  def __init__(self, *args):
    ImageScraperSource.__init__(self, 'tier0', [
      lemonimg("status", "info", ['t0exp_load_average'], "cache/", entity="c2cms/t0export", time=0, offset=0),
      lemonimg("status", "info", ['t0exp_cpu_utilization', 't0exp_net_utilization'], "/lemon-status/images/", entity="c2cms/t0export", time=0, offset=0),
      lemonimg("status", "info", ['t1transfer_load_average'], "cache/", entity="c2cms/t1transfer", time=0, offset=0),
      lemonimg("status", "info", ['t1transfer_cpu_utilization', 't1transfer_net_utilization'], "/lemon-status/images/", entity="c2cms/t1transfer", time=0, offset=0),
      lemonimg("web", "info", ['cmst0_load_average'], "cache/", entity="lxbatch/cmst0"),
      lemonimg("web", "info", ['cmst0_cpu_utilization', 'cmst0_net_utilization'], "/lemon-web/images/", entity="lxbatch/cmst0"),
      lemonimg("web", "info", ['cmsproc_load_average'], "cache/", entity="lxbatch/cmsproc"),
      lemonimg("web", "info", ['cmsproc_cpu_utilization', 'cmsproc_net_utilization'], "/lemon-web/images/", entity="lxbatch/cmsproc"),
      lrfimg(['tier0_jobs'], "/lrf-lsf/images/", queue="cmst0", detailed=0, auto_update=0, time=0),
      slsimg(['tier0export_space'], "graph.php", id="CASTORCMS_T0EXPORT", more="nv:Total Space TB", period="day") ])

class CompTier0Workspace(CompWorkspace):
  class Summary:
    plugin = 'CompTier0Summary'
    name = 'Tier 0 Workflows summary'

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'tier0', name, [self.Summary()])
    gui._addJSFragment('%s/javascript/Overview/Tier0.js' % gui.contentpath)

  def _state(self, session):
    return self._dostate(session, "Tier_0_workflows_monitoring")
