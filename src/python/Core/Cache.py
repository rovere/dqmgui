from Monitoring.Core.Utils import _logerr, _logwarn, _loginfo
from threading import Thread, Lock, Event
from copy import deepcopy, copy
from cherrypy import engine
import time, datetime
import cPickle
import sys
import random

class OverviewCache(Thread):
  """
  Thread-watched cache of data for prodmon (or anything else)
  All non _-prefixed methods should be thread-safe for external calling
  """
  exthook = "OverviewCache"

  def __init__(self,gui,interval=3,sizelimit=100000000,itemlimit=1000,wait_expiry=120,wait_interval=5):
    """
    Create a new ProdmonCache. Note that it is not started until the .start() method is called.

    @param interval: interval in seconds between the main loop running (3)
    @param cachesize: maximum size in bytes before old entries start getting culled. Note that this is done *imprecisely* and will not be strictly adhered to! (100MB)
    """
    _loginfo("CACHE: Creating new Overview cache interval=%s sec, sizelimit=%s bytes, itemlimit=%s"%(interval,sizelimit,itemlimit))

    Thread.__init__(self,name="OverviewCache")

    self.lock = Lock()
    self.stopme = False
    self.cachesize = 0
    self.cache = {} # objkey: tuple (expiry, size, data)
    self.interval = interval
    self.sizelimit = sizelimit
    self.itemlimit = itemlimit
    self.cacheevents = {}
    self.wait_expiry = wait_expiry
    self.wait_interval = wait_interval

    engine.subscribe('stop',self.stop)
    self.start()


  def stop(self):
    """
    When called, causes the thread to exit, the next time the main loop runs.
    """
    _loginfo("CACHE: Trying to stop cache")
    self.lock.acquire()
    try:
      self.stopme = True
    finally:
      self.lock.release()

  def run(self):
    while True:
      self.lock.acquire()
      #_loginfo("CACHE: Main loop, size=%s, items=%s"%(self.cachesize,len(self.cache)))
      try:
        if self.stopme:
          return
        keys_to_delete=[]
        now = time.time()
        for key in self.cache:
          if self.cache[key][0]<now:
            keys_to_delete.append(key)
        for key in keys_to_delete:
          #_loginfo("CACHE: Deleting expired key: %s"%key)
          self._remove(key)

          #if there is an associated key
          if key in self.cacheevents:
            del self.cacheevents[key]

      finally:
        self.lock.release()
      time.sleep(self.interval)

  def _get_oldest(self):
    t = sys.maxint
    oldest = None
    for key in self.cache:
      if self.cache[key][0]<t:
        t = self.cache[key][0]
        oldest = key
    return oldest

  def _cull_number(self):
    #_loginfo("CACHE: Culling excessive item number (%s)"%len(self.cache))
    while len(self.cache)>self.itemlimit:
      self._remove(self._get_oldest())

  def _cull_size(self):
    #_loginfo("CACHE: Culling excessive size (%s)"%self.cachesize)
    while self.cachesize>self.sizelimit:
      self._remove(self._get_oldest())



  def _insert(self,key,lifetime,size,data):
    #_loginfo("CACHE: _insert key=%s size=%s lifetime=%s" % (key,size,lifetime))
    if self._exists(key):
      self._remove(key)
    self.cache[key]=(lifetime+time.time(),size,data)
    self.cachesize+=size
    if len(self.cache)>self.itemlimit:
      self._cull_num()
    if self.cachesize>self.sizelimit:
      self._cull_size()


  def insert(self,key,lifetime,data):
    """
    Insert a new key into the cache, with associated expiry lifetime, size and data. Triggers volume cleaning if the new cache size exceeds the maximum.
    @param key: key for the new data - should be a string
    @param lifetime: desired cache lifetime of this object in seconds
    @param data: free data object. Preferably doesn't refer to other objects in the cache, or size calculations will become impossible.
    """
    self.lock.acquire()
    try:
      try:
        data = cPickle.dumps(data,cPickle.HIGHEST_PROTOCOL)
        self._insert(key,lifetime,len(data),data)
      except cPickle.PickleError:
        _logwarn("CACHE: Pickling error with key=%s"%key)
        pass
    finally:
      self.lock.release()


  def _remove(self,key):
    #_loginfo("CACHE: _remove key=%s"%key)
    size=-1
    if self._exists(key):
      size = self.cache[key][1]
      self.cachesize-=size
      del self.cache[key]

    return size

  def remove(self,key):
    """
    Remove a key from the cache, returning the freed space. Returns -1 if the key wasn't found (or if you set the key size to -1, which would be stupid.
    @param key: cache key to remove
    """
    size = -1
    self.lock.acquire()
    try:
      size = self._remove(key)
    finally:
      self.lock.release()
    return size

  def retrieve(self,key,default=None):
    #_loginfo("CACHE: Retrieve key=%s"%key)
    """
    Attempts to retrieve the data associated with a key from the cache. Returns default (None) if not found.
    @param key: cache key to retrieve data for
    @param default: what to return if the key is not in the cache (None)
    """
    #print "PC: Getting key [%s]" % (key)
    result=default
    self.lock.acquire()
    try:
      if self._exists(key):
        try:
          result = cPickle.loads(self.cache[key][2])
        except cPickle.PickleError:
          _logwarn("CACHE: Unpickling error with key=%s"%key)
          pass
    finally:
      self.lock.release()
    return result

  def _exists(self,key):
    return key in self.cache

  def exists(self,key):
    result=False
    self.lock.acquire()
    try:
      result = self._exists(key)
    finally:
      self.lock.release()
    return result

  def cachesize(self):
    """
    Get the current "size" of the cache
    """
    self.lock.acquire()
    try:
      result = self.cachesize
    finally:
      self.lock.release()
    return result

  def itemcount(self):
    """
    Get the number of items in the cache
    """
    self.lock.acquire()
    try:
      result = len(self.cache)
    finally:
      self.lock.release()
    return result

  def keylist(self):
    """
    Get a shallow-copied list of the keys in the cache
    """
    self.lock.acquire()
    try:
      result = self.cache.keys()
    finally:
      self.lock.release()
    return result

  def __getitem__(self,key):
    return self.retrieve(key)

  def __delitem__(self,key):
    return self.remove(key)

  def __len__(self):
    return self.itemcount()

  def __iter__(self):
    return self.keylist().__iter__()

  def __contains__(self,key):
    return self.exists(key)

  def key_wait_get(self,key):
    value = False
    self.lock.acquire()
    try:
      if key in self.cacheevents:
        value = self.cacheevents[key]
    finally:
      self.lock.release()
    return value

  def key_wait_set(self,key,value):
      self.lock.acquire()
      try:
          self.cacheevents[key]=value
      finally:
          self.lock.release()

  def cacheFetch(self,cachekey,f_produce,*args,**kwargs):
    if cachekey in self:
      #_loginfo("CACHE: cacheFetch (in cache) key=%s"%cachekey)
      cachedata = self[cachekey]
    else:
      #_loginfo("CACHE: cacheFetch (creating) key=%s"%cachekey)
      cachedata,lifetime = f_produce(*args,**kwargs)
      #_loginfo("CACHE: cacheFetch (finished creating) key=%s"%cachekey)
      if lifetime>0:
        self.insert(cachekey,lifetime,cachedata)
    return cachedata


  def lockedCacheFetch(self,cachekey,f_produce,*args,**kwargs):


    # If the key already exists, return it immediately
    if cachekey in self:
      #_loginfo("CACHE: lockedCacheFetch (in cache) key=%s"%cachekey)
      return self[cachekey]
    else:
      # Otherwise, if the 'wait' flag has been set for this key
      if self.key_wait_get(cachekey):
        #_loginfo("CACHE: lockedCacheFetch (waiting for flag) key=%s"%cachekey)
        # Pick a random time to wait between 2 and 5 minutes, after which we'll ignore the flag
        time_to_wait = self.wait_expiry
        # Wait until the 'wait' flag is unset, or our timeout expires
        while time_to_wait > 0 and self.key_wait_get(cachekey):
          time_to_wait -= self.wait_interval
          time.sleep(self.wait_interval)

        # Once the wait is up, either the flag was unset and the data is now available
        if cachekey in self:
          #_loginfo("CACHE: lockedCacheFetch (waited for flag, in cache) key=%s"%cachekey)
          return self[cachekey]
        # Or the time expired, in which case we'll make an attempt to fetch it, hopefully more successfully
        else:
          #_loginfo("CACHE: lockedCacheFetch (waited for flag, creating) key=%s"%cachekey)
          cachedata = None
          self.key_wait_set(cachekey,True)
          try:
            cachedata, lifetime = f_produce(*args, **kwargs)
            if lifetime>0:
              self.insert(cachekey,lifetime,cachedata)
          finally:
            self.key_wait_set(cachekey,False)
          return cachedata

      # The data doesn't exist, and no other thread is currently making it
      else:
        #_loginfo("CACHE: lockedCacheFetch (creating) key=%s"%cachekey)
        cachedata = None
        self.key_wait_set(cachekey,True)
        try:
          cachedata, lifetime = f_produce(*args, **kwargs)
          if lifetime>0:
            self.insert(cachekey,lifetime,cachedata)
        finally:
          self.key_wait_set(cachekey,False)
        return cachedata



  def asyncWorker(self,cachekey,f_produce,*args,**kwargs):
    """
    Worker function for async threads. This fills data in a separate thread, if it doesn't already exist.
    """
    #_loginfo("CACHE: asyncWorker key=%s"%cachekey)
    if not cachekey in self:
      try:
        cachedata, lifetime = f_produce(*args, **kwargs)
        if lifetime>0:
          self.insert(cachekey,lifetime,cachedata)
      except:
        pass


  def asyncCacheFill(self,cachekey,f_produce,*args,**kwargs):
    """
    Trigger a thread to try and asynchronously fill some data into the cache. Good for __init__ methods to prefill some information they require before request time.
    """
    kwargs['cachekey']=cachekey
    kwargs['f_produce']=f_produce
    async_fill_thread = Thread(name="asyncCacheFiller:%s"%cachekey,target=self.asyncWorker,args=args,kwargs=kwargs)
    async_fill_thread.start()


if __name__=='__main__':
    threadcount = 100
    keys = [str(i) for i in range(100)]
    def fast_producer(*args,**kwargs):
        rval1,rval2 = random.random(),random.random()
        if rval1<0.01:
            raise
        if rval2<0.01:
            return None
        return rval1,int(60*rval2)
    def slow_producer(*args,**kwargs):
        rval1,rval2 = random.random(),random.random()
        time.sleep(5*(rval1+rval2))
        if rval1<0.01:
            raise
        if rval2<0.01:
            return None
        return rval1,int(60*rval2)
    def fast_requestor(cache,name):
        ops=10000
        while ops>0:
            _loginfo("%s: alive, remain %s"%(name,ops))
            v1,v2 = random.random(),random.random()
            key = random.choice(keys)
            fill = fast_producer if v2>0.1 else slow_producer
            if v1>0.9:
                cache.asyncCacheFill(key,fill)
            elif v1>0.3:
                cache.lockedCacheFetch(key,fill)
            else:
                cache.cacheFetch(key,fill)
            ops-=1
        _loginfo("%s: finished"%name)

    cache = OverviewCache(None,interval=1,wait_expiry=5,wait_interval=1)
    threads = []
    for i in range(threadcount):
        tester = Thread(name='tester-%s'%i,target=fast_requestor,args=(cache,'TESTER-%s'%i))
        tester.start()
        threads.append(tester)
    for t in threads:
        t.join()
    cache.stop()

