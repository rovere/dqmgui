class VisDQMWorkspace
{
protected:
  py::object	gui_;
  double	guiTimeStamp_;
  std::string	guiServiceName_;
  std::string	guiServiceListJSON_;
  std::string	guiWorkspaceListJSON_;
  std::string	workspaceName_;
  void		(*snapdump_)(const char *);

public:
  VisDQMWorkspace(py::object gui, const std::string &name)
    : gui_(gui),
      guiTimeStamp_(py::extract<double>(gui.attr("stamp"))),
      guiServiceName_(py::extract<std::string>(gui.attr("serviceName"))),
      workspaceName_(name),
      snapdump_(0)
    {
      if (void *sym = dlsym(0, "igprof_dump_now"))
        snapdump_ = (void(*)(const char *)) sym;
    }

  virtual ~VisDQMWorkspace(void)
    {}

  virtual std::string
  state(py::dict /* session */)
    {
      return "{}";
    }

  virtual void
  profilesnap(void)
    {
      if (snapdump_)
      {
        char tofile[256];
        struct timeval tv;
        gettimeofday(&tv, 0);
        if (snprintf(tofile, sizeof(tofile), "|gzip -c>igprof.dqmgui.%ld.%f.gz",
	             (long) getpid(), tv.tv_sec + 1e-6*tv.tv_usec) < int(sizeof(tofile)))
	  snapdump_(tofile);
      }
    }

  // Helper function to compute next available run in current dataset
  int changeRun(py::dict session, bool forward)
    {
      std::vector<VisDQMSource *> srclist;
      std::set<long>		  runlist;
      sources(srclist);

      VisDQMSample sample(sessionSample(session));

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {

	PyReleaseInterpreterLock nogil;
	VisDQMSamples		samples;
	std::set<long>::iterator rli, rle;

	// Request samples from backends.
	for (size_t i = 0, e = srclist.size(); i != e; ++i)
	  srclist[i]->samples(samples);

	// Filter out samples which do not pass search criteria.  In
	// this particular case the fixed criteria are current sample
	// and current dataset.
	for (size_t i = 0, e = samples.size(); i != e; ++i)
	{
	  VisDQMSample &s = samples[i];

	  // Stay on current sample type
	  if (s.type != sample.type)
	    continue;
	  // Stay on current dataset
	  if (s.dataset != sample.dataset)
	    continue;
	  runlist.insert(s.runnr);
	}
      }
	// Now find current run and move according to the prescription
	// received.
	std::set<long>::iterator curr = runlist.find(sample.runnr);
	assert (curr != runlist.end());
	if (forward)
	  return ++curr != runlist.end() ? *curr : *(--curr);
	else
	  return curr == runlist.begin() ? *curr : *(--curr);
    }

protected:
  template <class T> T
  workspaceParam(const py::dict &session, const char *key)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(this->workspaceName_));
    }

  template <class T> T
  workspaceParam(const py::dict &session, const char *key, const T &def)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(this->workspaceName_, def));
    }

  template <class T> T
  workspaceParamOther(const py::dict &session, const char *key, const std::string &wspace)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(wspace));
    }

  template <class T> T
  workspaceParamOther(const py::dict &session, const char *key, const T &def, const std::string &wspace)
    {
      py::dict d = py::extract<py::dict>(session.get(key));
      return py::extract<T>(d.get(wspace, def));
    }

  // Produce panel configuration format.
  static std::string
  sessionPanelConfig(const py::dict &session, const char *panel)
    {
      char attrname[64];
      bool show;
      int  x;
      int  y;

      snprintf(attrname, sizeof(attrname), "dqm.panel.%.20s.show", panel);
      show = py::extract<bool>(session.get(attrname, false));

      snprintf(attrname, sizeof(attrname), "dqm.panel.%.20s.x", panel);
      x = py::extract<int>(session.get(attrname, -1));

      snprintf(attrname, sizeof(attrname), "dqm.panel.%.20s.y", panel);
      y = py::extract<int>(session.get(attrname, -1));

      return StringFormat("'%1':{'show':%2,'x':%3,'y':%4}")
	.arg(panel).arg(show).arg(x).arg(y);
    }

    // Produce zoom configuration format.
  static std::string
  sessionZoomConfig(const py::dict &session)
    {
      bool show, jsonmode;
      int  x, jx;
      int  y, jy;
      int  w, jw;
      int  h, jh;

      show = py::extract<bool>(session.get("dqm.zoom.show", false));
      x = py::extract<int>(session.get("dqm.zoom.x", -1));
      y = py::extract<int>(session.get("dqm.zoom.y", -1));
      w = py::extract<int>(session.get("dqm.zoom.w", -1));
      h = py::extract<int>(session.get("dqm.zoom.h", -1));
      jsonmode = py::extract<bool>(session.get("dqm.zoom.jsonmode", false));
      jx = py::extract<int>(session.get("dqm.zoom.jx", -1));
      jy = py::extract<int>(session.get("dqm.zoom.jy", -1));
      jw = py::extract<int>(session.get("dqm.zoom.jw", -1));
      jh = py::extract<int>(session.get("dqm.zoom.jh", -1));

      return StringFormat("'zoom':{'show':%1,'x':%2,'y':%3,'w':%4,'h':%5,\
                           'jsonmode':%6,'jx':%7,'jy':%8,'jw':%9,'jh':%10}")
          .arg(show).arg(x).arg(y).arg(w).arg(h)
          .arg(jsonmode).arg(jx).arg(jy).arg(jw).arg(jh);
    }

  // Produce Certification zoom configuration format.
  static std::string
  sessionCertZoomConfig(const py::dict &session)
    {
      bool show;
      int  x;
      int  y;
      int  w;
      int  h;

      show = py::extract<bool>(session.get("dqm.certzoom.show", false));
      x = py::extract<int>(session.get("dqm.certzoom.x", -1));
      y = py::extract<int>(session.get("dqm.certzoom.y", -1));
      w = py::extract<int>(session.get("dqm.certzoom.w", -1));
      h = py::extract<int>(session.get("dqm.certzoom.h", -1));

      return StringFormat("'certzoom':{'show':%1,'x':%2,'y':%3,'w':%4,'h':%5}")
	.arg(show).arg(x).arg(y).arg(w).arg(h);
    }

  static std::string
  sessionReferenceOne(const py::dict &ref)
    {
      return StringFormat("{'type':'%1','run':%2,'dataset':%3, 'ktest':%4}")
	.arg(py::extract<std::string>(ref.get("type"))) // refobj, other, none
	.arg(stringToJSON(py::extract<std::string>(ref.get("run"))))
	.arg(stringToJSON(py::extract<std::string>(ref.get("dataset"))))
	.arg(stringToJSON(py::extract<std::string>(ref.get("ktest"))));
    }

  // Produce a reference description.
  static std::string
  sessionReference(const py::dict &session)
    {
      py::dict refdict = py::extract<py::dict>(session.get("dqm.reference"));
      py::list refspec = py::extract<py::list>(refdict.get("param"));
      std::string ref1(sessionReferenceOne(py::extract<py::dict>(refspec[0])));
      std::string ref2(sessionReferenceOne(py::extract<py::dict>(refspec[1])));
      std::string ref3(sessionReferenceOne(py::extract<py::dict>(refspec[2])));
      std::string ref4(sessionReferenceOne(py::extract<py::dict>(refspec[3])));
      return StringFormat("{'position':'%1', 'show':'%2', \
                            'param':[%3,%4,%5,%6]}")
        .arg(py::extract<std::string>(refdict.get("position")))    // overlay, on-side
	.arg(py::extract<std::string>(refdict.get("show")))        // all, none, custom
	.arg(ref1).arg(ref2).arg(ref3).arg(ref4);
    }

  static std::string
  sessionStripChart(const py::dict &session)
    {
      std::string type = py::extract<std::string>(session.get("dqm.strip.type"));
      std::string omit = py::extract<std::string>(session.get("dqm.strip.omit"));
      std::string axis = py::extract<std::string>(session.get("dqm.strip.axis"));
      std::string n = py::extract<std::string>(session.get("dqm.strip.n"));
      return StringFormat("{'type':'%1', 'omit':'%2', 'axis':'%3','n':%4}")
	.arg(type).arg(omit).arg(axis).arg(stringToJSON(n));
    }


  // Produce a standard response.
  std::string
  makeResponse(const std::string &state,
	       int interval,
	       VisDQMEventNum &current,
	       const std::string &services,
	       const std::string &workspace,
	       const std::string &workspaces,
	       const std::string &submenu,
	       const VisDQMSample &sample,
	       const std::string &filter,
               const std::string &showstats,
               const std::string &showerrbars,
	       const std::string &reference,
	       const std::string &strip,
	       const std::string &rxstr,
	       const std::string &rxerr,
	       size_t rxmatches,
	       const std::string &toolspanel,
	       Time startTime)
    {
      StringFormat result
	= StringFormat("([{'kind':'AutoUpdate', 'interval':%1, 'stamp':%2, 'serverTime':%21},"
		       "{'kind':'DQMHeaderRow', 'run':\"%3\", 'lumi':\"%4\", 'event':\"%5\","
		       " 'runstart':\"%6\", 'service':'%7', 'services':[%8], 'workspace':'%9',"
		       " 'workspaces':[%10], 'view':{'show':'%11','sample': %12, 'filter':'%13',"
		       " 'showstats': %14, 'showerrbars': %15, 'reference':%16, 'strip':%17,"
                       " 'search':%18, %19}},%20])")
	.arg(interval)
	.arg(guiTimeStamp_, 0, 'f')
	.arg(current.runnr < 0 ? std::string("(None)")
	     : current.runnr <= 1 ? std::string("(Simulated)")
	     : thousands(StringFormat("%1").arg(current.runnr)))
	.arg(current.luminr < 0 ? std::string("(None)")
	     : thousands(StringFormat("%1").arg(current.luminr)))
	.arg(current.eventnr < 0 ? std::string("(None)")
	     : thousands(StringFormat("%1").arg(current.eventnr)))
	.arg(formatStartTime(current.runstart))
	.arg(guiServiceName_)
	.arg(services)
	.arg(workspace)
	.arg(workspaces)
	.arg(submenu)
	.arg(sampleToJSON(sample))
	.arg(filter)
        .arg(showstats)
        .arg(showerrbars)
	.arg(reference)
	.arg(strip)
	.arg(StringFormat("{'pattern':%1, 'valid':%2, 'nmatches':%3, 'error':%4}")
	     .arg(stringToJSON(rxstr))
	     .arg(rxerr.empty() ? 1 : 0)
	     .arg(rxmatches)
	     .arg(stringToJSON(rxerr)))
	.arg(toolspanel)
	.arg(state);

      return result.arg((Time::current() - startTime).ns() * 1e-6, 0, 'f');
    }

  // Get the list of native VisDQMSources configured in the GUI.
  void
  sources(std::vector<VisDQMSource *> &into)
    {
      py::list sources = py::extract<py::list>(gui_.attr("sources"));
      py::stl_input_iterator<py::object> i(sources), e;
      for ( ; i != e; ++i)
      {
	py::extract<VisDQMSource *> x(*i);
	if (x.check())
	  into.push_back(x());
      }
    }

  // Build and return a cached JSON representation of the DQM GUI
  // service list.  This must be called when it's safe to access
  // python.  Will build the list just once.
  const std::string &
  serviceListJSON(void)
    {
      if (guiServiceListJSON_.empty())
      {
	StringList parts;
	parts.reserve(100);

	py::list services = py::extract<py::list>(gui_.attr("services"));
	for (py::stl_input_iterator<py::tuple> i(services), e; i != e; ++i)
	  parts.push_back(StringFormat("{'title':'%1', 'href':'%2'}")
			  .arg(py::extract<std::string>((*i)[0]))
			  .arg(py::extract<std::string>((*i)[1])));
	guiServiceListJSON_ = StringOps::join(parts, ", ");
      }

      return guiServiceListJSON_;
    }

  // Build and return a cached JSON representation of the DQM GUI
  // workspace object list.  This must be called when it's safe to
  // access python.  Will build the list just once.
  const std::string &
  workspaceListJSON(void)
    {
      if (guiWorkspaceListJSON_.empty())
      {
	StringList parts;
	parts.reserve(100);
	py::list workspaces = py::extract<py::list>(gui_.attr("workspaces"));
	for (py::stl_input_iterator<py::object> i(workspaces), e; i != e; ++i)
	{
	  int         rank(py::extract<int>(i->attr("rank")));
	  std::string name(py::extract<std::string>(i->attr("name")));
	  std::string category(py::extract<std::string>(i->attr("category")));
	  std::string lower(name);
	  std::transform(lower.begin(), lower.end(), lower.begin(), tolower);
	  parts.push_back(StringFormat("{'title':'%1', 'label':'%2', 'category':'%3', 'rank':%4}")
			  .arg(name).arg(lower).arg(category).arg(rank));
	}
	guiWorkspaceListJSON_ = StringOps::join(parts, ", ");
      }

      return guiWorkspaceListJSON_;
    }

  // Get the contents currently shown in this session.
  void
  buildContents(const std::vector<VisDQMSource *> &sources,
		VisDQMItems &contents,

		StringAtomSet &myobjs,
		const VisDQMSample &sample,
		VisDQMEventNum &current,

		VisDQMRegexp *rxmatch,
		Regexp *rxsearch,
		bool *alarm,

		std::string *layoutroot,
		VisDQMRegexp *rxlayout)
    {
      // Now get filtered contents from all the sources.
      for (size_t i = 0, e = sources.size(); i != e; ++i)
      {
	sources[i]->scan(contents, sample, current,
			 rxmatch, rxsearch, alarm,
			 layoutroot, rxlayout);
      }


      // Add in the quick collection, filtered against rx if there is
      // one, but not the general workspace filter.
      StringAtomSet::iterator mi, me;
      for (mi = myobjs.begin(), me = myobjs.end(); mi != me; ++mi)
      {
	std::string trailer(mi->string(), StringOps::rfind(mi->string(), '/')+1);
	StringAtom label(&stree, "Quick collection/" + trailer);
	if (! rxsearch || rxsearch->search(label.string()) >= 0)
	{
	  shared_ptr<VisDQMItem> i(new VisDQMItem);
	  i->flags = 0;
	  i->version = 0;
	  i->name = label;
	  i->plotter = 0;
	  buildParentNames(i->parents, i->name);

	  VisDQMItems::iterator pos = contents.find(*mi);
	  if (pos != contents.end() && pos->second->layout)
	    i->layout = pos->second->layout;
	  else
	  {
	    shared_ptr<VisDQMLayoutRows> layout(new VisDQMLayoutRows);
	    shared_ptr<VisDQMLayoutRow> row(new VisDQMLayoutRow);
	    shared_ptr<VisDQMLayoutItem> column(new VisDQMLayoutItem);
	    column->path = *mi;
	    layout->push_back(row);
	    row->columns.push_back(column);
	    i->layout = layout;
	  }

	  contents[label] = i;
	}
      }
    }

  // Utility method to build a look-up table of shown objects.
  void
  buildShown(const VisDQMItems &contents,
	     StringAtomSet &shown,
	     const std::string &root)
    {
      std::string myroot;
      if (! root.empty())
      {
	myroot.reserve(root.size() + 2);
	myroot += root;
	myroot += '/';
      }

      VisDQMItems::const_iterator ci, ce;
      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	if (ci->first.string().size() > myroot.size()
	    && ci->first.string().compare(0, myroot.size(), myroot) == 0
	    && ci->first.string().find('/', myroot.size()) == std::string::npos)
	  shown.insert(ci->first);
    }

  // Update the object metrics for the items in @a contents.
  // "Shown" means the object is included in canvas listing.
  void
  updateStatus(const VisDQMItems &contents,
	       VisDQMStatusMap &status)
    {
      VisDQMStatus empty;
      empty.nleaves = 0;
      empty.nalarm = 0;
      empty.nlive = 0;

      status[StringAtom(&stree, "")] = empty;
      status[StringAtom(&stree, "Quick collection")] = empty;

      status.resize(10*contents.size());
      VisDQMItems::const_iterator ci, ce, lpos;
      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
      {
	// Initialise object data.
	VisDQMItem &o = *ci->second;
	VisDQMStatus &s = status[ci->first];
	s.nleaves = 1;
	s.nalarm = ((o.flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) ? 1 : 0);
	s.nlive = 1;

	// If this is a layout, resolve it.
	if (o.layout)
	{
	  VisDQMLayoutRows &rows = *o.layout;
	  for (size_t nrow = 0; nrow < rows.size(); ++nrow)
	  {
	    VisDQMLayoutRow &row = *rows[nrow];
	    for (size_t ncol = 0; ncol < row.columns.size(); ++ncol)
	    {
	      VisDQMLayoutItem &col = *row.columns[ncol];
	      if ((lpos = contents.find(col.path)) != contents.end())
		s.nalarm += ((lpos->second->flags & VisDQMIndex::SUMMARY_PROP_REPORT_ALARM) ? 1 : 0);
	      else if (! col.path.string().empty())
		s.nlive = 0;
	    }
	  }
	}

	// Update data to "virtual parents."
	for (size_t i = 0, e = o.parents.size(); i != e; ++i)
	{
	  VisDQMStatusMap::iterator pos = status.find(o.parents[i]);
	  if (pos == status.end())
	    pos = status.insert(VisDQMStatusMap::value_type(o.parents[i], empty)).first;
	  pos->second.nleaves += s.nleaves;
	  pos->second.nalarm += s.nalarm;
	  pos->second.nlive += s.nlive;
	}
      }
    }

private:
  VisDQMWorkspace(VisDQMWorkspace &);
  VisDQMWorkspace &operator=(VisDQMWorkspace &);
};

class VisDQMSummaryWorkspace : public VisDQMWorkspace
{
  typedef std::map<std::string, std::string> KeyValueMap;
  typedef std::map<std::string, KeyValueMap> SummaryMap;

  Regexp rxevi_;
public:
  VisDQMSummaryWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name),
      rxevi_("(.*?)/EventInfo/(.*)")
    {
      rxevi_.study();
    }

  virtual ~VisDQMSummaryWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample sample(sessionSample(session));
      std::string toolspanel(sessionPanelConfig(session, "tools"));
      std::string qplot(py::extract<std::string>(session.get("dqm.qplot", "")));
      std::string filter(py::extract<std::string>(session.get("dqm.filter")));
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string reference(sessionReference(session));
      std::string strip(sessionStripChart(session));
      std::string submenu(py::extract<std::string>(session.get("dqm.submenu")));
      std::string rxstr(py::extract<std::string>(session.get("dqm.search")));
      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		myobjs; // deliberately empty, not needed
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	RegexpMatch		m;
	std::string		plotter;
	SummaryMap		summary;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), 0, 0, 0);
	updateStatus(contents, status);

	for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  VisDQMItem &o = *ci->second;
	  if (! isScalarType(o.flags))
	    continue;

	  m.reset();
	  if (! rxevi_.match(o.name.string(), 0, 0, &m))
	    continue;

	  std::string system(m.matchString(o.name.string(), 1));
	  std::string param(m.matchString(o.name.string(), 2));

	  if (rxsearch && rxsearch->search(system) < 0)
	    continue;

	  // Alarm filter. Somewhat convoluted as we need to dig out the
	  // report summary value, and use the value known by JavaScript
	  // to check for items passing an alarm filter here.
	  if (qalarm)
	  {
	    StringAtom reportSummary(&stree,
				     system + "/EventInfo/reportSummary",
				     StringAtom::TestOnly);

	    VisDQMItems::iterator pos = contents.find(reportSummary);
	    if (pos != ce && isRealType(pos->second->flags))
	    {
	      double value = atof(pos->second->data.c_str());
	      if ((value >= 0 && value < 0.95) != alarms)
		continue;
	    }
	  }

	  KeyValueMap &info = summary[system];
	  if (info.empty())
	  {
	    plotter = StringFormat("%1/%2%3")
		      .arg(o.plotter ? o.plotter->plotter() : "unknown")
		      .arg(sample.runnr)
		      .arg(sample.dataset);
	    info["MEs"] = StringFormat("%1").arg(status[StringAtom(&stree, system)].nleaves);
	    info["processName"] = system;
	  }

	  info[param] = o.data;
	}

        return makeResponse(StringFormat("{'kind':'DQMSummary', 'items':[%1],"
                            		 " 'qrender':'%2', 'qplot':'%3'}")
			    .arg(summaryToJSON(summary))
			    .arg(plotter)
			    .arg(qplot),
                            sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, summary.size(),
                            toolspanel, startTime);
      }
    }

private:
  static std::string
  summaryToJSON(const SummaryMap &summary)
    {
      size_t len = 4 * summary.size();
      SummaryMap::const_iterator si, se;
      KeyValueMap::const_iterator ki, ke;
      for (si = summary.begin(), se = summary.end(); si != se; ++si)
      {
	len += 8 * si->second.size();
	for (ki = si->second.begin(), ke = si->second.end(); ki != ke; ++ki)
	  len += ki->first.size() + 2 * ki->second.size();
      }

      std::string result;
      result.reserve(len + 1);
      for (si = summary.begin(), se = summary.end(); si != se; ++si)
      {
	if (! result.empty())
	  result += ", ";
	result += "{";

	bool first = true;
	for (ki = si->second.begin(), ke = si->second.end(); ki != ke; ++ki, first = false)
	{
	  if (! first)
	    result += ", ";
	  result += stringToJSON(ki->first);
	  result += ":";
	  result += stringToJSON(ki->second);
	}

	result += "}";
      }

      return result;
    }
};

class VisDQMCertificationWorkspace : public VisDQMWorkspace
{
  typedef std::map<std::string, std::vector<std::string> > KeyVectorMap;

  Regexp rxevi_;
public:
  VisDQMCertificationWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name),
      rxevi_("(.*?)/EventInfo/(CertificationContents|DAQContents|DCSContents|reportSummaryContents)/(.*)")
    {
      rxevi_.study();
    }

  virtual ~VisDQMCertificationWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample sample(sessionSample(session));
      std::string toolspanel(sessionPanelConfig(session, "tools"));
      std::string zoom(sessionCertZoomConfig(session));
      std::string qplot(py::extract<std::string>(session.get("dqm.qplot", "")));
      std::string filter(py::extract<std::string>(session.get("dqm.filter")));
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string reference(sessionReference(session));
      std::string strip(sessionStripChart(session));
      std::string submenu(py::extract<std::string>(session.get("dqm.submenu")));
      std::string rxstr(py::extract<std::string>(session.get("dqm.search")));
      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		myobjs; // deliberately empty, not needed
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	RegexpMatch		m;
	std::string		plotter;
	KeyVectorMap		cert;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), 0, 0, 0);
	updateStatus(contents, status);

	for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  VisDQMItem &o = *ci->second;
	  if (! isScalarType(o.flags))
	    continue;

	  m.reset();
	  if (! rxevi_.match(o.name.string(), 0, 0, &m))
	    continue;

	  std::string system(m.matchString(o.name.string(), 1));
	  std::string kind(m.matchString(o.name.string(), 2));
	  std::string variable(m.matchString(o.name.string(), 3));

	  if (rxsearch && rxsearch->search(system) < 0)
	    continue;

	  std::vector<std::string> &info = cert[system];
	  if (info.empty())
	  {
	    plotter = StringFormat("%1/%2%3")
		      .arg(o.plotter ? o.plotter->plotter() : "unknown")
		      .arg(sample.runnr)
		      .arg(sample.dataset);
	  }
	  info.push_back(kind+"/"+variable);
	}

        return makeResponse(StringFormat("{\"kind\":\"DQMCertification\", \"items\":[%1],"
					 " \"current\":\"%2\", %3}")
			    .arg(certificationToJSON(cert))
			    .arg(plotter)
			    .arg(zoom),
			    sample.type == SAMPLE_LIVE ? 30 : 300, current,
			    services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, cert.size(),
                            toolspanel, startTime);
      }
    }

private:
  static std::string
  certificationToJSON(const KeyVectorMap &cert)
    {
      size_t len = 4 * cert.size();
      KeyVectorMap::const_iterator si, se;
      std::vector<std::string>::const_iterator vi, ve;
      for (si = cert.begin(), se = cert.end(); si != se; ++si)
      {
	len += 8 * si->second.size();
	for (vi = si->second.begin(), ve = si->second.end(); vi != ve; ++vi)
	  len += 2 * vi->size();
      }

      std::string result;
      result.reserve(len + 1);
      bool first = true;
      for (si = cert.begin(), se = cert.end(); si != se; ++si, first = false)
      {
	if (! first)
	    result += ", ";
	result += StringFormat("{ \"text\": \"%1\", \"children\": [")
		  .arg(si->first);
	std::string prevFolder = "";
	for (vi = si->second.begin(), ve = si->second.end(); vi != ve; ++vi)
	{
	  std::string validationFolder = vi->substr(0,vi->rfind('/'));
	  std::string variable = vi->substr(vi->rfind('/')+1, vi->size());
	  if (prevFolder.empty())
	  {
	    prevFolder = validationFolder;
	    result += StringFormat("{ \"text\": \"%1\", \"children\": [{\"text\": \"%2\", \"leaf\": true}")
		      .arg(validationFolder)
		      .arg(variable);
	  }
	  if (prevFolder != validationFolder)
	  {
	    prevFolder = validationFolder;
	    result += StringFormat("]}, { \"text\": \"%1\", \"children\": [{\"text\": \"%2\", \"leaf\": true}")
		      .arg(validationFolder)
		      .arg(variable);
	  }
	  else
	    result += StringFormat(",{\"text\": \"%1\", \"leaf\": true}")
		      .arg(variable);
	}
	result += "]}]}";
      }
      return result ;
    }
};

class VisDQMQualityWorkspace : public VisDQMWorkspace
{
  struct QMapItem
  {
    std::string location;
    std::string name;
    std::string	reportSummary;
    std::string eventTimeStamp;
    uint64_t version;
  };

  typedef std::map<std::string, QMapItem> QMap;
public:
  VisDQMQualityWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMQualityWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample sample(sessionSample(session));
      std::string toolspanel(sessionPanelConfig(session, "tools"));
      std::string filter(py::extract<std::string>(session.get("dqm.filter")));
      std::string showstats(py::extract<std::string>(session.get("dqm.showstats")));
      std::string showerrbars(py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string reference(sessionReference(session));
      std::string strip(sessionStripChart(session));
      std::string submenu(py::extract<std::string>(session.get("dqm.submenu")));
      std::string rxstr(py::extract<std::string>(session.get("dqm.search")));
      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		myobjs; // deliberately empty, not needed
	VisDQMItems::iterator	ci, ce;
	std::string		plotter;
	std::string		label;
	QMap			qmap;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), 0, 0, 0);

	for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
	{
	  VisDQMItem &o = *ci->second;
	  if (o.name.string().size() < 27)
	    continue;

	  size_t pos = o.name.string().size() - 27;
	  if (o.name.string().compare(pos, 27, "/EventInfo/reportSummaryMap") != 0)
	    continue;

	  if (! isScalarType(o.flags))
	  {
	    size_t start = o.name.string().rfind('/', pos ? pos-1 : 0);
	    if (start == std::string::npos) start = 0;
	    label = o.name.string().substr(start, pos-start);
	    if (rxsearch && rxsearch->search(label) < 0)
	      continue;

	    StringAtom reportSummary(&stree,
				     label + "/EventInfo/reportSummary",
				     StringAtom::TestOnly);

	    StringAtom eventTimeStamp(&stree,
				      label + "/EventInfo/eventTimeStamp",
				      StringAtom::TestOnly);

	    // Alarm filter. Somewhat convoluted as we need to dig out the
	    // report summary value, and use the value known by JavaScript
	    // to check for items passing an alarm filter here.
	    if (qalarm)
	    {
	      VisDQMItems::iterator pos = contents.find(reportSummary);
	      if (pos != ce && isRealType(pos->second->flags))
	      {
		double value = atof(pos->second->data.c_str());
		if ((value >= 0 && value < 0.95) != alarms)
		  continue;
	      }
	    }

	    QMapItem &i = qmap[label];
	    i.name = o.name.string();
	    i.version = o.version;
	    i.reportSummary.clear();
	    i.eventTimeStamp.clear();
	    i.location = StringFormat("%1/%2%3")
			 .arg(o.plotter ? o.plotter->plotter() : "unknown")
			 .arg(sample.runnr)
			 .arg(sample.dataset);

	    VisDQMItems::iterator other;
	    if ((other = contents.find(reportSummary)) != ce
		&& isRealType(other->second->flags))
	      i.reportSummary = other->second->data;

	    if ((other = contents.find(eventTimeStamp)) != ce
		&& isRealType(other->second->flags))
	      i.eventTimeStamp = other->second->data;
	  }
	}

        return makeResponse(StringFormat("{'kind':'DQMQuality', 'items':[%1]}")
			    .arg(qmapToJSON(qmap)),
			    sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, qmap.size(),
                            toolspanel, startTime);
      }
    }

private:
  static std::string
  qmapToJSON(const QMap &qmap)
    {
      size_t len = 30 * qmap.size();
      QMap::const_iterator i, e;
      for (i = qmap.begin(), e = qmap.end(); i != e; ++i)
	len += 100 + 2 * (i->first.size()
			  + i->second.location.size()
			  + i->second.name.size()
			  + i->second.reportSummary.size()
			  + i->second.eventTimeStamp.size());

      std::string result;
      result.reserve(len);
      for (i = qmap.begin(), e = qmap.end(); i != e; ++i)
      {
	char buf[32];
	__extension__
	  sprintf(buf, "%ju", (uintmax_t) i->second.version);
	if (! result.empty())
	  result += ", ";
	result += "{'label':";
	result += stringToJSON(i->first);
	result += ",'version':";
	result += buf;
	result += ",'name':";
	result += stringToJSON(i->second.name);
	result += ",'location':";
	result += stringToJSON(i->second.location);
	result += ",'reportSummary':";
	result += stringToJSON(i->second.reportSummary);
	result += ",'eventTimeStamp':";
	result += stringToJSON(i->second.eventTimeStamp);
	result += "}";
      }

      return result;
    }
};

class VisDQMSampleWorkspace : public VisDQMWorkspace
{
public:
  VisDQMSampleWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMSampleWorkspace(void)
    {}

  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      VisDQMSample sample(sessionSample(session));
      std::string vary(py::extract<std::string>(session.get("dqm.sample.vary")));
      std::string order(py::extract<std::string>(session.get("dqm.sample.order")));
      std::string dynsearch(py::extract<std::string>(session.get("dqm.sample.dynsearch")));
      std::string pat(py::extract<std::string>(session.get("dqm.sample.pattern")));

      // If the current sample is live, force varying off, otherwise
      // it's too hard for users to find anything else than the live.
      if (sample.type == SAMPLE_LIVE)
	vary = "any";

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	typedef std::set<std::string> MatchSet;
	typedef std::list< std::pair<std::string, shared_ptr<Regexp> > > RXList;

	PyReleaseInterpreterLock nogil;
	VisDQMSamples		samples;
	VisDQMSamples		final;
	StringList		pats;
	StringList		runlist;
	RXList			rxlist;
	std::string		rxerr;
	MatchSet		uqpats;
        bool			varyAny = (vary == "any");
	bool			varyRun = (vary == "run");
	bool			varyDataset = (vary == "dataset");
	std::string		samplebase;

	if (sample.type == SAMPLE_OFFLINE_RELVAL)
	  samplebase = StringOps::remove(sample.dataset, RX_CMSSW_VERSION);

	pats = StringOps::split(pat, Regexp::rxwhite, StringOps::TrimEmpty);
	runlist.reserve(pats.size());
	uqpats.insert(pats.begin(), pats.end());

	// Process space-separated patterns.  Split pure numbers to
	// run number criteria, and others to regexp criteria.  The
	// run number requires a prefix match on the number, not an
	// exact match, and we do the prefix comparison as strings.
	for (size_t i = 0, e = pats.size(); i != e; ++i)
	{
	  errno = 0;
	  char *p = 0;
	  strtol(pats[i].c_str(), &p, 10);
	  if (p && ! *p && ! errno)
	    runlist.push_back(pats[i]);

	  shared_ptr<Regexp> rx;
	  makerx(pats[i], rx, rxerr, Regexp::IgnoreCase);
	  if (rx)
	    rxlist.push_back(RXList::value_type(pats[i], rx));
	}

	// Request samples from backends.
	for (size_t i = 0, e = srclist.size(); i != e; ++i)
	  srclist[i]->samples(samples);

	// Filter out samples which do not pass search criteria.
	// Require all pattern components to match at least one
	// criteria (run number or something in the dataset name).
        final.reserve(samples.size());
	for (size_t i = 0, e = samples.size(); i != e; ++i)
	{
	  MatchSet matched;
	  StringList::iterator vi, ve;
	  StringList::iterator rli, rle;
	  RXList::iterator rxi, rxe;
	  VisDQMSample &s = samples[i];

	  if (varyAny)
	    /* keep all */;
          else if (s.type == SAMPLE_LIVE)
	    /* always provide link to live sample */;
	  else if (varyDataset)
	  {
	    if (sample.type == SAMPLE_OFFLINE_RELVAL)
	    {
	      // current sample has no version set, search dataset name.
	      if (s.type != sample.type
		  || sample.dataset.find(s.version) == std::string::npos)
	        continue;
	    }
	    else if (s.runnr != sample.runnr || s.version != sample.version)
	      continue;
          }
          else if (varyRun)
          {
	    if (sample.type == SAMPLE_OFFLINE_RELVAL)
	    {
	      if (s.type != sample.type
		  || samplebase != StringOps::remove(s.dataset, RX_CMSSW_VERSION))
		continue;
	    }
	    else if (s.dataset != sample.dataset)
	      continue;
	  }
	  else
	    continue;

	  for (rli = runlist.begin(), rle = runlist.end(); rli != rle; ++rli)
	  {
	    char buf[32];
	    sprintf(buf, "%ld", s.runnr);
	    if (! strncmp(buf, rli->c_str(), rli->size()))
	      matched.insert(*rli);
	  }

	  for (rxi = rxlist.begin(), rxe = rxlist.end(); rxi != rxe; ++rxi)
	    if (rxi->second->search(s.dataset) >= 0
		|| rxi->second->search(sampleTypeLabel[s.type]) >= 0)
	      matched.insert(rxi->first);

	  if (matched.size() == uqpats.size())
	    final.push_back(samples[i]);
	}

        std::sort(final.begin(), final.end(),
		  (order == "run" ? orderSamplesByRun
		   : orderSamplesByDataset));

	StringFormat result
	  = StringFormat("([{'kind':'AutoUpdate', 'interval':%1, 'stamp':%2, 'serverTime':%9},"
			 "{'kind':'DQMSample', 'vary':%3, 'order':%4, 'dynsearch':%5, 'search':%6, 'current':%7,"
			 " 'items':[%8]}])")
	  .arg(sample.type == SAMPLE_LIVE ? 30 : 300)
	  .arg(guiTimeStamp_, 0, 'f')
	  .arg(stringToJSON(vary))
	  .arg(stringToJSON(order))
	  .arg(stringToJSON(dynsearch))
	  .arg(stringToJSON(pat))
	  .arg(sampleToJSON(sample))
	  .arg(samplesToJSON(final));
	return result.arg((Time::current() - startTime).ns() * 1e-6, 0, 'f');
      }
    }

private:
};

class VisDQMContentWorkspace : public VisDQMWorkspace, public VisDQMLayoutTools
{
  VisDQMRegexp  rxmatch_;
  VisDQMRegexp	rxlayout_;
public:
  VisDQMContentWorkspace(py::object gui,
			 const std::string &name,
			 const std::string &match,
			 const std::string &layouts)
    : VisDQMWorkspace(gui, name)
    {
      if (! match.empty() && match != "^")
      {
        fastrx(rxmatch_, match);
        if (layouts.empty())
          fastrx(rxlayout_, "(" + match + "Layouts)/.*");
      }
      if (! layouts.empty() && layouts !="^")
        fastrx(rxlayout_, "(" + layouts + ")/.*");

    }

  virtual ~VisDQMContentWorkspace(void)
    {}

  // Helper function to present session state.  Returns a JSON object
  // representing current menu and canvas state.
  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample  sample       (sessionSample(session));
      std::string   toolspanel   (sessionPanelConfig(session, "tools"));
      std::string   helppanel    (sessionPanelConfig(session, "help"));
      std::string   custompanel  (sessionPanelConfig(session, "custom"));
      std::string   zoom         (sessionZoomConfig(session));
      std::string   root         (workspaceParam<std::string>(session, "dqm.root", ""));
      std::string   focus        (workspaceParam<std::string>(session, "dqm.focus", ""));  // None
      std::string   filter       (py::extract<std::string>(session.get("dqm.filter")));
      std::string   showstats    (py::extract<std::string>(session.get("dqm.showstats")));
      std::string   showerrbars  (py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string   reference    (sessionReference(session));
      std::string   strip	 (sessionStripChart(session));
      std::string   submenu      (py::extract<std::string>(session.get("dqm.submenu")));
      std::string   size	 (py::extract<std::string>(session.get("dqm.size")));
      std::string   rxstr        (py::extract<std::string>(session.get("dqm.search")));
      py::dict      pydrawopts   (workspaceParam<py::dict>(session, "dqm.drawopts", py::dict()));
      py::tuple     myobjst      (workspaceParam<py::tuple>(session, "dqm.myobjs", py::tuple()));
      StringAtomSet myobjs;

      for (py::stl_input_iterator<std::string> e, i(myobjst); i != e; ++i)
        myobjs.insert(StringAtom(&stree, *i));

      VisDQMDrawOptionMap drawopts;
      for (py::stl_input_iterator<std::string> i(pydrawopts), e; i != e; ++i)
	translateDrawOptions(py::extract<py::dict>(pydrawopts.get(*i)),
			     drawopts[StringAtom(&stree, *i)]);

      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		shown;
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	std::string		data;
	std::string		layoutroot;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample, current,
		      &rxmatch_, rxsearch.get(), qalarm,
		      &layoutroot, &rxlayout_);
	buildShown(contents, shown, root);
	updateStatus(contents, status);

	if (! status.count(StringAtom(&stree, root)))
	  root = ""; // FIXME: #36093

        return makeResponse(StringFormat("{'kind':'DQMCanvas', 'items':%1,"
                                         " 'root':%2, 'focus':%3, 'size':'%4', %5,"
                                         " %6, 'showstats': %7, 'showerrbars': %8, %9,"
                                         " 'reference':%10, 'strip':%11,"
                                         " 'layoutroot':%12}")
                            .arg(shownToJSON(contents, status, drawopts,
                                             StringAtom(&stree, root),
                                             sample, shown))
                            .arg(stringToJSON(root))
                            .arg(stringToJSON(focus, true))
                            .arg(size)
                            .arg(helppanel)
                            .arg(custompanel)
                            .arg(showstats)
                            .arg(showerrbars)
                            .arg(zoom)
                            .arg(reference)
                            .arg(strip)
                            .arg(stringToJSON(layoutroot)),
                            sample.type == SAMPLE_LIVE ? 30 : 300, current,
                            services, workspaceName_, workspaces,
                            submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, contents.size(),
                            toolspanel, startTime);
      }
    }
};

class VisDQMPlayWorkspace : public VisDQMWorkspace, public VisDQMLayoutTools
{
public:
  VisDQMPlayWorkspace(py::object gui, const std::string &name)
    : VisDQMWorkspace(gui, name)
    {}

  virtual ~VisDQMPlayWorkspace(void)
    {}

  // Helper function to present session state.  Returns a JSON object
  // representing current menu and canvas state.
  virtual std::string
  state(py::dict session)
    {
      Time startTime = Time::current();
      gui_.attr("_noResponseCaching")();
      std::vector<VisDQMSource *> srclist;
      sources(srclist);

      const std::string &services = serviceListJSON();
      const std::string &workspaces = workspaceListJSON();

      VisDQMSample  sample       (sessionSample(session));
      std::string   toolspanel   (sessionPanelConfig(session, "tools"));
      std::string   workspace    (py::extract<std::string>(session.get("dqm.play.prevws")));
      std::string   root         (workspaceParamOther<std::string>(session, "dqm.root", "", workspace));
      std::string   filter       (py::extract<std::string>(session.get("dqm.filter")));
      std::string   showstats    (py::extract<std::string>(session.get("dqm.showstats")));
      std::string   showerrbars  (py::extract<std::string>(session.get("dqm.showerrbars")));
      std::string   reference    (sessionReference(session));
      std::string   strip	 (sessionStripChart(session));
      std::string   submenu      (py::extract<std::string>(session.get("dqm.submenu")));
      std::string   rxstr        (py::extract<std::string>(session.get("dqm.search")));
      int           playpos      (py::extract<int>(session.get("dqm.play.pos")));
      int           playinterval (py::extract<int>(session.get("dqm.play.interval")));
      py::dict      pydrawopts   (workspaceParamOther<py::dict>(session, "dqm.drawopts", py::dict(), workspace));
      py::tuple     myobjst      (workspaceParamOther<py::tuple>(session, "dqm.myobjs", py::tuple(), workspace));
      StringAtomSet myobjs;

      for (py::stl_input_iterator<std::string> e, i (myobjst); i != e; ++i)
        myobjs.insert(StringAtom(&stree, *i));

      VisDQMDrawOptionMap drawopts;
      for (py::stl_input_iterator<std::string> i(pydrawopts), e; i != e; ++i)
	translateDrawOptions(py::extract<py::dict>(pydrawopts.get(*i)),
			     drawopts[StringAtom(&stree, *i)]);

      std::string rxerr;
      shared_ptr<Regexp> rxsearch;
      makerx(rxstr, rxsearch, rxerr, Regexp::IgnoreCase);

      // Give sources pre-scan warning so they can do python stuff.
      for (size_t i = 0, e = srclist.size(); i != e; ++i)
	srclist[i]->prescan();

      // Now do the hard stuff, out of python.
      {
	PyReleaseInterpreterLock nogil;

	VisDQMItems		contents;
	StringAtomSet		shown;
	VisDQMItems::iterator	ci, ce;
	VisDQMStatusMap		status;
	std::string		data;
	VisDQMEventNum		current = { "", -1, -1, -1, -1 };
	bool			alarms = false;
	bool			*qalarm = 0;

	if (filter == "all")
	  qalarm = 0;
	else if (filter == "alarms")
	  alarms = true, qalarm = &alarms;
	else if (filter == "nonalarms")
	  alarms = false, qalarm = &alarms;

	buildContents(srclist, contents, myobjs, sample,
		      current, 0, rxsearch.get(), qalarm, 0, 0);
	buildShown(contents, shown, root);
	updateStatus(contents, status);

	if (! status.count(StringAtom(&stree, root)))
	  root = ""; // FIXME: #36093

	if (shown.size())
	  playpos = std::min(playpos, int(shown.size())-1);
	else
	  playpos = 0;

        return makeResponse(StringFormat("{'kind':'DQMPlay', 'items':%1,"
					 " 'interval':%2, 'pos':%3, 'max':%4,"
					 " 'reference':%5, 'strip':%6, 'showstats': %7,"
                                         " 'showerrbars': %8}")
			    .arg(shownToJSON(contents, status, drawopts,
					     StringAtom(&stree, root),
					     sample, shown))
			    .arg(playinterval)
			    .arg(playpos)
			    .arg(shown.size())
			    .arg(reference)
			    .arg(strip)
                            .arg(showstats)
                            .arg(showerrbars),
			    300, current, services, workspace, workspaces,
			    submenu, sample, filter, showstats, showerrbars,
                            reference, strip, rxstr, rxerr, contents.size(),
                            toolspanel, startTime);
      }
    }
};
