#!/usr/bin/env python

from Monitoring.Core.Plot import *
from Monitoring.Core.Utils import _logerr, _logwarn, _loginfo
from Monitoring.Overview.GUI import CompWorkspace
from copy import deepcopy, copy
import time, datetime
import logging
import random
import urllib
import socket
import re
import json

class FileLightWorkspace(CompWorkspace):
  sessiondef = {'filelight.view':'site'}
  class FileLightView:
    viewdef = {}
    def __init__(self,workspace):
      self.workspace = workspace
    def state(self,session):
      result = {'kind':self.pluginname}
      result.update(self._stateHook(session))
      return json.dumps(result)
    def initialiseSession(self, session, *args, **kwargs):
      for key,value in self.viewdef.iteritems():
        if key not in session:
          session['filelight.%s.%s'%(self.label,key)]=copy(value)

  class FileLightSiteView(FileLightView):
    label='site'
    pluginname='FileLightSite'
    name='Site'

    viewdef={
      'group':0,
      'height':1200,
      'width':1200,
      'type':'png',
      'site':'null',
      'incomplete':0,
      'custodial':0,
      'subscribed':0,
      'filter':'null',
      'scheme':'prod:cond:tiers:dataset'
    }

    def _stateHook(self,session):
      return {
        'height':session['filelight.site.height'],
        'width':session['filelight.site.width'],
        'type':session['filelight.site.type'],
        'site':session['filelight.site.site'],
        'incomplete':session['filelight.site.incomplete'],
        'custodial':session['filelight.site.custodial'],
        'subscribed':session['filelight.site.subscribed'],
        'filter':session['filelight.site.filter'],
        'scheme':session['filelight.site.scheme'],
        'sites':self.workspace.source.nodeNames()
      }

    def validate(self,key,value):
      return value


  class FileLightGroupView(FileLightView):
    label='group'
    pluginname='FileLightGroup'
    name='Group'

    viewdef={
      'height':800,
      'width':800,
      'type':'png',
      'group':'null'
    }

    def _stateHook(self,session):
      return {
        'height':session['filelight.group.height'],
        'width':session['filelight.group.width'],
        'type':session['filelight.group.type'],
        'group':session['filelight.group.group'],
        'groups':self.workspace.source.groupNames()
      }

    def validate(self,key,value):
      return value

  def __init__(self,gui,rank,category,name,*args):
    _loginfo("FILELIGHT_W: Workspace created")
    CompWorkspace.__init__(self, gui, rank, category, 'filelight', name, [self.FileLightSiteView(self),self.FileLightGroupView(self)])
    gui._addJSFragment("%s/javascript/Overview/FileLight.js" % gui.contentpath)
    self.source = None
    for s in gui.sources:
      if s.plothook=='filelight':
        self.source = s
    if self.source==None:
      _logerr("FILELIGHT_W: Couldn't find FileLightSource")
      raise Exception, "FILELIGHT_W Couldn't find source"

  def _state(self,session):
    return self._dostate(session)

  def sessionOpt(self,session,*args,**kwargs):
    viewstr = session['filelight.view']
    view = self._getView(viewstr)
    for key,value in kwargs.items():
      if key in view.viewdef:
        value = view.validate(key,value)
        session['filelight.%s.%s'%(viewstr,key)]=value
    self.gui._saveSession(session)
    return self._state(session)

class FileLightSource(object):
  plothook = 'filelight'
  types={'pdf':'application/pdf','ps':'application/postscript','eps':'application/postscript','png':'image/png','svg':  'image/svg+xml','json':'text/plain'}

  def __init__(self,server,*args):
    _loginfo("FILELIGHT_S: Source created.")
    self.cache = [e for e in server.extensions if e.exthook=="OverviewCache"][0]

    self.cache.asyncCacheFill('FL_NODENAMES',self._nodeNames)
    self.cache.asyncCacheFill('FL_GROUPNAMES',self._groupNames)

  def plot(self,*path,**attrs):
    #_loginfo('FL_S: plot %s %s'%(path,attrs))
    if len(path)==2:
      what = path[0]
      which = path[1]

      if not 'type' in attrs:
        attrs['type']='png'
      elif not attrs['type'] in self.types:
        attrs['type']='png'

      if not 'width' in attrs:
        attrs['width']=1000
      elif not attrs['width'].isdigit():
        attrs['width']=1000
      else:
        attrs['width']=int(attrs['width'])

      if not 'height' in attrs:
        attrs['height']=1000
      elif not attrs['height'].isdigit():
        attrs['height']=1000
      else:
        attrs['height']=int(attrs['height'])

      if what=='site':
        if not 'custodial' in attrs:
          attrs['custodial'] = False
        if not 'incomplete' in attrs:
          attrs['incomplete'] = False
        if not 'subscribed' in attrs:
          attrs['subscribed'] = False
        if not 'filter' in attrs:
          attrs['filter'] = False
        if 'scheme' in attrs:
          attrs['scheme'] = attrs['scheme'].lower()
        else:
          attrs['scheme'] = False
        return self.types[attrs['type']],self.cache.lockedCacheFetch('FL_NODEPLOT:%s:%s:%sx%s:%s:%s:%s:%s:%s'%(which,attrs['type'],attrs['width'],attrs['height'],attrs['custodial'],attrs['incomplete'], attrs['subscribed'],attrs['filter'],attrs['scheme']),self._makeSitePlot,which,attrs)
      if what=='group':
        return self.types[attrs['type']],self.cache.lockedCacheFetch('FL_GROUPPLOT:%s:%s:%sx%s'%(which,attrs['type'],attrs['width'],attrs['height']),self._makeGroupPlot,which,attrs)

    return None,None

  def _get_named_dict(self,name,dictlist):
    for d in dictlist:
      if d['name']==name:
        return d
    return None

  def _makeSitePlot(self,site,attrs):
    #_loginfo('FL_S: makeSitePlot start %s'%site)
    blocks = self.cache.lockedCacheFetch('FL_NODE:%s'%site,self._nodeData,site)
    #_loginfo('FL_S: makeSitePlot got %s blocks for %s'%(len(blocks),site))


    transform = self.NodeTransform(attrs)

    baodata = {'name':'ROOT','value':0,'children':[]}
    for name,size in transform(blocks).items():
      parent = baodata
      parent['value'] += size
      for nn in name:
        child = self._get_named_dict(nn,parent['children'])
        if child:
          child['value']+=size
          parent = child
        else:
          parent['children'].append({'name':nn,'value':size,'children':[]})
          parent = self._get_named_dict(nn,parent['children'])

    plot = BaobabChart(baodata,None,{
      'imageType':attrs['type'],
      'height':attrs['height'],
      'width':attrs['width'],
      'title':'%s\n%s'%(site,time.strftime('%H:%M %d/%m/%Y',time.gmtime())),
      'dpi':72,
      'unit':'B'
      })
    imgdata = plot.draw().read()
    #_loginfo('FL_S: makeSitePlot returns %s'%site)
    return imgdata,3600


  def _makeGroupPlot(self,group,attrs):
    sizes = self.cache.lockedCacheFetch('FL_GROUP:%s'%group,self._groupData,group)

    def site_name_splitter(name):
      match = re.match(r'(T\d)_([A-Z]+)_(\w+)',name)
      if match:
        return [match.group(1),match.group(2),match.group(3)]
      return [name]

    baodata = {'name':'ROOT','value':0,'children':[]}
    for n,s in sizes.items():
      parent = baodata
      parent['value'] += s
      for nn in site_name_splitter(n):
        child = self._get_named_dict(nn,parent['children'])
        if child:
          child['value']+=s
          parent = child
        else:
          parent['children'].append({'name':nn,'value':s,'children':[]})
          parent = self._get_named_dict(nn,parent['children'])

    plot = BaobabChart(baodata,None,{
      'imageType':attrs['type'],
      'height':attrs['height'],
      'width':attrs['width'],
      'title':'%s\n%s'%(group,time.strftime('%H:%M %d/%m/%Y',time.gmtime())),
      'dpi':72,
      'unit':'B'
      })
    imgdata = plot.draw().read()

    return imgdata,3600


  def nodeNames(self):
    return self.cache.lockedCacheFetch('FL_NODENAMES',self._nodeNames)
  def groupNames(self):
    return self.cache.lockedCacheFetch('FL_GROUPNAMES',self._groupNames)


  def _fetchJSON(self,url):
    #_loginfo("FL_S: fetchJSON %s"%url)
    req = urllib.urlopen(url)
    jsondata = json.load(req)
    #_loginfo("FL_S: fetchJSON: returning JSON")
    return jsondata

  def _nodeNames(self):
    data = self._fetchJSON("http://cmsweb.cern.ch/phedex/datasvc/json/prod/nodes")
    nodes = sorted([n['name'] for n in data['phedex']['node']])
    return nodes,86400

  def _groupNames(self):
    data = self._fetchJSON("http://cmsweb.cern.ch/phedex/datasvc/json/prod/groups")
    groups = sorted([g['name'] for g in data['phedex']['group']])
    return groups,86400

  def _nodeData(self,node):
    data = self._fetchJSON( "http://cmsweb.cern.ch/phedex/datasvc/json/prod/blockreplicas?node=%s"%node)

    #make a lightweight version containing only the info we need
    # we still want the data so, eg, multiple image sizes can be made
    # but a large JSON document can be 10+MB and serialization kills memory and CPU, and is unnecessary

    blocks = []
    for block in data['phedex']['block']:
      if len(block['replica'])==1:
        blocks.append({
          'name':str(block['name']),
          'size':int(block['replica'][0]['bytes']),
          'complete':block['replica'][0]['complete']=='y',
          'custodial':block['replica'][0]['custodial']=='y',
          'group':str(block['replica'][0]['group']),
          'subscribed':block['replica'][0]['subscribed']=='y'
        })

    return blocks, 3600

  def _groupData(self,group):
    data = self._fetchJSON("http://cmsweb.cern.ch/phedex/datasvc/json/prod/groupusage?group=%s"%group)

    sizes = {}
    for node in data['phedex']['node']:
      name = node['name']
      size = 0
      if len(node['group'])==1:
        size = int(node['group'][0]['node_bytes'])
      sizes[name]=size

    return sizes,3600

  class NodeTransform(object):
    namesplit = re.compile(r'/(.*?)/([A-Za-z0-9]+)(.*?)/([\w-]+)#([\w-]+)')

    def __init__(self,attrs):
      self.parts = {
        'complete':self.block_complete,
        'custodial':self.block_custodial,
        'subscribed':self.block_subscribed,
        'group':self.block_group,
        'dataset':self.block_dataset,
        'prod':self.block_prod,
        'cond':self.block_cond,
        'tiers':self.block_tiers,
        'id':self.block_id
      }

      self.include_incomplete = attrs.get('incomplete',False)
      self.only_custodial = attrs.get('custodial',False)
      self.only_subscribed = attrs.get('subscribed',False)
      filter_name = attrs.get('filter',False)
      self.filter = False
      if filter_name:
        try:
          self.filter = re.compile(filter_name,re.IGNORECASE)
        except:
          self.filter = False
      scheme = attrs.get('scheme',False)

      if scheme and scheme=='prod:cond:tiers:dataset':
        self.split = self.default_split
      elif scheme:
        scheme = scheme.split(':')
        scheme = filter(lambda x: x in self.parts, scheme)
        scheme = [self.parts[s] for s in scheme]
        if len(scheme)>0 and len(scheme)<=10:
          self.scheme = scheme
        else:
          self.split = self.default_split
      else:
        self.split = self.default_split

    def default_split(self,block):
      match = self.namesplit.match(block['name'])
      if match:
        return (match.group(2),match.group(3).lstrip('-_'),match.group(4),match.group(1))
      return (block['name'].split('#')[0],)

    def split(self,block):
      match = self.namesplit.match(block['name'])
      if not match:
        print block['name']
      return tuple([s(block,match) for s in self.scheme])

    def __call__(self,blocks):
      result = {}
      for block in blocks:
        if block['complete'] or self.include_incomplete:
          if (not self.only_custodial) or (block['custodial']):
            if (not self.only_subscribed) or (block['subscribed']):
              if (not self.filter) or self.filter.search(block['name']):
                key = self.split(block)
                if key in result:
                  result[key]+=block['size']
                else:
                  result[key]=block['size']
      return result

    @staticmethod
    def block_complete(block,match):
      if block['complete']:
        return 'Complete'
      return 'Incomplete'
    @staticmethod
    def block_custodial(block,match):
      if block['custodial']:
        return 'Custodial'
      return 'Non-Custodial'
    @staticmethod
    def block_subscribed(block,match):
      if block['subscribed']:
        return 'Subscribed'
      return 'Non-Subscribed'
    @staticmethod
    def block_group(block,match):
      return block['group']
    @staticmethod
    def block_dataset(block,match):
      if match:
        return match.group(1)
      return 'Unknown'
    @staticmethod
    def block_prod(block,match):
      if match:
        return match.group(2)
      return 'Unknown'
    @staticmethod
    def block_cond(block,match):
      if match:
        return match.group(3).lstrip('-_')
      return 'Unknown'
    @staticmethod
    def block_tiers(block,match):
      if match:
        return match.group(4)
      return 'Unknown'
    @staticmethod
    def block_id(block,match):
      if match:
        return match.group(5)
      return 'Unknown'




