class VisDQMLayoutTools
{
protected:
  static std::string
  axisToJSON(const VisDQMAxisOptions &axis)
    {
      return StringFormat("{'min':%1, 'max':%2, 'type':%3}")
	.arg(stringToJSON(axis.min, true))
	.arg(stringToJSON(axis.max, true))
	.arg(stringToJSON(axis.type));
    }

  static std::string
  layoutToJSON(const VisDQMShownItem &x, const VisDQMSample &sample)
    {
      char vbuf[64]; __extension__ sprintf(vbuf, "%ju", (uintmax_t) x.version);
      return StringFormat("{'name':%1, 'desc':%2, 'location':\"%3\","
			  " 'version':\"%4\", 'alarm':%5, 'live':%6,"
			  " 'xaxis':%7, 'yaxis':%8, 'zaxis':%9,"
			  " 'drawopts':%10, 'withref':%11, 'overlays':%12}")
          .arg(stringToJSON(x.name.string(), true))
          .arg(stringToJSON(x.desc, true))
          .arg(StringFormat("%1/%2%3")
               .arg(x.plotter ? x.plotter->plotter() : "unknown")
               .arg(sample.runnr)
               .arg(sample.dataset))
          .arg(vbuf)
          .arg(x.nalarm)
          .arg(x.nlive)
          .arg(axisToJSON(x.drawopts.xaxis))
          .arg(axisToJSON(x.drawopts.yaxis))
          .arg(axisToJSON(x.drawopts.zaxis))
          .arg(stringToJSON(x.drawopts.drawopts))
          .arg(stringToJSON(x.drawopts.withref))
          .arg(stringsToJSON(x.overlays));
    }

  static std::string
  layoutToJSON(const VisDQMShownRow &layout, const VisDQMSample &sample)
    {
      VisDQMShownItems::const_iterator i, e;
      std::string result;
      result.reserve(2048);
      result += '[';
      for (size_t i = 0, e = layout.columns.size(); i != e; ++i)
      {
	if (i > 0)
	  result += ", ";
	result += layoutToJSON(*layout.columns[i], sample);
      }
      result += ']';
      return result;
    }

  static std::string
  layoutToJSON(const VisDQMShownRows &layout, const VisDQMSample &sample)
    {
      VisDQMShownRows::const_iterator i, e;
      std::string result;
      result.reserve(2048);
      result += '[';
      for (size_t i = 0, e = layout.size(); i != e; ++i)
      {
	if (i > 0)
	  result += ", ";
	result += layoutToJSON(*layout[i], sample);
      }
      result += ']';
      return result;
    }

  static std::string
  shownToJSON(const VisDQMItems &contents,
	      const VisDQMStatusMap &status,
	      const VisDQMDrawOptionMap &drawopts,
	      const StringAtom &path,
	      const VisDQMSample &sample,
	      const StringAtomSet &shown)
    {
      std::string prefix;
      if (! path.string().empty())
      {
	prefix.reserve(path.size() + 2);
	prefix += path.string();
	prefix += '/';
      }

      StringAtomList names;
      StringAtomSet subdirs;
      VisDQMItems::const_iterator ci, ce;
      for (ci = contents.begin(), ce = contents.end(); ci != ce; ++ci)
      {
	size_t slash;
	const std::string &name = ci->first.string();
	if (name.size() > prefix.size()
	    && name.compare(0, prefix.size(), prefix) == 0
	    && (slash = name.find('/', prefix.size())) != std::string::npos)
	  subdirs.insert(StringAtom(&stree, std::string(name, 0, slash)));
      }

      std::string result;
      result.reserve(102400);
      result += '[';

      names.insert(names.end(), subdirs.begin(), subdirs.end());
      std::sort(names.begin(), names.end(), natcmpstratom);
      for (size_t i = 0, e = names.size(); i < e; ++i)
      {
	if (result.size() > 1)
	  result += ", ";

	result += shownDirToJSON(contents, status, names[i]);
      }

      names.clear();
      names.insert(names.end(), shown.begin(), shown.end());
      std::sort(names.begin(), names.end(), natcmpstratom);
      for (size_t i = 0, e = names.size(); i < e; ++i)
      {
	if (result.size() > 1)
	  result += ", ";
	result += shownObjToJSON(contents, status, drawopts, sample, names[i]);
      }

      result += ']';
      return result;
    }

  static std::string
  shownDirToJSON(const VisDQMItems & /* contents */,
		 const VisDQMStatusMap &status,
		 const StringAtom &x)
    {
      assert(status.count(x));
      const VisDQMStatus &info = status.find(x)->second;
      return StringFormat("{'name':%1, 'dir':{'nalarm':%2, 'nlive':%3, 'nleaves':%4}}")
	.arg(stringToJSON(x.string(), true))
	.arg(info.nalarm)
	.arg(info.nlive)
	.arg(info.nleaves);
    }

  static std::string
  shownObjToJSON(const VisDQMItems &contents,
		 const VisDQMStatusMap &status,
		 const VisDQMDrawOptionMap &drawopts,
		 const VisDQMSample &sample,
		 const StringAtom &x)
    {
      VisDQMShownRows layout;
      layoutOne(layout, contents, status, drawopts, x);
      return StringFormat("{'name':%1, 'items':%2}")
	.arg(stringToJSON(x.string()))
	.arg(layoutToJSON(layout, sample));
    }

  static void
  copyAxisOptions(VisDQMAxisOptions &to, const VisDQMAxisOptions &from)
    {
      if (! from.min.empty())
	to.min = from.min;

      if (! from.max.empty())
	to.max = from.max;

      if (! from.type.empty())
	to.type = from.type;
    }

  static void
  copyDrawOptions(VisDQMDrawOptions *to, const VisDQMDrawOptions *from)
    {
      if (! from)
	return;

      if (! from->withref.empty())
	to->withref = from->withref;

      if (! from->drawopts.empty())
	to->drawopts = from->drawopts;

      copyAxisOptions(to->xaxis, from->xaxis);
      copyAxisOptions(to->yaxis, from->yaxis);
      copyAxisOptions(to->zaxis, from->zaxis);
    }

  static void
  setDrawOptions(VisDQMDrawOptions *to,
		 const VisDQMDrawOptions *opt1 = 0,
		 const VisDQMDrawOptions *opt2 = 0)
    {
      to->withref = "def";
      to->drawopts = "";
      to->xaxis.type = "def";
      to->yaxis.type = "def";
      to->zaxis.type = "def";
      copyDrawOptions(to, opt1);
      copyDrawOptions(to, opt2);
    }

  // Build a layout on this node, 1x1 if it's not a layout otherwise.
  static void
  layoutOne(VisDQMShownRows &layout,
	    const VisDQMItems &contents,
	    const VisDQMStatusMap &status,
	    const VisDQMDrawOptionMap &drawopts,
	    const StringAtom &path)
    {
      VisDQMItems::const_iterator          objpos = contents.find(path);
      VisDQMStatusMap::const_iterator      statuspos = status.find(path);

      assert(objpos != contents.end());
      assert(objpos->second->name == path);
      assert(statuspos != status.end());

      const VisDQMItem &obj = *objpos->second;
      const VisDQMStatus &info = statuspos->second;

      if (! obj.layout)
      {
	VisDQMDrawOptionMap::const_iterator drawpos = drawopts.find(path);
	const VisDQMDrawOptions *xopts = 0;
	if (drawpos != drawopts.end())
	  xopts = &drawpos->second;

	shared_ptr<VisDQMShownItem> item(new VisDQMShownItem);
	item->version = obj.version;
	item->nalarm = info.nalarm;
	item->nlive = info.nlive;
	item->name = obj.name;
	item->plotter = obj.plotter;
	setDrawOptions(&item->drawopts, xopts);
	shared_ptr<VisDQMShownRow> row(new VisDQMShownRow);
	row->columns.push_back(item);
	layout.push_back(row);
      }
      else
      {
	const VisDQMLayoutRows &rows = *obj.layout;
	layout.reserve(rows.size());

	for (size_t nrow = 0; nrow < rows.size(); ++nrow)
	{
	  const VisDQMLayoutRow &row = *rows[nrow];
	  shared_ptr<VisDQMShownRow> srow(new VisDQMShownRow);
	  srow->columns.reserve(row.columns.size());
	  layout.push_back(srow);

	  for (size_t ncol = 0; ncol < row.columns.size(); ++ncol)
	  {
	    const VisDQMDrawOptions *xopts = 0;
	    const VisDQMDrawOptions *lopts = 0;
	    const VisDQMLayoutItem &col = *row.columns[ncol];
	    shared_ptr<VisDQMShownItem> scol(new VisDQMShownItem);

	    if ((objpos = contents.find(col.path)) != contents.end())
	    {
	      statuspos = status.find(col.path);
	      assert(statuspos != status.end());

	      const VisDQMItem &lobj = *objpos->second;
	      const VisDQMStatus &linfo = statuspos->second;
	      assert(lobj.name == col.path);

	      scol->version = lobj.version;
	      scol->nalarm = linfo.nalarm;
	      scol->nlive = linfo.nlive;
	      scol->name = col.path;
	      scol->desc = col.desc;
	      scol->plotter = lobj.plotter;
              scol->overlays = col.overlays;
	      lopts = &col.drawopts;

	      VisDQMDrawOptionMap::const_iterator drawpos = drawopts.find(col.path);
	      if (drawpos != drawopts.end())
		xopts = &drawpos->second;
	    }
	    else
	    {
	      scol->version = 0;
	      scol->nalarm = 0;
	      scol->nlive = 1;
	      scol->name = col.path;
	      scol->desc = col.desc;
	      scol->plotter = 0;
	    }

	    setDrawOptions(&scol->drawopts, lopts, xopts);
	    srow->columns.push_back(scol);
	  }
	}
      }
    }
};
