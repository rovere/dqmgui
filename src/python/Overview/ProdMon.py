#!/usr/bin/env python

from Monitoring.Core.Plot import *
from Monitoring.Core.Utils import _logerr, _logwarn, _loginfo
from Monitoring.Overview.GUI import CompWorkspace
from cherrypy import HTTPError, engine
from copy import deepcopy, copy
import time, datetime
import logging
import random
import urllib,urllib2
import socket
import re
import json
from xml.dom.minidom import parseString

class ProdMonWorkspace(CompWorkspace):
  """
  Main production monitoring workspace class.
  Handles the current front end state, context,
  recent plots etc.
  """
  sessiondef = {'prodmon.view':'summary'}
  class ProdMonView:
    def __init__(self,workspace):
      self.workspace = workspace

  class ProdMonSummary(ProdMonView):
    label="summary"
    pluginname="ProdMonSummary"
    name="Summary"
    viewdef={}
    def state(self,session):
      result = {'kind': self.pluginname}
      result.update(self.workspace.source.getStatsSummary())
      return json.dumps(result)
    def initialiseSession(self,session,*args,**kwargs):
      pass

  class ProdMonPlot(ProdMonView):
    label="plot"
    pluginname="ProdMonPlot"
    name="FreePlot"

    plotdef={
         'sortby':'site',
         'what':'evts_read_any',
         'job_type':'Any',
         'graph':'bar',
         'regex':'null',
         'period':48,
         'use_period':'true',
         'group':'null',
         'filters':[],
         'image_width':800,
         'image_height':600,
         'image_dpi':72,
         'starttime':'',
         'endtime':'',
         'baseurl':'',
         'optstr':'',
         'title':'',
         'oldtitle':''
         }
    viewdef={'prodmon.plot.open':[deepcopy(plotdef)],
             'prodmon.plot.current':0
             }
    def state(self,session):
      result = {'kind': self.pluginname}
      result.update(session['prodmon.plot.open'][session['prodmon.plot.current']])
      result['open'] = [{'index':i,'title':o['title'],'baseurl':o['baseurl'],'optstr':o['optstr']} for i,o in enumerate(session['prodmon.plot.open'])]
      result['current'] = session['prodmon.plot.current']

      return json.dumps(result)

    def initialiseSession(self,session,*args,**kwargs):
      for key,value in self.viewdef.iteritems():
        if key not in session:
          session[key]=deepcopy(value)
      current_plot = session['prodmon.plot.open'][session['prodmon.plot.current']]
      if 'new' in kwargs:
        current_plot = deepcopy(current_plot)
        session['prodmon.plot.open'].append(current_plot)
        session['prodmon.plot.current']=len(session['prodmon.plot.open'])-1
      for key,value in kwargs.items():
        value=urllib.unquote(value)
        if key in ('sortby','what','job_type','starttime','endtime','regex','graph','group','image_width','image_height','image_dpi','period','use_period','title'):
          if self.workspace._verify(key,value):
            current_plot[key]=value
      if 'filter_name' in kwargs and 'filter_value' in kwargs:
        name = urllib.unquote(kwargs['filter_name'])
        value = urllib.unquote(kwargs['filter_value'])
        if '|' in name and '|' in value:
          current_plot['filters'] = [[n,v] for n,v in zip(name.split('|'),value.split('|'))]
        else:
          current_plot['filters'] = [[name,value]]
      self.workspace._updateURLs(session)

  def __init__(self,gui,rank,category,name,*args):
    """
    Create a new ProdMonWorkspace instance. Note that we need to find ProdMonSource in the server
    source list, or we'll exit ingloriously.

    TODO: good mechanism for finding sources
    TODO: SiteDB source integration
    TODO: ProdRequest integration

    """
    _loginfo("PRODMON_W: Workspace created")
    CompWorkspace.__init__(self, gui, rank, category, 'prodmon', name, [ProdMonWorkspace.ProdMonSummary(self),ProdMonWorkspace.ProdMonPlot(self)])
    gui._addJSFragment("%s/javascript/Overview/ProdMon.js" % gui.contentpath)
    gui._addCSSFragment("%s/css/Overview/ProdMon.css" % gui.contentpath)
    self.source = None
    for s in gui.sources:
      if s.plothook=='prodmon':
        self.source = s
    if self.source==None:
      _logerr("PRODMON_W: Couldn't find ProdMonSource")
      raise Exception, "PW: Failed to find prodmon source. Fatal."

  def _state(self,session):
    return self._dostate(session)

  def _verify(self,key,value):
    timepattern = re.compile('^[0-9]{4}-[0-9]{1,2}-[0-9]{1,2} [0-9]{2}:[0-9]{2}:[0-9]{2}$')
    verifier = {
      'sortby': lambda x: x in ProdMonSource.params['sortby'],
      'what': lambda x: x in ProdMonSource.params['what'],
      'job_type': lambda x: x in ProdMonSource.params['job_type'],
      'graph': lambda x: x in ProdMonSource.params['plot'],
      'use_period': lambda x: x in ('true','false'),
      'period': lambda x: x.isdigit() and int(x)>0 and int(x)<=8760,
      'image_height': lambda x: x.isdigit() and int(x)>0 and int(x)<=2000,
      'image_width': lambda x: x.isdigit() and int(x)>0 and int(x)<=10000,
      'image_dpi': lambda x: x.isdigit() and int(x)>0 and int(x)<=600,
      'starttime': lambda x: timepattern.match(x),
      'endtime': lambda x: timepattern.match(x)
    }
    if key in verifier:
      if verifier[key](value):
        return value
      else:
        _logwarn("PRODMON_W: Verification failed for %s=%s"%(key,value))
        return deepcopy(ProdMonWorkspace.ProdMonPlot.plotdef[key])
    else:
      return value

  def _updateURLs(self,session):
    openplot =  session['prodmon.plot.open'][session['prodmon.plot.current']]
    if openplot['what']=='dts_timings':
      openplot['graph']='bar'
    if openplot['what']=='time_by_span':
      if openplot['graph']=='pie':
        openplot['graph']='bar'
    baseurl = '/plotfairy/prodmon/%s/%s/%s/%s/' % (openplot['what'],openplot['sortby'],openplot['graph'],openplot['job_type']) #already verified that none of these should contain special characters

    opts = {}
    if not (openplot['regex']=='' or openplot['regex']=='null'):
      opts['regex'] = urllib.quote(openplot['regex'])
    if not (openplot['group']=='' or openplot['group']=='null'):
      opts['group'] = urllib.quote(openplot['group'])
    for f in openplot['filters']:
      opts['filter_%s' % f[0]] = urllib.quote(f[1])
    if openplot['use_period']=='true':
      opts['period']=openplot['period']
    else:
      opts['starttime']=urllib.quote(openplot['starttime'])
      opts['endtime']=urllib.quote(openplot['endtime'])
    opts['width']=openplot['image_width']
    opts['height']=openplot['image_height']
    opts['dpi']=openplot['image_dpi']

    optstr = urllib.urlencode(opts)
    openplot['baseurl']=baseurl
    openplot['optstr']=optstr

    if openplot['title']==openplot['oldtitle']:
      openplot['title'] = self._composeTitle(openplot)
      openplot['oldtitle'] = copy(openplot['title'])


  def _composeTitle(self,openplot):
    return "%s/%s"%(ProdMonSource.params['what'][openplot['what']],ProdMonSource.params['sortby'][openplot['sortby']])

  def _formatTime(self,offset=0):
    return time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime(time.time()-offset))

  def sessionGraph(self,session,*args,**kwargs):
    """
    Update one of the parameters (what, job_type, start/endtime, regex, graph type)
    Add the new parameter set to the recent plots list.
    """
    for s in ('sortby','what','job_type','starttime','endtime','regex','graph','group','image_width','image_height','image_dpi','period','use_period','title'):
      if s in kwargs and kwargs[s]:
        value = self._verify(s,urllib.unquote(kwargs[s]))
        session['prodmon.plot.open'][session['prodmon.plot.current']][s]=value

    self._updateURLs(session)
    self.gui._saveSession(session)
    return self._state(session)

  def sessionFilters(self,session,*args,**kwargs):
    if 'name' in kwargs and kwargs['name']:
      name = urllib.unquote(kwargs['name'])
      if 'value' in kwargs and kwargs['value']:
        value = urllib.unquote(kwargs['value'])
        filters = []
        if '|' in name and '|' in value:
          for n,v in zip(name.split('|'),value.split('|')):
            filters.append([n,v])
        else:
          filters.append([name,value])
        session['prodmon.plot.open'][session['prodmon.plot.current']]['filters']=filters
    if 'clear' in kwargs:
      session['prodmon.plot.open'][session['prodmon.plot.current']]['filters']=[]
    self._updateURLs(session)
    self.gui._saveSession(session)
    return self._state(session)

  def sessionClose(self,session,*args,**kwargs):
    if 'index' in kwargs and kwargs['index'].isdigit():
      index = int(kwargs['index'])
    else:
      index = session['prodmon.plot.current']
    if index>=0 and index<len(session['prodmon.plot.open']):
      del session['prodmon.plot.open'][index]
    if len(session['prodmon.plot.open'])==0:
      session['prodmon.plot.open']=[deepcopy(ProdMonWorkspace.ProdMonPlot.plotdef)]
    session['prodmon.plot.current']=(session['prodmon.plot.current']+1)%len(session['prodmon.plot.open'])
    self._updateURLs(session)
    self.gui._saveSession(session)
    return self._state(session)

  def sessionAdd(self,session,*args,**kwargs):
    #add a new graph window
    if 'copy' in kwargs:
      copy = kwargs['copy']
    else:
      copy=False
    index=-1
    if 'index' in kwargs and kwargs['index'].isdigit():
      index=int(kwargs['index'])
      if not (index>=0 and index<len(session['prodmon.plot.open'])):
        index=-1
    if copy:
      newplot = deepcopy(session['prodmon.plot.open'][session['prodmon.plot.current']])
    else:
      newplot = deepcopy(ProdMonWorkspace.ProdMonPlot.plotdef)
    if index==-1:
      session['prodmon.plot.open'].append(newplot)
      session['prodmon.plot.current']=len(session['prodmon.plot.open'])
    else:
      session['prodmon.plot.open'].insert(index,newplot)
      session['prodmon.plot.current']=index
    self.gui._saveSession(session)
    return self._state(session)

  def sessionMove(self,session,*args,**kwargs):
    if 'index' in kwargs and kwargs['index'].isdigit():
      index = int(kwargs['index'])
      session['prodmon.plot.current']=index%len(session['prodmon.plot.open'])
    self.gui._saveSession(session)
    return self._state(session)


class ProdMonSource:
  url = 'http://dashb-cmspa.cern.ch/dashboard/request.py/PAquery?'
  plothook = "prodmon"
  params={}
  params['sortby']={'site':'Site','wf':'Workflow','dts':'Dataset','exit_code':'CMSSW Exit Code','prod_team':'Production Team','prod_agent':'ProdAgent'}
  params['what']={'evts_read_failed':'Events Read [Failed]','evts_read_success':'Events Read [Success]','evts_read_any':'Events Read','evts_written_failed':'Events Written [Failed]','evts_written_success':'Events written [Success]','evts_written_any':'Events Written','time_failed':'Time [Failed]','time_success':'Time [Success]','time_any':'Time','jobs_failed':'Jobs [Failed]','jobs_success':'Jobs [Success]','jobs_any':'Jobs','CMSSW_fail_codes':'CMSSW Failure Codes','time_by_span':'Approximate Batch Slot Usage','reco_rate':'Tier 0 RECO Rate','jobs_success_rates':'Job Success Rate','dts_timings':'Dataset Timings (Unnormalized)','jobs_failure_types':'Job Failure Types','wf_summary':'Workflow Summary Table'}
  params['plot'] = {'bar': 'Bar', 'pie':'Pie', 'cumulative':'Cumulative','baobab':'Baobab'}
  params['job_type'] = {'Merge': 'Merge', 'Any': 'Merge and Processing', 'Processing': 'Processing'}
  params['starttime']='Start Time'
  params['endtime']='End Time'
  filters={'site':'Site','wf':'Workflow','dts':'Dataset','exit_code':'CMSSW Exit Code','prod_team':'Production Team','prod_agent':'ProdAgent'}

  types={'pdf':'application/pdf','ps':'application/postscript','eps':'application/postscript','png':'image/png','svg':  'image/svg+xml','json':'text/plain'}
  national={'CH':'Switzerland','DE':'Germany','ES':'Spain','IT':'Italy','TW':'Taiwan','UK':'United Kingdom','US':'United States','AT':'Austria','BE':'Belgium','BR':'Brazil','CN':'China','EE':'Estonia','FI':'Finland','FR':'France','HU':'Hungary','IN':'India','KR':'Korea','PK':'Pakistan','PL':'Poland','PT':'Portugal','RU':'Russia','TR':'Turkey','UA':'Ukraine','CO':'Colombia','GR':'Greece'}
  regional={}
  groups={}
  groups['site']={'null':'None','tier':'Tier','country':'Country','tiercountry':'Tier, Country','top5site':'Top 5 Sites'}
  groups['wf']={'null':'None','1':'Primary','2':'Secondary','3':'Tertiary','top5wf':'Top 5 Workflows'}
  groups['dts']={'null':'None','generator':'Generator/Dataset','campaign':'Production Campaign','conditions':'Conditions','tiers':'Data Tiers','merged':'Merge Status'}
  groups['exit_code']={'null':'None','10k':'10k'}
  groups['prod_team']={}
  groups['prod_agent']={}
  def __init__(self,server,*args):
    _loginfo("PRODMON_S: Source created.")
    self.cache = [e for e in server.extensions if e.exthook=="OverviewCache"][0]
    self.cache.asyncCacheFill('PA_WFSUMMARY',self._getWFSummary)
  def plot(self,*path,**attrs):
    """
    Landing function for plotfairy requests.
    Format in *path should be what/sortby/plot/jobtype/datatype?starttime=XXX&endtime=...
    *path arguments are mandatory and this function should fail and return None,None if they are not found.
    **kwargs are all optional, defaults are provided in all cases.
    This function formats the query into a ProdMonQuery object, then passes it to GetPlotFromQuery.
    Defaults for missing keyword args are 48h plot, png image 800x600@72dpi (although these defaults are applied within ProdMonQuery)

    Also handles summary plots with paths of the form summary/{site,wf}/{plot,legend,map}
    An appropriate query object is created for these and then they are generated by the normal code path.
    """


    if len(path) == 3:
      if path[0] in ('summary') and path[1] in ('site','wf','team') and path[2] in ('plot','legend','map','plot+legend'): # Deprecated table output, now handled in state
        resulttype=path[2]

        if path[1] == 'site':
          query = ProdMonQuery('evts_read_any','site','pie','Any',type='png',width='400',height='300',group='top5site',period='48',notitle='1',nolabels='1')
        elif path[1] == 'wf':
          query = ProdMonQuery('evts_read_any','wf','pie','Any',type='png',width='400',height='300',group='top5wf',period='48',notitle='1',nolabels='1')
        elif path[1] == 'team':
          query =  ProdMonQuery('evts_read_any','prod_team','pie','Any',type='png',width='400',height='300',period='48',notitle='1',nolabels='1')
        else:
          _logwarn("PRODMON_S: Invalid summary requested: %s"%path)
          return None,None # Type of plot we don't understand
        return self._getPlotFromQuery(query,resulttype)

    elif len(path) == 5:
      query = ProdMonQuery(*path,**attrs)
      resulttype=path[4]
      return self._getPlotFromQuery(query,resulttype)

    elif len(path) >= 2:
      if path[0]=='autocomplete':
        ac_type = path[1]
        ac_text = '/'.join(path[2:])
        if ac_type in ('wf','dts','site','prod_team','prod_agent','exit_code'):
          return self._autoComplete(ac_type,ac_text)
        else:
          _logwarn("PRODMON_S: Invalid autocomplete requested: %s"%path)
          return None,None

    _logwarn("PRODMON_S: Bad path length: %s"%path)
    return None,None # Fall through if the path length didn't match 3 or 5, or the summary format was inconsistent

  def _getPlotFromQuery(self,query,resulttype):
    """
    GetPlotFromQuery evaluates a query object, testing it for validity before using the cache access functions to return the data.
    """
    if not (resulttype in ('plot','legend','map','plot+legend','thumbnail')): #or (query['plot']=='data' and resulttype=='table')):
      _logwarn("PRODMON_S: Unknown or inconsistent result type: %s"%resulttype)
      return None,None
    if not query.isValid():
      _logwarn("PRODMON_S: Invalid query.")
      return None,None

    plot = query['plot']
    if not plot in ProdMonSource.params['plot']:
      _logwarn("PRODMON_S: Unknown plot type: %s"%plot)
      return None,None

    plotdata = self.cache.lockedCacheFetch(query.cacheKey('PA_PLOT'),self._plotMaker,query)
    #_loginfo("PRODMON_S: Result plotsize=%s legendsize=%s mapsize=%s plot+legendsize=%s thumbnailsize=%s" %(len(plotdata[0]),len(plotdata[1]),len(plotdata[2]),len(plotdata[3]),len(plotdata[4])))
    if resulttype=='plot':
      return ProdMonSource.types[query['type']],plotdata[0]
    elif resulttype=='legend':
      return ProdMonSource.types[query['type']],plotdata[1]
    elif resulttype=='map':
      return 'text/plain',plotdata[2]
    elif resulttype=='plot+legend':
      return ProdMonSource.types[query['type']],plotdata[3]
    elif resulttype=='thumbnail':
      return ProdMonSource.types[query['type']],plotdata[4]
    else:
      pass # Already validated that we have one of the above

  def _plotMaker(self,query):
    """
    PlotMaker is a cache maker function which returns the appropriate plot,legend,map tuple if they aren't found in cache.
    """
    _loginfo("PRODMON_S: _plotMaker: %s"%query.cacheKey('PA_PLOT'))
    plot = query['plot']
    if plot=='pie':
      plotdata = self._plotPie(query)
    elif plot=='bar':
      plotdata = self._plotBar(query)
    elif plot=='cumulative':
      plotdata = self._plotCumulative(query)
    elif plot=='baobab':
      plotdata = self._plotBaobab(query)
    else:
      pass # Plot type validity has already been checked
    plotdata = plotdata[0],plotdata[1],plotdata[2],stitchPlotAndLegend(plotdata[0],plotdata[1]),makeThumbnail(plotdata[0])
    return plotdata,1800

  def summaryList(self,which,n=5):
    if which=='site':
      return self.cache.cacheFetch("PA_SITETOP%s"%(n),self._getSiteTop,n)
    elif which=='wf':
      return self.cache.cacheFetch("PA_WFTOP%s"%(n),self._getWFTop,n)
    return None

  def _getFilterGroup(self,query):
    filterer = lambda x: True
    if 'regex' in query and not query['regex']=='':
      filterer = self._regexFilter(query['regex'])

    group = lambda x: x
    if 'group' in query:
      if query['group'] in ProdMonSource.groups[query['sortby']]:
        if query['sortby']=='site':
          group = self._groupSite(query['group'])
        if query['sortby']=='wf':
          group = self._groupWF(query['group'])
        if query['sortby']=='dts':
          group = self._groupDTS(query['group'])
        if query['sortby']=='exit_code':
          group = self._groupCode(query['group'])
        if query['sortby']=='prod_agent':
          group = self._groupAgent(query['group'])
        if query['sortby']=='prod_team':
          group = self._groupTeam(query['group'])
    return filterer,group

  def _generateMapURL(self,query):
    """
    This is hook to provide deeper behaviour when a graph is clicked on. The default implementation just goes to a filtered version of the current plot, but it would be possible to add any sort of behaviour here.
    """
    base_query = {'what':query['what'],
                  'sortby':query['sortby'],
                  'job_type':query['job_type'],
                  'graph':query['plot'],
                  'name':'plot'
                 }
    if 'group' in query:
      base_query['group']=query['group']
    if 'period' in query:
      base_query['use_period']='true'
      base_query['period']=query['period']
    else:
      base_query['use_period']='false'
      base_query['starttime']=query['starttime']
      base_query['endtime']=query['endtime']
    base_query['filter_name']=query['sortby']
    return 'view?'+urllib.urlencode(base_query)+'&filter_value=$n'


  def _plotBaobab(self,query):

    baobab_site_grouper = lambda item: item.split('_',2)
    baobab_wf_grouper = lambda item: item.split('-',2)
    baobab_dts_grouper = lambda item: item[1:].split('/',2)
    baobab_other_grouper = lambda item: [item]

    baobab_group = baobab_other_grouper
    if query['sortby']=='site':
      baobab_group = baobab_site_grouper
    if query['sortby']=='wf':
      baobab_group = baobab_wf_grouper
    if query['sortby']=='dts':
      baobab_group = baobab_dts_grouper

    plotkey = query.cacheKey("PA_PLOT")
    result = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)

    filterer,group = self._getFilterGroup(query)

    names=[]
    data={}
    hierarchy={}

    # Only need to handle name,value for pie data, epoch not included
    for item in result:
      self._fixItem(query,item)
      name = item['NAME']
      if filterer(name):
        value=float(item['VALUE'])
        if value>0.:
          if not name in names:
            names.append(name)
            hierarchy[name]=baobab_group(name)
          if not name in data:
            data[name]=value
          else:
            data[name]+=value

    def get_named_dict(name,dictlist):
      for d in dictlist:
        if d['name']==name:
          return d
      return None

    baobab = {'name':'ROOT','value':0.,'children':[]}
    for name in data:
      parent=baobab
      parent['value']+=data[name]
      for i in hierarchy[name]:
        child = get_named_dict(i,parent['children'])
        if child:
          child['value']+=data[name]
          parent = child
        else:
          parent['children'].append({'name':i,'value':data[name],'children':[]})
          parent = get_named_dict(i,parent['children'])

    if query['type']=='json':
      json = "[%r]" % (baobab)
      return json,json,json

    legend = None

    plot = BaobabChart(baobab,legend,{
            'imageType':query['type'],
            'height':query['height'],
            'width':query['width'],
            'dpi':query['dpi'],
            'title': query.plotTitle(),
            'starttime':query._stampToTime(query['starttime']),
            'endtime':query._stampToTime(query['endtime']),
            'integral':True,
            'notitle':query['notitle'],
            'nolabels':query['nolabels'],
            'sort':True,
            'mapName':'mainplot',
            'map_href':'#',
            #'map_onmouseover':"return GUI.Plugin.ProdMonGraphBase.handleMapMouse('%s','$n','$v')" % query['sortby'],
            'map_onclick':"return GUI.Plugin.ProdMonPlot.handleMapClick('%s')" % self._generateMapURL(query),
            'map_title':'$n: $v' # A note on tooltips. From Netscape 6, gecko based browsers ignore the alt-tag for tooltip and only use the title attribute. MSIE draws either, with alt taking precedence. HTML standard technically requires alt, but this will only increase map size. Including an onmouseover trigger suppresses either from displaying (this could be used for something more elaborate, eg YUI tooltips, but currently isn't).
            })
    #print "making plot"
    imgdata,legenddata,mapdata = plot.make()
    #print "returning plot"
    return imgdata,legenddata,mapdata

  def _plotPie(self,query):

    plotkey = query.cacheKey("PA_PLOT")
    result = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)

    filterer,group = self._getFilterGroup(query)

    names=[]
    data={}

    # Only need to handle name,value for pie data, epoch not included
    for item in result:
      self._fixItem(query,item)
      name = item['NAME']
      if filterer(name):
        value=float(item['VALUE'])
        if value>0.:
          name = group(name)
          if not name in names:
            names.append(name)
          if not name in data:
            data[name]=value
          else:
            data[name]+=value

    if query['type']=='json':
      json = "[%r]" % (data)
      return json,json,json

    legend = self._getLegend(names)

    plot = PieChart(data,legend,{
            'imageType':query['type'],
            'height':query['height'],
            'width':query['width'],
            'dpi':query['dpi'],
            'title': query.plotTitle(),
            'starttime':query._stampToTime(query['starttime']),
            'endtime':query._stampToTime(query['endtime']),
            'integral':True,
            'notitle':query['notitle'],
            'nolabels':query['nolabels'],
            'sort':True,
            'mapName':'mainplot',
            'map_href':'#',
            #'map_onmouseover':"return GUI.Plugin.ProdMonGraphBase.handleMapMouse('%s','$n','$v')" % query['sortby'],
            'map_onclick':"return GUI.Plugin.ProdMonPlot.handleMapClick('%s')" % self._generateMapURL(query),
            'map_title':'$n: $v' # A note on tooltips. From Netscape 6, gecko based browsers ignore the alt-tag for tooltip and only use the title attribute. MSIE draws either, with alt taking precedence. HTML standard technically requires alt, but this will only increase map size. Including an onmouseover trigger suppresses either from displaying (this could be used for something more elaborate, eg YUI tooltips, but currently isn't).
            })
    imgdata,legenddata,mapdata = plot.make()
    return imgdata,legenddata,mapdata

  def _getSeries(self,result,interval):
    epochset=set()
    for item in result:
      if 'EPOCH' in item:
        epochset.add(int(item['EPOCH']))
      else:
        epochset.add(0)
        item['EPOCH']=0
    if len(epochset)>0:
      first = min(epochset)
      last = max(epochset)
      series = []
      now = first
      while (now<=last):
        series.append((now,now+interval))
        now += interval
      return series
    else:
      return None

  def _getLegend(self,names):
    colors = [ "#7D818C", "#007092", "#75B3CE", "#B40019", "#FFDC8C", "#F79025",
         "#BBBEC5", "#231F20", "#e66266", "#fff8a9", "#7bea81", "#8d4dff",
         "#ffbc71", "#a57e81", "#baceac", "#00ccff", "#ccffff", "#ff99cc",
         "#cc99ff", "#ffcc99", "#3366ff", "#33cccc", "#ffffff", "#bbbbbb",
         "#777777", "#333333" ]
    legend = []
    for i,n in enumerate(names):
      legend.append({'key':n,'name':n,'attrs':{'facecolor':colors[i % len(colors)]}})
    return legend

  def _fixItem(self,query,item):
    if query['what']=='jobs_success_rates':
      item['VALUE']=float(item['SUCCESS_JOBS'])/(float(item['FAILED_JOBS'])+float(item['SUCCESS_JOBS']))
    if query['what']=='dts_timings':
      item['VALUE']=float(item['AVERAGE_S_PER_EVT'])

  def _plotBar(self,query):

    result = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)

    filterer,group = self._getFilterGroup(query)

    #series - ((start,end),(start2,end2)...)
    #data - ({name:value,name2:value2},{...},...)
    span,interval = query.getInterval()
    series=self._getSeries(result,interval)

    if series:

      names=[]
      epochindex={}
      data=[]
      for i,s in enumerate(series):
        epochindex[s[0]]=i
        data.append({})

      for item in result:
        self._fixItem(query,item)
        name = item['NAME']
        if filterer(name):
          epoch = int(item['EPOCH'])
          value = float(item['VALUE'])
          if value>0.:
            name = group(name)
            if not name in names:
              names.append(name)
            if not name in data[epochindex[epoch]]:
              data[epochindex[epoch]][name]=value
            else:
              data[epochindex[epoch]][name]+=value

      #this is an ugly hack to ensure there is a data item known to the legend in each epoch
      #otherwise we get a daft blank graph
      if len(names)>=1:
        for d in data:
          if len(d)==0:
            d[names[0]]=0

    else:
      starttime = query._stampToTime(query['starttime'])
      endtime = query._stampToTime(query['endtime'])
      periods = int((endtime-starttime)/interval)
      series = [(starttime+n*interval,starttime+(n+1)*interval) for n in range(periods)]
      data=[{} for n in range(periods)]
      names=[]

    if query['type']=='json':
      json = "[%r,%r]" % (series,data)
      return json,json,json

    legend = self._getLegend(names)

    plot = TimeSeriesBarChart(span, series, data, legend, {
                  'imageType':query['type'],
                  'height':query['height'],
                  'width':query['width'],
                  'dpi':query['dpi'],
                  'title': query.plotTitle(),
                  'notitle':query['notitle'],
                  'nolabels':query['nolabels'],
                  'yAxisTitle': ProdMonSource.params['what'][query['what']],
                  'integral':True,
                  'sort':True,
                  'mapName':'mainplot',
                  'map_href':'#',
                  'map_title':'$n: $v for $b-$e',
                  'map_onclick':"return GUI.Plugin.ProdMonPlot.handleMapClick('%s')" % self._generateMapURL(query),
                  #'map_onmouseover':"return GUI.Plugin.ProdMonGraphBase.handleMapMouse('%s','$n','$v')" % query['sortby']
                  })
    imgdata,legenddata,mapdata = plot.make()
    return imgdata,legenddata,mapdata


  def _plotCumulative(self,query):

    result = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)

    filterer,group = self._getFilterGroup(query)

    #series - ((start,end),(start2,end2)...)
    #data - ({name:value,name2:value2},{...},...)
    span,interval = query.getInterval()
    series=self._getSeries(result,interval)

    if series:
      names=[]
      epochindex={}
      data=[]
      for i,s in enumerate(series):
        epochindex[s[0]]=i
        data.append({})

      for item in result:
        self._fixItem(query,item)
        name = item['NAME']
        if filterer(name):
          epoch = int(item['EPOCH'])
          value = float(item['VALUE'])
          if value>0.:
            name = group(name)
            if not name in names:
              names.append(name)
            if not name in data[epochindex[epoch]]:
              data[epochindex[epoch]][name]=value
            else:
              data[epochindex[epoch]][name]+=value

      #this is an ugly hack to ensure there is a data item known to the legend in each epoch
      #otherwise we get a daft blank graph
      #who knows why?
      if len(names)>=1:
        for d in data:
          if len(d)==0:
            d[names[0]]=0

    else:
      starttime = query._stampToTime(query['starttime'])
      endtime = query._stampToTime(query['endtime'])
      periods = int((endtime-starttime)/interval)
      series = [(starttime+n*interval,starttime+(n+1)*interval) for n in range(periods)]
      data=[{} for n in range(periods)]
      names=[]

    if query['type']=='json':
      json = "[%r,%r]" % (series,data)
      return json,json,json

    legend = self._getLegend(names)

    plot = CumulativeTimeSeriesChart(span, series, data, legend, {
                  'imageType':query['type'],
                  'height':query['height'],
                  'width':query['width'],
                  'dpi':query['dpi'],
                  'title': query.plotTitle(),
                  'yAxisTitle': ProdMonSource.params['what'][query['what']],
                  'integral':True,
                  'notitle':query['notitle'],
                  'nolabels':query['nolabels'],
                  'sort':True,
                  'mapName':'mainplot',
                  'map_href':'#',
                  'map_title':'$n: $v',
                  'map_onclick':"return GUI.Plugin.ProdMonPlot.handleMapClick('%s')" % self._generateMapURL(query),
                  #'map_onmouseover':"return GUI.Plugin.ProdMonGraphBase.handleMapMouse('%s','$n','$v')" % query['sortby']
                  })
    imgdata,legenddata,mapdata = plot.make()
    return imgdata,legenddata,mapdata

  def _getResult(self,query):
    """
    GetResult is a cachemaker function which actually gets data from the backend. HTTP errors are not actually handled here (logged and re-raised) since we don't want dead data to get cached - better to end up with HTTP 500 instead
    """
    urlquery = query.getURL()
    try:
      url = "%s%s" % (ProdMonSource.url,urlquery)
      data = urllib2.urlopen(url).read()
    except urllib2.HTTPError,e:
      _logwarn("PRODMON_S: HTTPError code=%s url=%s"%(e.code,url))
      raise
    except urllib2.URLError,e:
      _logwarn("PRODMON_S: URLError reason=%s url=%s"%(e.reason,url))
      raise
    if ("Exception" in data.split("\n",1)[0]):
      _logwarn("PRODMON_S: Dashboard returned exception url=%s"%url)
      raise IOError, "Dashboard returned error %s in query %s" % (data.split("\n",1)[0],self.urlquery)
    result = ProdMonQueryResult(data)
    return result,1200

  def _getWFList(self):
    """
    Get the list of all known workflows from the wf_summary table.
    Updated _getWFList for use with _fetchCache
    """
    query = ProdMonQuery('wf_summary','wf','data','Any')
    qresult = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)
    result = []
    for item in qresult:
      result.append(copy(item['WORKFLOW'].encode('ascii')))
    return result,1800

  def getStatsSummary(self):
    if "PA_WFSUMMARY" in self.cache:
      table = self.cache.cacheFetch('PA_WFSUMMARY',self._getWFSummary)
      result = {}
      result['workflow_count']=len(table)
      result['total_unmerged']=0
      result['total_merged']=0
      time_per_event = []
      for t in table:
        result['total_unmerged']+=int(table[t]['UNMERGED_EVTS'])
        result['total_merged']+=int(table[t]['MERGED_EVTS'])
        time_per_event.append(float(table[t]['TIME_PER_EVENT']))
      if len(time_per_event)>0:
        result['avg_time_per_event']='%.2f'%(sum(time_per_event)/len(time_per_event))
        result['min_time_per_event']='%.2f'%min(time_per_event)
        result['max_time_per_event']='%.2f'%max(time_per_event)
      else:
        _logwarn("PRODMON_S: WFSummary table contains no workflows")
        result['avg_time_per_event']='No workflows found'
        result['min_time_per_event']='No workflows found'
        result['max_time_per_event']='No workflows found'
      return result
    else:
      self.cache.asyncCacheFill('PA_WFSUMMARY',self._getWFSummary)
      return {
        'workflow_count':'Data Unavailable',
        'total_unmerged':'Data Unavailable',
        'total_merged':'Data Unavailable',
        'avg_time_per_event':'Data Unavailable',
        'max_time_per_event':'Data Unavailable',
        'min_time_per_event':'Data Unavailable'
      }

  def _getWFSummary(self):
    """
    GetWFSummary fetches the special-case wf summary table, and parses it into a dictionary.
    """
    query = ProdMonQuery('wf_summary','wf','data','Any')
    qresult = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)
    result = {}
    for item in qresult:
      itemresult={}
      wf = copy(item['WORKFLOW'])
      for attrib in item:
        itemresult[attrib] = copy(item[attrib])
      result[wf]=itemresult
    return result,1800

  def _getWFTop(self,count,sortkey='MERGED_EVTS'):
    wfsummary = self.cache.cacheFetch('PA_WFSUMMARY',self._getWFSummary)

    wflist = wfsummary.keys()
    wflist.sort(key=lambda x: int(wfsummary[x][sortkey]))
    wflist.reverse()
    wflist = wflist[0:count]
    return wflist,1800

  def _getSiteTop(self,count):
    query = ProdMonQuery('evts_read_any','site','pie','Any')
    result = self.cache.lockedCacheFetch(query.cacheKey('PA_QUERY',False),self._getResult,query)

    sites = [(item['NAME'],int(item['VALUE'])) for item in result]
    sites.sort(key=lambda x: x[1])
    sites.reverse()

    sitelist = [s[0] for s in sites[0:count]]

    return sitelist,1800

  def getWFList(self,wffilter=None):
    if not wffilter:
      wffilter = lambda x: True
    wflist = self.cache.cacheFetch("PA_WFLIST",self._getWFList)
    return filter(wffilter,wflist)


  def _regexFilter(self,regex):
    class _re_filter:
      def __init__(self,regex):
        try:
          self.regex = re.compile(regex)
        except re.error:
          self.regex = None
      def __call__(self,x):
        if self.regex:
          try:
            return self.regex.search(x)!=None
          except re.error:
            return False
        return False
    return _re_filter(regex)


  def _groupSite(self,mode):
    """
    GroupSite is a factory method that returns a callable() object which groups site names according to the selected mode.

    TODO: Regional doesn't work yet.
    TODO: Many of these functions should be delegated to sitedbsource.
    """

    class site_grouper:

      grouping={}
      grouping['tier']=lambda x: x.split('_')[0]
      grouping['country']=lambda x: self.national.get(x.split('_')[1],x.split('_')[1])
      grouping['tiercountry']=lambda x: "%s %s" % (x.split('_')[0], self.national.get(x.split('_')[1],x.split('_')[1]))
      def __init__(self,mode):
        self.function = self.grouping[mode]
      def __call__(self,site):
        try:
          return self.function(site)
        except IndexError:
          return site

    class top5_group_site:
      def __init__(self,toplist):
        self.toplist=toplist
      def __call__(self,site):
        if site in self.toplist:
          return site
        else:
          return 'Other'

    if mode in site_grouper.grouping:
      return site_grouper(mode)
    else:
      if mode=='top5site':
        return top5_group_site(self.cache.cacheFetch("PA_SITETOP5",self._getSiteTop,5))
    return lambda x: x


  def _groupWF(self,mode):
    """
    GroupWF is a factory method that returns a callable for workflow grouping.
    """
    class count_group_wf:
      separator='-'
      def __init__(self,count):
        self.count = count
      def __call__(self,wf):
        if wf.count(self.separator)>=self.count:
          parts = wf.split(self.separator)
          return self.separator.join(parts[0:self.count])
        else:
          return wf
    class top5_group_wf:
      def __init__(self,toplist):
        self.toplist=toplist
      def __call__(self,wf):
        if wf in self.toplist:
          return wf
        else:
          return 'Other'

    if mode.isdigit():
      mode = int(mode)
      return count_group_wf(mode)
    else:
      if mode=='top5wf':
        return top5_group_wf(self.cache.cacheFetch("PA_WFTOP5",self._getWFTop,5))
    return lambda x: x

  def _groupDTS(self,mode):
    class dts_grouper:
      def merged(x):
        if 'unmerged' in x:
          return 'UnMerged'
        else:
          return 'Merged'
      grouping={}
      grouping['generator']=lambda x: x.split('/')[1]
      grouping['campaign']=lambda x: x.split('/')[2].split('_')[0]
      grouping['conditions']=lambda x: x.split('/')[2].split('_')[1]
      grouping['tiers']=lambda x: x.split('/')[3]
      grouping['merged']=merged
      def __init__(self,mode):
        self.function = self.grouping[mode]

      def __call__(self,x):
        try:
          return self.function(x)
        except IndexError:
          return site
    return dts_grouper(mode)

  def _groupCode(self,mode):
    if mode=='10k':
      def k10(x):
        if x.isdigit():
          return "%sxxxx" % int(int(x)/10000)
        return x
      return k10
    return lambda x:x

  def _groupAgent(self,mode):
    return lambda x:x

  def _groupTeam(self,mode):
    return lambda x:x

  def _autoComplete(self,mode,text):
    return 'text/plain','([])'

class ProdMonQuery:
  """
  Wrapper class for a query to the backend system. Turns it into a query string, adds timestamps if none exists and checks for validity.
  """
  timedelta = 3600 #all time variables are set to previous 30 minute interval, to make them cacheable (a request for 12:47:19 is converted to 12:30:00 etc)
  def __init__(self,what,sortby,plot,job_type,*args,**attrs):
    """
    Create a new Query from the path (as passed to ProdMonSource.plot) and attributes (from which we're only interested in time keys).
    Time variables are rounded to the nearest timedelta (30 minutes) - data isn't available at shorter timespans than this and otherwise no data is going to get cached.
    """
    self.query = {'what':what, 'sortby':sortby,'plot':plot,'job_type':job_type}
    #if no endtime is set, set it to now (with timedelta)
    if 'starttime' in attrs and 'endtime' in attrs:
      starttime = self._stampToTime(urllib.unquote(attrs['starttime']))
      endtime = self._stampToTime(urllib.unquote(attrs['endtime']))
      if not (starttime<endtime and endtime<time.time()):
        starttime = time.time()-172800
        endtime = time.time()
    elif 'period' in attrs:
      period = urllib.unquote(attrs['period'])
      if period.isdigit():
        period = int(period)
        starttime = time.time()-period*3600
        endtime = time.time()
      else:
        starttime = time.time()-172800
        endtime = time.time()
    else:
      starttime = time.time()-172800
      endtime = time.time()

    starttime = starttime - (starttime % ProdMonQuery.timedelta)
    endtime = endtime - (endtime % ProdMonQuery.timedelta)
    self.query['starttime']=self._timeToStamp(starttime)
    self.query['endtime']=self._timeToStamp(endtime)

    #these are known parameters which need to be considered in the plot cache key
    for s in ('filter_site','filter_wf','filter_dts','filter_prod_team','filter_prod_agent','filter_exit_code','regex','group','width','height','dpi','type','period','notitle','nolabels'):
      if s in attrs and not (attrs[s]=='' or attrs[s]=='null'):
        self.query[s]=urllib.unquote(copy(attrs[s]))

    #set some defaults for optional image arguments
    if not 'height' in self.query:
      self.query['height']='600'
    if not 'width' in self.query:
      self.query['width']='800'
    if not 'dpi' in self.query:
      self.query['dpi']='72'
    if not 'type' in self.query:
      self.query['type']='png'
    else:
      if not self.query['type'] in ProdMonSource.types:
        self.query['type']='png'
    if not 'notitle' in self.query:
      self.query['notitle']=False
    if not 'nolabels' in self.query:
      self.query['nolabels']=False

  def getURL(self):
    """
    Return the query encoded to a url, ready to be appended to the source URL+?
    """
    #parameters understood by PAQuery
    #urlparam=('what','sortby','plot','job_type','starttime','endtime','span','site','wf','dts','prod_team')
    toencode={}
    for q in self.query:
      if q in ProdMonSource.params:
        toencode[q]=self.query[q]
      if q.startswith('filter_'):
        if q[7:] in ProdMonSource.filters:
          toencode[q[7:]]=self.query[q]

    if self.query['plot']=='baobab':
      toencode['plot']='pie'
    return urllib.urlencode(toencode)

  def isValid(self):
    """
    Test if the query is valid. Tests that all the mandatory parameters are filled (or the wf_summary special case), and that the start and end times are properly set and in the correct order.
    """

    matched = 0
    for key in self.query:
      if key in ProdMonSource.params:
        if self.query[key] in ProdMonSource.params[key]:
          matched += 1
    if matched==4 or (matched==2 and self.query['what']=='wf_summary' and self.query['plot']=='data'):
      if 'starttime' in self.query and 'endtime' in self.query:
        try:
          starttime = self._stampToTime(self.query['starttime'])
          endtime = self._stampToTime(self.query['endtime'])
        except ValueError:
          return False
        if starttime<=endtime and endtime<=time.time():
          return True
      else:
        return False
    else:
      return False

  def cacheKey(self,prefix='',full=True):
    """
    Return this query as a key to use for the backing cache.

    Two forms available - short key containing just the query parameters (full=False) for use in caching the query, and the long form (full=True) containing the image parameters and post-query options like regexes.
    """
    base = "%s|%s|%s|%s|%s|%s" % (self.query['what'],self.query['sortby'],self.query['plot'],self.query['job_type'],self.query['starttime'],self.query['endtime'])
    filters = ["|f:%s:%s"%(f[7:],self.query[f]) for f in self.query.keys() if f[0:6]=='filter']
    base+='|'.join(filters)
    image=""
    if 'width' in self.query and 'height' in self.query and 'dpi' in self.query and 'type' in self.query:
      image = "|%s|%sx%sx%s" % (self['type'],self['width'],self['height'],self['dpi'])
    extra = ""
    if 'regex' in self.query:
      extra += "|regex:%s" % (self['regex'])
    if 'group' in self.query:
      extra += "|group:%s" % (self['group'])
    if full:
      return "%s@%s%s%s" % (prefix,base,image,extra)
    else:
      return "%s@%s" % (prefix,base)


  def _timeToStamp(self,time):
    """
    Convert a seconds since epoch time number into the query timestamp format.
    TODO: check timezone behaviour of this
    """
    dt = datetime.datetime.fromtimestamp(time)
    return "%s-%s-%s %s:%s:%s" % (dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second)
  def _stampToTime(self,stamp):
    """
    Convert a query timestamp into seconds since the epoch.
    TODO: check timezone behaviour of this
    """
    ts = time.mktime(time.strptime(stamp,"%Y-%m-%d %H:%M:%S"))
    return ts
  def __getitem__(self,key):
    """
    Direct access to the query keys. Don't you just love python operator overriding?
    """
    return self.query[key]

  def plotTitle(self):
    """
    Get a formatted plot title based on this query.
    """
    start = "%s (%s jobs)\n" % (ProdMonSource.params['what'][self['what']],ProdMonSource.params['job_type'][self['job_type']])
    sort = "sorted by %s" % (ProdMonSource.params['sortby'][self['sortby']])
    group = ""
    if 'group' in self.query and self.query['group'] in ProdMonSource.groups[self['sortby']]:
      group = "grouped by %s" % (ProdMonSource.groups[self['sortby']][self['group']])
    regex=""
    if 'regex' in self.query and not (self['regex']=='' or self['regex']=='null'):
      regex = "matching %s" % self['regex']

    filters = ["%s~%s"%(f[7:],self.query[f]) for f in self.query.keys() if f[0:6]=='filter']
    return " ".join([start,sort,group,regex]+filters)

  def getInterval(self):
    """
    Get the time binning interval for this data. This should probably be done by examination of the data,
    but as it currently stands short of compiling a histogram this is impossible and it will instead be
    inferred on the basis of 1-day intervals for >168 hours and 1-hour intervals otherwise
    """
    start = self._stampToTime(self['starttime'])
    end = self._stampToTime(self['endtime'])
    if end-start >= 604800:
      return 'day',86400
    else:
      return 'hour',3600



  def __iter__(self):
    return self.query.__iter__()


class ProdMonQueryResult:
  """
  Parses the raw data from an xml request (using DOM) and provides an iterator over the items it returns.
  """
  def __init__(self,data):
    """
    Create a new Query result from raw XML data
    """
    dom = parseString(data)
    self.items=[]
    for item in dom.getElementsByTagName("item"):
      result={}
      for c in item.childNodes:
        tagname = copy(c.tagName)
        if c.firstChild:
          child = copy(c.firstChild.nodeValue.encode('ascii'))
          result[tagname]=child
        else:
          result[tagname]=''
      self.items.append(result)
    dom.unlink()

  def __getitem__(self,i):
    """
    Get an indexed item, returning a dictionary of the named sub-keys and their values.
    """
    return self.items[i]
  def __iter__(self):
    """
    Return a QueryIterator over this result.
    """
    return self.items.__iter__()
  def __len__(self):
    """
    Get the number of items this result contains.
    """
    return len(self.items)

  def __str__(self):
    return str(self.items)

  def __repr__(self):
    return repr(self.items)


