#include "DQM/VisDQMArchiveSource.h"

#include <pthread.h>
#include <inttypes.h>

#include "classlib/iobase/IOStatus.h"

#include "DQM/VisDQMCache.h"
#include "DQM/VisDQMSource.h"
#include "DQM/VisDQMRenderLink.h"
#include "DQM/VisDQMTools.h"
#include "DQM/typedefs.h"
#include "DQM/PyReleaseInterpreterLock.h"

extern StringAtomTree stree;

void VisDQMArchiveSource::reset(void) {
  dataFiles_.clear();
  infoFiles_.clear();
  streamers_.clear();
  objnames_.clear();
  dsnames_.clear();
  vnames_.clear();
  samples_.clear();
  watch_.clear();
  retry_ = false;
}

void VisDQMArchiveSource::loadIndex(void) {
  // FIXME: Try doing the index re-read in the watcher thread
  // and simply swap 'current' values in place here, so we do
  // not become unnecessarily blocked waiting for the I/O while
  // serving HTTP requests.  We do have to block when file
  // open errors have occurred however.
  VisDQMFile *master = 0;
  index_.beginRead(master);
  for (VisDQMFile::ReadHead rdhead(master, IndexKey(0, 0));
       !rdhead.isdone(); rdhead.next()) {
    void *begin;
    void *end;
    IndexKey key;

    rdhead.get(&key, &begin, &end);
    if (key.catalogIndex() == VisDQMIndex::MASTER_SAMPLE_RECORD)
      samples_.push_back(*(const VisDQMIndex::Sample *)begin);
    else
      break;
  }

  readStrings(dsnames_, master,
              IndexKey(0, VisDQMIndex::MASTER_DATASET_NAME));
  readStrings(vnames_, master,
              IndexKey(0, VisDQMIndex::MASTER_CMSSW_VERSION));
  readStrings(objnames_, master,
              IndexKey(0, VisDQMIndex::MASTER_OBJECT_NAME));
  readStrings(streamers_, master,
              IndexKey(0, VisDQMIndex::MASTER_TSTREAMERINFO));
  index_.finishRead();
}

bool VisDQMArchiveSource::fileReadFailure(int ntries, const char *message) {
  bool abandon = (ntries < 0 || ntries > 10);
  loginfo()
      << (ntries >= 0 ? ntries : 1)
      << " transient error" << (ntries > 1 ? "s" : "")
      << " while reading data, " << (abandon ? "giving up" : "retrying")
      << "; error was: " << message << std::endl;
  pthread_rwlock_wrlock(&lock_);
  retry_ = true;
  pthread_rwlock_unlock(&lock_);
  return abandon;
}

void VisDQMArchiveSource::maybeReloadIndex(void) {
  // First determine if we should retry index open.
  pthread_rwlock_rdlock(&lock_);
  bool retry = retry_;
  pthread_rwlock_unlock(&lock_);

  // If we've seen file read errors, force index refresh.  But
  // avoid storming-heard several thread reload by checking
  // once again after we obtain the exclusive write lock.
  if (retry) {
    WRLock wrgate(&lock_);
    if (retry_) {
      reset();
      loadIndex();
    }
  }

  // If the index changed we should reload, so do so now.
  // Again avoid storming herd repeated updates.  Note that if
  // the previous block updated index, it cleared the watcher.
  if (watch_.changed()) {
    WRLock wrgate(&lock_);
    if (watch_.changed()) {
      reset();
      loadIndex();
    }
  }
}

VisDQMArchiveSource::VisDQMFilePtr
VisDQMArchiveSource::open(uint16_t kind, const uint32_t *file) {
  FileMap &files = (kind == VisDQMIndex::MASTER_FILE_INFO
                    ? infoFiles_ : dataFiles_);
  FileMap::iterator pos = files.find(file[kind]);
  if (pos == files.end()) {
    pthread_rwlock_unlock(&lock_);
    pthread_rwlock_wrlock(&lock_);
    pos = files.find(file[kind]);
  }

  if (pos == files.end()) {
    VisDQMFilePtr ptr(index_.open(kind,
                                  file[kind] >> 16,
                                  file[kind] & 0xffff,
                                  VisDQMFile::OPEN_READ),
                      &closeDQMFile);
    files.insert(FileMap::value_type(file[kind], ptr));
    return ptr;
  } else {
    return pos->second;
  }
}

VisDQMArchiveSource::SampleList::iterator
VisDQMArchiveSource::findSample(const VisDQMSample &sample) {
  SampleList::iterator i;
  SampleList::iterator e;
  for (i = samples_.begin(), e = samples_.end(); i != e; ++i)
    if (i->numObjects > 0
        && i->runNumber == sample.runnr
        && sample.dataset == dsnames_.key(i->datasetNameIdx))
      break;

  return i;
}


void VisDQMArchiveSource::getdata(const VisDQMSample &sample,
                                          const std::string &path,
                                          std::string &streamers,
                                          DQMNet::Object &obj) {
  streamers.clear();
  clearobj(obj);

  // Keep retrying until we can successfully read the data.
  for (int ntries = 1; true; ++ntries) {
    try {
      // Reload index if necessary.
      maybeReloadIndex();

      // Locate the requested sample.
      RDLock rdgate(&lock_);
      SampleList::const_iterator si = findSample(sample);
      uint32_t objindex = StringAtom(&objnames_, path,
                                     StringAtom::TestOnly).index();

      // Check if we found what we are looking for.  If yes, try
      // opening the data file and return the streamed data for
      // the requested object; if this fails we'll end up in a
      // catch statement which will loop around.
      if (si != samples_.end() && objindex != 0) {
        // FIXME: handle keys other than run summary?
        IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, objindex);
        IndexKey key;
        void *begin;
        void *end;

        {
          VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
          VisDQMFile::ReadHead rdhead(file.get(), keyidx);
          if (!rdhead.isdone()) {
            rdhead.get(&key, &begin, &end);
            if (key == keyidx) {
              VisDQMIndex::Summary *s = (VisDQMIndex::Summary *)begin;
              const char *data = (s->dataLength ? (const char *) (s+1) : 0);
              const char *qdata = (s->qtestLength ? (const char *) (s+1) + s->dataLength : 0);
              obj.flags = s->properties;
              obj.tag = s->tag;
              obj.version = si->lastImportTime;
              if (data)
                obj.scalar.append(data, s->dataLength-1);
              if (qdata)
                obj.qdata.append(qdata, s->qtestLength);
              streamers = streamers_.key(si->streamerInfoIdx);
            }
          }
        }

        if (isROOTType(obj.flags)) {
          VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_DATA, si->files));
          VisDQMFile::ReadHead rdhead(file.get(), keyidx);
          if (!rdhead.isdone()) {
            rdhead.get(&key, &begin, &end);
            if (key == keyidx && begin != end)
              obj.rawdata.insert(obj.rawdata.end(),
                                 (const unsigned char *)begin,
                                 (const unsigned char *)end);
          }
        }
      }

      // Not found, return null data.
      break;
    } catch(Error &e) {
      if (fileReadFailure(ntries, e.explain().c_str()))
        break;
    } catch(std::exception &e) {
      if (fileReadFailure(ntries, e.what()))
        break;
    } catch(...) {
      if (fileReadFailure(ntries, "(unknown error)"))
        break;
    }
  }
}

void VisDQMArchiveSource::getattr(const VisDQMSample &sample,
                                          const std::string &path,
                                          VisDQMIndex::Summary &attrs,
                                          std::string & /* xstreamers */,
                                          DQMNet::Object & /* xobj */) {
  clearattr(attrs);

  // Keep retrying until we can successfully read the data.
  for (int ntries = 1; true; ++ntries) {
    try {
      // Reload index if necessary.
      maybeReloadIndex();

      // Locate the requested sample.
      RDLock rdgate(&lock_);
      SampleList::const_iterator si = findSample(sample);
      uint32_t objindex = StringAtom(&objnames_, path,
                                     StringAtom::TestOnly).index();

      // Check if we found what we are looking for.  If yes, try
      // opening the data file and return the streamed data for
      // the requested object; if this fails we'll end up in a
      // catch statement which will loop around.
      if (si != samples_.end() && objindex != 0) {
        // FIXME: handle keys other than run summary?
        IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, objindex);
        IndexKey key;
        void *begin;
        void *end;

        VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
        VisDQMFile::ReadHead rdhead(file.get(), keyidx);
        if (!rdhead.isdone()) {
          rdhead.get(&key, &begin, &end);
          if (key == keyidx)
            attrs = *(VisDQMIndex::Summary *)begin;
        }
      }

      // No error, return.
      break;
    } catch (Error &e) {
      if (fileReadFailure(ntries, e.explain().c_str()))
        break;
    } catch (std::exception &e) {
      if (fileReadFailure(ntries, e.what()))
        break;
    } catch (...) {
      if (fileReadFailure(ntries, "(unknown error)"))
        break;
    }
  }
}

py::str VisDQMArchiveSource::getJson(const int runnr,
                                     const std::string &dataset,
                                     const std::string &path,
                                     py::dict opts) {
  VisDQMSample sample(SAMPLE_ANY, runnr, dataset);
  std::map<std::string, std::string> options;
  bool jsonok = false;
  std::string jsonData;
  std::string imagetype;
  std::string streamers;
  DQMNet::Object obj;
  copyopts(opts, options);

  {
    PyReleaseInterpreterLock nogil;
    getdata(sample, path, streamers, obj);
    jsonok = link_->prepareJson(jsonData, imagetype, path, options,
                                &streamers, &obj, 1, STDIMGOPTS);
  }
  if (jsonok) {
    py::str _str(jsonData);
    return _str;
  }
  return "\"error\":\"JSON preparing process was interrupted.\"";
}

py::tuple VisDQMArchiveSource::plot(int runnr,
                                    const std::string &dataset,
                                    const std::string &path,
                                    py::dict opts) {
  VisDQMSample sample(SAMPLE_ANY, runnr, dataset);
  std::map<std::string, std::string> options;
  bool imageok = false;
  std::string imagedata;
  std::string imagetype;
  std::string streamers;
  DQMNet::Object obj;
  copyopts(opts, options);

  {
    PyReleaseInterpreterLock nogil;
    getdata(sample, path, streamers, obj);
    imageok = link_->render(imagedata, imagetype, path, options,
                            &streamers, &obj, 1, STDIMGOPTS);
  }

  if (imageok)
    return py::make_tuple(imagetype, imagedata);
  else
    return py::make_tuple(py::object(), py::object());
}

// Refresh data from the database for a run.  Checks if the master
// database exists, and if so, if it has changed relative to the
// last copy and the time we last read in the data.  If so, discards
// any existing cached data and copies the file again.  Then checks
// to see if the data for the requested run is already cached, and
// if not, reads it in.  Also discards cached data for runs that
// have not been accessed recently, where "recent" is MAX_CACHE_LIFE
// seconds.
void VisDQMArchiveSource::update(VisDQMItems &items,
                                 const VisDQMSample &sample,
                                 VisDQMEventNum &current,
                                 VisDQMRegexp *rxmatch,
                                 Regexp *rxsearch,
                                 bool *alarm) {
  // No point in even trying unless this is archived data.
  if (sample.type < SAMPLE_ONLINE_DATA || sample.type >= SAMPLE_ANY)
    return;

  // Keep retrying until we can successfully read the data.
  for (int ntries = 1; true; ++ntries) {
    try {
      // Reload index if necessary.
      maybeReloadIndex();

      // Locate the requested sample.
      RDLock rdgate(&lock_);
      SampleList::const_iterator si = findSample(sample);

      // Return with empty "items" if we didn't find the sample.
      if (si == samples_.end())
        return;

      // Found the sample, try opening the data file.  If this
      // fails we will end up in the catch statement which will
      // loop around to retry a certain number of times.  The
      // stack unwinding automatically handles lock release.
      VisDQMFilePtr file(open(VisDQMIndex::MASTER_FILE_INFO, si->files));

      // Read the summary for the requested sample.
      VisDQMEventNumList eventnums;
      IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, 0);
      for (VisDQMFile::ReadHead rdhead(file.get(), keyidx);
           !rdhead.isdone(); rdhead.next()) {
        IndexKey key;
        void *begin;
        void *end;
        rdhead.get(&key, &begin, &end);
        // FIXME: handle keys other than run summary?
        uint64_t keyparts[4] = { key.sampleidx(),
                                 key.type(),
                                 key.lumiend(),
                                 key.objnameidx() };
        if (keyparts[0] != keyidx.sampleidx() || keyparts[1] != 0)
          break;

        VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
        uint32_t report = s->properties & VisDQMIndex::SUMMARY_PROP_REPORT_MASK;
        const char *data = (s->dataLength ? (const char *) (s+1) : 0);
        const std::string &name = objnames_.key(keyparts[3]);
        StringAtom sname(&stree, name);

        // Update run number information.
        if (isIntegerType(s->properties) && name.find("/EventInfo/i") != std::string::npos)
          getEventInfoNum(name, data, eventnums);
        else if (isRealType(s->properties) && name.find("/EventInfo/ru") != std::string::npos)
          getEventInfoNum(name, data, eventnums);

        // If it doesn't match search criteria, skip it.
        if (!fastmatch(rxmatch, sname)
            || (rxsearch && rxsearch->search(name) < 0)
            || (alarm && (report != 0) != (*alarm == true)))
          continue;

        // We want to keep it, construct an item for use.
        shared_ptr<VisDQMItem> i(new VisDQMItem);
        i->flags = s->properties;
        i->version = si->lastImportTime;
        i->name = sname;
        i->plotter = this;
        buildParentNames(i->parents, i->name);
        if (s->dataLength)
          i->data = data;

        items[i->name] = i;
      }

      // Pick greatest run/lumi/event number combo seen.
      setEventInfoNums(eventnums, current);

      // Successfully read the data, return.
      return;
    } catch (Error &e) {
      if (fileReadFailure(ntries, e.explain().c_str()))
        return;
    } catch (std::exception &e) {
      if (fileReadFailure(ntries, e.what()))
        return;
    } catch (...) {
      if (fileReadFailure(ntries, "(unknown error)"))
        return;
    }
  }
}

// Initialise a new session.  Select the most recent sample.
void VisDQMArchiveSource::prepareSession(py::dict session) {
  if (!session.has_key("dqm.sample.type")) {
    bool found = false;
    long runnr = -1;
    uint32_t importversion = 0;
    std::string dataset;
    VisDQMSampleType type = SAMPLE_ANY;

    {
      PyReleaseInterpreterLock nogil;

      // Make sure we are up to date.
      VisDQMItems items;
      VisDQMSample sample(SAMPLE_ONLINE_DATA, 0);
      VisDQMEventNum current = { "", -1, -1, -1, -1 };
      update(items, sample, current, 0, 0, 0);

      // Scan samples for the most recent one.
      RDLock rdgate(&lock_);
      SampleList::const_iterator newest;
      SampleList::const_iterator si;
      SampleList::const_iterator se;
      for (newest = si = samples_.begin(), se = samples_.end();
           si != se; ++si)
        if (si->runNumber > newest->runNumber)
          newest = si;
        else if (si->runNumber == newest->runNumber
                 && si->lastImportTime > newest->lastImportTime)
          newest = si;

      if (newest != se) {
        found = true;
        runnr = newest->runNumber;
        dataset = dsnames_.key(newest->datasetNameIdx);
        importversion = newest->importVersion;
        type = (newest->cmsswVersion > 0 ? SAMPLE_OFFLINE_RELVAL
                : newest->runNumber == 1 ? SAMPLE_OFFLINE_MC
                : rxonline_.search(dataset) < 0
                ? SAMPLE_OFFLINE_DATA : SAMPLE_ONLINE_DATA);
      }
    }

    if (found) {
      session["dqm.sample.runnr"] = runnr;
      session["dqm.sample.dataset"] = dataset;
      session["dqm.sample.importversion"] = importversion;
      session["dqm.sample.type"] = long(type);
    }
  }
}

// Extract data from this source.  Work is done by update().
void VisDQMArchiveSource::scan(VisDQMItems &result,
                                       const VisDQMSample &sample,
                                       VisDQMEventNum &current,
                                       VisDQMRegexp *rxmatch,
                                       Regexp *rxsearch,
                                       bool *alarm,
                                       std::string *,
                                       VisDQMRegexp *) {
  update(result, sample, current, rxmatch, rxsearch, alarm);
}

void VisDQMArchiveSource::json(const VisDQMSample &sample,
                                       const std::string &rootpath,
                                       bool fulldata,
                                       bool lumisect,
                                       std::string &result,
                                       double &stamp,
                                       const std::string &mename,
                                       std::set<std::string> &dirs) {
  // No point in even trying unless this is archived data.
  if (sample.type < SAMPLE_ONLINE_DATA)
    return;

  // Keep retrying until we can successfully read the data.
  for (int ntries = 1; true; ++ntries) {
    try {
      // Reload index if necessary.
      maybeReloadIndex();

      // Locate the requested sample.
      RDLock rdgate(&lock_);
      SampleList::const_iterator si = findSample(sample);

      // Return with empty "items" if we didn't find the sample.
      if (si == samples_.end())
        return;

      // Stamp as latest import time.
      stamp = std::max(stamp, si->processedTime * 1e-9);

      // Stuff streamers first, in case it is missing.
      if (result.find("streamerinfo") == std::string::npos)
        result += StringFormat("%1{ \"streamerinfo\":\"%2\" }\n")
            .arg(result.empty() ? "" : ", ")
            .arg(fulldata ? hexlify(streamers_.key(si->streamerInfoIdx)) : std::string());

      // Found the sample, try opening the data file.  If this
      // fails we will end up in the catch statement which will
      // loop around to retry a certain number of times.  The
      // stack unwinding automatically handles lock release.
      VisDQMFilePtr info(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
      VisDQMFilePtr data(open(VisDQMIndex::MASTER_FILE_DATA, si->files));
      std::set<std::string>::iterator di, de;
      DQMNet::DataBlob rawdata;
      DQMNet::QReports qreports;
      std::string qstr;
      std::string name;
      std::string dir;

      // Read the summary for the requested sample.
      IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, 0);
      VisDQMFile::ReadHead rdinfo(info.get(), keyidx);
      for ( ; !rdinfo.isdone(); rdinfo.next()) {
        IndexKey ikey;
        IndexKey dkey;
        void *begin;
        void *end;
        rdinfo.get(&ikey, &begin, &end);
        uint64_t keyparts[4] = { ikey.sampleidx(),
                                 ikey.type(),
                                 ikey.lumiend(),
                                 ikey.objnameidx() };
        if (keyparts[0] != keyidx.sampleidx()
            || (!lumisect && (keyparts[1] != 0)))
          break;

        VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
        const std::string &path = objnames_.key(keyparts[3]);
        name.clear();
        dir.clear();
        splitPath(dir, name, path);

        if (rootpath == dir)
          /* Keep object directly in rootpath directory */;
        else if (isSubdirectory(rootpath, dir)) {
          // Object in subdirectory, remember subdirectory part
          size_t begin = (rootpath.empty() ? 0 : rootpath.size()+1);
          size_t slash = dir.find('/', begin);
          dirs.insert(std::string(dir, begin, slash - begin));
          continue;
        } else {
          // Object outside directory of interest to us, skip
          continue;
        }

        // If we filter by ME name, get rid of this ME in case we
        // have no match
        if (!mename.empty() && mename != name)
          continue;

        // Get data for this object.
        if (fulldata) {
          rawdata.clear();
          VisDQMFile::ReadHead rddata(data.get(), ikey);
          if (!rddata.isdone()) {
            rddata.get(&dkey, &begin, &end);
            if (dkey == ikey)
              rawdata.insert(rawdata.end(),
                             (unsigned char *) begin,
                             (unsigned char *) end);
          }
        }

        const char *data = (s->dataLength ? (const char *) (s+1) : "");
        const char *qdata
            = (s->qtestLength ? ((const char *) (s+1) + s->dataLength) : "");

        objectToJSON(name,
                     data,
                     qdata,
                     rawdata,
                     keyparts[1] == 0 ? 0 : keyparts[2],
                     s->properties,
                     s->tag,
                     s->nentries,
                     s->nbins,
                     s->mean,
                     s->rms,
                     s->bounds,
                     qreports,
                     qstr,
                     result);
      }

      // Format sub-directories, only in case we were not
      // explicitely asked for a single MonitorElement: in this
      // case omit directory listing.
      if (mename.empty())
        for (di = dirs.begin(), de = dirs.end(); di != de; ++di)
          result += StringFormat(", { \"subdir\": %1 }\n").arg(stringToJSON(*di));

      // Successfully read the data, return.
      return;
    } catch (Error &e) {
      if (fileReadFailure(ntries, e.explain().c_str()))
        return;
    } catch (std::exception &e) {
      if (fileReadFailure(ntries, e.what()))
        return;
    } catch (...) {
      if (fileReadFailure(ntries, "(unknown error)"))
        return;
    }
  }
}

void VisDQMArchiveSource::getcert(VisDQMSample &sample,
                                          const std::string &rootpath,
                                          const std::string &variableName,
                                          std::vector<VisDQMIndex::Summary> &attrs,
                                          std::vector<double> &axisvals,
                                          std::string &binlabels) {
  attrs.reserve(4000);
  axisvals.reserve(4000);
  char tmpbuf[64];
  // No point in even trying unless this is archived data.
  if (sample.type < SAMPLE_ONLINE_DATA)
    return;

  // Keep retrying until we can successfully read the data.
  for (int ntries = 1; true; ++ntries) {
    try {
      // Reload index if necessary.
      maybeReloadIndex();

      // Locate the requested sample.
      RDLock rdgate(&lock_);
      SampleList::const_iterator si = findSample(sample);

      // Return with empty "items" if we didn't find the sample.
      if (si == samples_.end())
        return;

      // Found the sample, try opening the info file.  If this
      // fails we will end up in the catch statement which will
      // loop around to retry a certain number of times.  The
      // stack unwinding automatically handles lock release.
      VisDQMFilePtr info(open(VisDQMIndex::MASTER_FILE_INFO, si->files));
      std::string name;
      std::string dir;
      std::string path;

      // Read the summary for the requested sample.
      IndexKey keyidx(uint64_t(si - samples_.begin()), 0, 0, 0);
      VisDQMFile::ReadHead rdinfo(info.get(), keyidx);
      for ( ; !rdinfo.isdone(); rdinfo.next()) {
        IndexKey ikey;
        void *begin;
        void *end;
        rdinfo.get(&ikey, &begin, &end);
        uint64_t keyparts[4] = { ikey.sampleidx(),
                                 ikey.type(),
                                 ikey.lumiend(),
                                 ikey.objnameidx() };
        if (keyparts[0] != keyidx.sampleidx())
          break;
        if (keyparts[1] == 0)
          continue;

        VisDQMIndex::Summary *s = (VisDQMIndex::Summary *) begin;
        const std::string &path = objnames_.key(keyparts[3]);
        name.clear();
        dir.clear();
        splitPath(dir, name, path);

        // Skip object outside directory of interest to us.
        if (rootpath != dir)
          continue;

        // Select only the proper variable, in case one was
        // supplied.
        if (!variableName.empty() && variableName != name)
          continue;

        uint32_t type = s->properties & DQMNet::DQM_PROP_TYPE_MASK;
        // Expose only scalar data: there is no point in making a
        // trend plot for TH* objects.
        if (type == DQMNet::DQM_PROP_TYPE_INT
            || type == DQMNet::DQM_PROP_TYPE_REAL) {
          // Get data for this object.
          const char *data = (s->dataLength ? (const char *) (s+1) : "");
          double t = atof(data);
          // Check if the last lumisection used is equal to the
          // current one decremented by 1 unit. If this is not the
          // case, fill the gap with values -2 (to differentiate
          // between default -1), until we reach the current
          // lumisection.
          if (attrs.size() > 0)
            while (attrs.back().mean[0] < (double)(keyparts[2]-1)) {
              VisDQMIndex::Summary tmp = attrs.back();
              tmp.mean[0] += 1.;
              tmp.mean[1] = -2.;
              attrs.push_back(tmp);
              axisvals.push_back(tmp.mean[0]); /* Lumisection number */
              makeBinLabel(tmpbuf, (uint64_t)tmp.mean[0]);
              std::string binlabel(tmpbuf);
              binlabels.append(binlabel.c_str(), binlabel.size()+1);
            }
          axisvals.push_back((double)keyparts[2]); /* Lumisection number */
          attrs.push_back(*s);
          attrs.back().mean[0] = (double)keyparts[2];
          attrs.back().mean[1] = t;
          makeBinLabel(tmpbuf, keyparts[2]);
          std::string binlabel(tmpbuf);
          binlabels.append(binlabel.c_str(), binlabel.size()+1);
        }
      }

      // Successfully read the data, return.
      return;
    } catch (Error &e) {
      if (fileReadFailure(ntries, e.explain().c_str()))
        return;
    } catch (std::exception &e) {
      if (fileReadFailure(ntries, e.what()))
        return;
    } catch (...) {
      if (fileReadFailure(ntries, "(unknown error)"))
        return;
    }
  }
}

void VisDQMArchiveSource::samples(VisDQMSamples &samples) {
  try {
    // Reload index if necessary.
    maybeReloadIndex();

    // Locate the requested sample.
    RDLock rdgate(&lock_);
    SampleList::const_iterator si;
    SampleList::const_iterator se;
    samples.reserve(samples.size() + samples_.size());
    for (si = samples_.begin(), se = samples_.end(); si != se; ++si) {
      if (!si->numObjects)
        continue;

      samples.push_back(VisDQMSample());
      VisDQMSample &s = samples.back();
      s.dataset = dsnames_.key(si->datasetNameIdx);
      s.version = vnames_.key(si->cmsswVersion);
      s.importversion = si->importVersion;
      s.runnr = si->runNumber;
      s.type = (si->cmsswVersion > 0 ? SAMPLE_OFFLINE_RELVAL
                : si->runNumber == 1 ? SAMPLE_OFFLINE_MC
                : rxonline_.search(s.dataset) < 0 ? SAMPLE_OFFLINE_DATA
                : SAMPLE_ONLINE_DATA);
      s.origin = this;
      s.time = si->processedTime;
    }
  } catch (Error &e) {
    fileReadFailure(-1, e.explain().c_str());
  } catch (std::exception &e) {
    fileReadFailure(-1, e.what());
  } catch (...) {
    fileReadFailure(-1, "(unknown error)");
  }
}
