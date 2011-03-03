from Monitoring.Overview.GUI import CompWorkspace
from urllib import quote_plus
from time import time
import urllib2

def graphlnk(base, **kwargs):
  return ('http://cmsweb.cern.ch/phedex/graphs/%s?' % base) \
    + "&".join("%s=%s" % (k, quote_plus(v, "/")) for k,v in
	       sorted(kwargs.iteritems()))

class CompDDTSource:
  plothook = 'ddt'

  def __init__(self, *args):
    stime = int(time() / 3600) * 3600 - 12 * 3600
    etime = int(time() / 3600) * 3600
    self.images = {}
    self.images["t0_ch_cern"] = \
      (graphlnk("request", link="dest", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600", node="T0_CH_CERN_MSS"),
       graphlnk("pending", link="dest", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600",
	        from_node=".*", to_node="T0_CH_CERN"),
       graphlnk("pending", link="link", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="600",
	        from_node="T0_CH_CERN_Export", to_node="T1.*"),
       graphlnk("quality_all", link="link", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600",
	        from_node="T0_CH_CERN_Export", to_node="T1.*"),
       graphlnk("pending", link="link", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600",
	        from_node="T0_CH_CERN_Export", to_node="T0_CH_CERN_MSS"),
       graphlnk("quantity", link="link", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600",
	        from_node="T0_CH_CERN_Export", to_node="T0_CH_CERN_MSS"),
       graphlnk("idle", link="link", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600", node="T0_CH_CERN_MSS"),
       graphlnk("request", link="dest", conn="Prod/WebSite",
	        starttime=str(stime), endtime=str(etime),
	        no_mss="false", span="3600", node="T0_CH_CERN_MSS"))
    for s in ("T1_CH_CERN", "T1_DE_FZK", "T1_ES_PIC", "T1_FR_CCIN2P3",
               "T1_IT_CNAF", "T1_TW_ASGC", "T1_UK_RAL", "T1_US_FNAL",
	       "T2_CH_CAF"):
      self.images[s.lower()] = \
        (graphlnk("request", link="dest", conn="Prod/WebSite",
	          starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="false", node=s),
	 graphlnk("pending", link="dest", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="false", from_node=".*", to_node=s),
	 graphlnk("pending", link="link", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="300",
		  no_mss="true", from_node="^T[01]", to_node=s),
	 graphlnk("quality_all", link="link", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="true", from_node="^T[01]", to_node=s),
	 graphlnk("quantity", link="link", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="false", from_node=".*", to_node=s + "_Buffer"),
	 graphlnk("quantity", link="link", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="false", from_node=".*", to_node=s),
	 graphlnk("idle", link="link", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="false", node=s),
	 graphlnk("request", link="dest", conn="Prod/WebSite",
		  starttime=str(stime), endtime=str(etime), span="3600",
		  no_mss="false", node=s))

  def plot(self, site, n, **attrs):
    return ('image/png', urllib2.urlopen(self.images[site.lower()][int(n)]).read())

class CompDDTWorkspace(CompWorkspace):
  class Summary:
    plugin = 'CompDDTSummary'
    name = 'Distributed Data Transfers'

  class Site:
    def __init__(self, name):
      self.name = name
    def state(self, session):
      return "{'kind':'CompDDTSite', 'name':'%s'}" % self.name

  def __init__(self, gui, rank, category, name, *args):
    CompWorkspace.__init__(self, gui, rank, category, 'ddt', name,
			   [self.Summary(),
			    self.Site("T0_CH_CERN"),
			    self.Site("T1_CH_CERN"),
			    self.Site("T1_DE_FZK"),
			    self.Site("T1_ES_PIC"),
			    self.Site("T1_FR_CCIN2P3"),
			    self.Site("T1_IT_CNAF"),
			    self.Site("T1_TW_ASGC"),
			    self.Site("T1_UK_RAL"),
			    self.Site("T1_US_FNAL"),
			    self.Site("T2_CH_CAF")])
    gui._addJSFragment('%s/javascript/Overview/DDT.js' % gui.contentpath)

  def _state(self, session):
    return self._dostate(session, "Distributed_Data_Transfer_monito")
