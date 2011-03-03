/** A hidden internal workspace for making data sample selections.

    DQMSample displays is a temporary internal workspace into which
    user is transferred whenever they request to select a different
    sample.  Once the user selects another sample, or chooses to
    return without making a change, the view is restored to the
    workspace and settings which were interrupted.

    The workspace eliminates most of the header field except to show
    the currently selected data sample name and the run number if
    applicable.  It displays the list of available datasets and runs
    (for data) or CMSSW versions (for relvals) in the canvas area,
    preceded by a search field.  Datasets are grouped by type and
    ordered by the server.

    Change of the text in the search field causes a server side
    incremental search to narrow down the list of choices.  Each
    space-separated token in the search field is taken to be a
    criteria which is matched case-insensitively against the dataset
    name, and if the term is purely a number, against the run number.
    All the terms must match, so for example "cosmics relval" would
    narrow the list down to all datasets which have both "cosmics" and
    "relval" in their names. */
GUI.Plugin.DQMSample = new function() {
  /** Reference to this, for use in callbacks where "this" is
      ambiguous or at least not this closure. */
  var _self = this;

  /** Reference to the GUI main object, for making server calls. */
  var _gui = null;

  /** The DOM element for the canvas area where we display contents. */
  var _canvas = $('canvas');

  /** The DOM element for the main header. */
  var _header = $('header');

  /** The DOM element for the sub-header below the main header, where
      we show the current dataset and run number. */
  var _subheader = $('subheader');

  /** The DQM toolbar with all elements that pertain to search action. */
  var _optsarea		= $('canvas-opts');

  /** The DOM element for the main search field. */
  var _search = null;

  /** A time-out timer for auto-search.  Started when a key press or
      paste is received on the search field.  Once the timer expires
      will check if the search input field has changed and if so
      issues an incremental server side search. */
  var _searchDelay = null;

  /** The last search input field value seen.  Used by the search
      timer function to determine if a new search is needed. */
  var _searchString = "";

  /** The DOM element for showing the list of available datasets. */
  var _datasets = null;

  /** The DOM element for order by run. */
  var _runorder = null;

  /** The DOM element for order by dataset. */
  var _dsorder = null;

  /** The most recent server state. */
  var _data = null;

  /** Template how to represent a selected element. */
  var _selem = "<em style='color:#c30;background-color:#fff0f0'>";

  /** Template how to represent style for a selected element.  */
  var _selemstyle = 'color:#c30;background-color:#fff0f0';

  /** Object that contains the list of expanded items.  */
  var _expanded = {};

  // ----------------------------------------------------------------
  /** Attach this plug-in to the GUI display.  Fills the header and
      subheader document elements with choices appropriate to the
      sample selection, and creates the navigation document elements
      on the canvas. */
  this.attach = function(gui)
  {
    _gui = gui;
    _expanded = {'run': new Array(), 'dataset': new Array()};
    _optsarea.style.display = '';
    var tb = new Ext.Toolbar({id: 'canvas-submenu'});
    var rbMinWidth = 120;
    var lblStyle = {'font-weight': 'bold', color: '#00f'};
    tb.add({ text       : 'Search:',
	     xtype      : 'tbtext',
	     style      : lblStyle
	   },
	   { xtype           :'textfield',
	     enableKeyEvents : true,
	     id              : 'sample-search',
	     style           : {width: '100%'},
	     ctCls           : 'search-field'
	   }, '-',
	   { xtype      : 'tbtext',
	     text       : 'Vary By:',
	     style      : lblStyle
	   },
	   { xtype      : 'radio',
             name       : 'rb-vary',
	     inputValue : 'Any',
	     boxLabel   : 'Any',
	     id         : 'vary-any'
	   },
	   { xtype      : 'radio',
             name       : 'rb-vary',
	     inputValue : 'Run',
	     boxLabel   : 'Run',
	     ctCls      : 'search-tb-radiobtn',
             id         : 'vary-run'
	   },
	   { xtype      : 'radio',
             name       : 'rb-vary',
	     inputValue : 'Dataset',
	     boxLabel   : 'Dataset',
	     ctCls      : 'search-tb-radiobtn',
	     id         : 'vary-dataset',
	     checked    : true
	   }, '-',
	   { xtype      : 'tbtext',
	     text       : 'Order by:',
	     style      : lblStyle
	   },
	   { xtype      : 'radio',
	     name       : 'rb-order',
	     inputValue : 'Run',
	     boxLabel   : 'Run',
	     ctCls      : 'search-tb-radiobtn',
	     id         : 'order-run'
	   },
	   { xtype      : 'radio',
	     name       : 'rb-order',
	     inputValue : 'Dataset',
	     boxLabel   : 'Dataset',
	     ctCls      : 'search-tb-radiobtn',
	     checked    : true,
	     id         : 'order-dataset'
	   }, '-',
	   { xtype      : 'checkbox',
	     boxLabel   : 'Search while typing',
	     ctCls      : 'search-tb-radiobtn',
	     id         : 'dynamic-search',
	     checked    : true
	   }, '-',
	   { xtype      : 'button',
	     text       : 'Search',
	     id         : 'searchBtn',
	     handler    : function()
	     {
	       if (! $('dynamic-search').checked)
		 _self.search();
	     }
	   }, '-',
	   { xtype      : 'button',
	     text       : 'Help',
	     id         : 'helpBtn',
	     handler    : function(){window.open('https://twiki.cern.ch/twiki/bin/view/CMS/DQMGuiSearch', 'helpWindow');}
	   }
	  );

    tb.render('canvas-opts');
    tb.doLayout();

    // Attach action on radio buttons.
    var anyBtns = ['vary-any', 'vary-dataset', 'vary-run', 'order-run', 'order-dataset'];
    for (var i = 0; i < anyBtns.length; ++i)
    {
      var radio = $(anyBtns[i]);
      if (radio)
      {
	radio.onchange = function()
	{
	  return GUI.Plugin.DQMSample.modify(this.id.replace(/-/, '='));
	};
      }
    }
    $('dynamic-search').onchange = function()
    {
      return GUI.Plugin.DQMSample.modify('dynsearch=' + ($('dynamic-search').checked ? 'yes' : 'no'));
    };


    // Attach action to the search text field
    Ext.getCmp('sample-search').on('keyup', function(){if ($('dynamic-search').checked) return GUI.Plugin.DQMSample.startSearch();});

    _canvas.innerHTML = "<div align='center' style='margin:1em 1em'>"
      + "<div id='datasets' align='left'></div></div>";
    _search = $('sample-search');
    _datasets = $('datasets');
  };

  /** Detach this plug-in from the GUI display.  Clears the header,
      sub-header and canvas document elements from the objects
      #attach() added and clears closure references for garbage
      collection. */
  this.detach = function()
  {
    // Clear any 'heading' elements we added.
    for (var m, n = _header.firstChild; n; n = m)
    {
      m = n.nextSibling;
      if (n.className == "heading")
	_header.removeChild(n);
    }

    // Clear subheader entirely.
    while (_subheader.firstChild)
      _subheader.removeChild(_subheader.firstChild);

    // Clear the canvas.
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);

    // Clear and reset the search toolbar
    _optsarea.innerHTML = "";
    _optsarea.style.display = 'none';

    // Turn off searching if it was activated.
    _searchString = "";
    if (_searchDelay)
    {
      clearTimeout(_searchDelay);
      _searchDelay = null;
    }

    _data = null;
    _datasets = null;
    _expanded = null;
  };

  // ----------------------------------------------------------------
  /** Response callback to return without making any changes. */
  this.goback = function()
  {
    _gui.makeCall(_url() + "/return");
    return false;
  };

  /** Response callback to make a dataset selection. */
  this.select = function(type, dsname, run)
  {
    _gui.makeCall(_url() + "/select?type="
		  + encodeURIComponent(type) + ";dataset="
		  + encodeURIComponent(dsname) + ";runnr="
		  + encodeURIComponent(run));
    return false;
  };

  /** Response callback triggered by changes to the search input.
      Triggers the actual search response 50ms after the last
      change has happened. */
  this.startSearch = function()
  {
    if (_searchDelay)
      clearTimeout(_searchDelay);
    _searchDelay = setTimeout(_self.search, 150);
    return true;
  };

  /** Response callback for actually executing a search; user is no
      longer immediately modifying the search field.  Issues a server
      side search request with the current search pattern.

      Note that #update() will refrain from updating the search field
      value if the user has continued to edit the search field between
      the time the request is submitted here and the server responds.
      In that case we'll automatically issue another search shortly
      afterwards. */
  this.search = function()
  {
    if (_searchString != _search.value)
    {
      _searchString = _search.value;
      _gui.makeCall(_url() + "/modify?pat=" + encodeURIComponent(_search.value));
    }
  };

  /** Response callback for changing listing options. */
  this.modify = function(arg)
  {
    _gui.makeCall(_url() + "/modify?" + arg);
    return false;
  };

  // ----------------------------------------------------------------
  /** Update this plug-in display.  Updates the menu areas and the
      canvas navigation area to the current state. */
  this.update = function(data)
  {
    _data = data;

    // Fill in the sub-header area with current sample identification.
    var type = "";
    var cur = data.current;
    var runid = ", Run " + cur.run;
    var dsname = _sanitise(cur.dataset);

    if (cur.type == "live")
      type = "Live", dsname = runid = "";
    else if (cur.type == "online_data")
      type = "Online", dsname = "";
    else if (cur.type == "offline_data")
      type = "Offline: ";
    else if (cur.type == "offline_relval")
      type = "RelVal: ", runid = "";
    else if (cur.type == "offline_mc")
      type = "MC: ", runid = "";
    else
      type = "Other: ";

    var content = "<span class='item'>" + type + dsname + runid
		  + "</span><span class='item'><a href='#' onclick='"
		  + "return GUI.Plugin.DQMSample.goback()'>"
		  + "(Back)</a></span>";

    if (_subheader.innerHTML != content)
      _subheader.innerHTML = content;


    /** Update canvas position taking into account header dynamic
        size. Do that only after we have inserted additional text to the
        head menu, since this changes the size of the header itself.  */
    $('canvas-group').style.top = $('header').offsetHeight + 'px';

    // Update the search value field *iff* the value is different and
    // the user didn't edit the value since the server call.
    if (! _searchDelay && _search.value != data.search)
      _search.value = _searchString = data.search;

    // Update highlight on search criteria.
    $('vary-'+data.vary).checked = true;
    $('order-'+data.order).checked = true;

    Ext.getCmp('dynamic-search').suspendEvents();
    Ext.getCmp('dynamic-search').setValue((data.dynsearch == 'yes') ? true : false);
    Ext.getCmp('dynamic-search').resumeEvents();

    // Build the dataset area contents and update if changed.
    content = this.display(cur, data.items, data.order);
    if (_datasets.innerHTML != content)
      _datasets.innerHTML = content;
  };

  this.display = function(cur, items, order)
  {
    // Build the dataset area listing, grouping datasets by sample
    // category, then sample name (folding similar relval names), and
    // then by run (for data) or CMSSW version (for relval).  Note
    // that simulated and live samples will have restricted choices.
    var content = "";
    var byrun = (order == "run");
    for (var i = 0, e = items.length; i < e; ++i)
    {
      var g = items[i];
      var sel = (g.type == cur.type);
      content += "<div style='clear:both;font-weight:bold'>" + (sel ? _selem : "");

      if (g.type == "live")
	content += "<a href='#' onclick='return GUI.Plugin.DQMSample.select(\""
		   + g.type + "\", \"" + g.items[0].dataset + "\", "
		   + g.items[0].run + ")'>Live</a>";
      else if (g.type == "online_data")
	content += "Online";
      else if (g.type == "offline_data")
	content += "Offline";
      else if (g.type == "offline_relval")
	content += "RelVal";
      else if (g.type == "offline_mc")
	content += "MC";
      else
	content += "Other";

      content += (sel ? "</em>" : "") + "</div>";

      if (order == "run")
        content += this.byrun(cur, g.items, g.type);
      else
        content += this.bydataset(cur, g.items, g.type);
    }

    return content;
  };

  this.bydataset = function(cur, items, type)
  {
    /* Greatly improve speed of this function by partitioning the
       string over which the expensive replace() method is called:
       used to be _content_, which contains the complete html page to
       be rendered; now it is _singleDatasetContent_, which contains
       at most the HTML snippet for a single dataset.
       _singleDatasetContent_ must be properly reset every time a
       transition to a new dataset happens. The same comment applies
       to the byrun() method.*/

    var content = "";
    var singleDatasetContent = "";
    var prev = null;
    var expanded = false;
    var prevexpanded = false;
    var runsInSingleDS = 0;

    for (var j = 0, k = items.length; j < k; ++j)
    {
      var s = items[j];
      var dataset = s.dataset;
      var run = s.run;
      var version = s.version;
      var ctype = s.type;
      var selds = (cur.type == ctype && cur.dataset == dataset);
      var selrun = (selds && cur.run == run);
      var selver = (selds && cur.version == version);

      expanded = false;
      prevexpanded = false;
      for (var i = 0, e = _expanded.dataset.length; i < e; ++i)
      {
	if (dataset == _expanded.dataset[i])
	  expanded = true;
	if (prev != null && prev == _expanded.dataset[i])
	  prevexpanded = true;
      }

      if (prev != null && dataset != prev)
      {
	singleDatasetContent += prevexpanded ? "</div></div>" : "</div>";
	content += singleDatasetContent.replace(/#TOBEREPLACED/, runsInSingleDS);
	singleDatasetContent = "";
	runsInSingleDS = 0;
      }

      if (prev == null || dataset != prev)
      {
	content += singleDatasetContent.replace(/#TOBEREPLACED/, runsInSingleDS);
	runsInSingleDS = 0;
	singleDatasetContent = "";
	prev = dataset;

	if (type == "live")
	  prev = null;
	else
	  singleDatasetContent += _("<div style='clear:both;margin:0 2em'>"
				    + "<div onclick='return"
				    + " GUI.Plugin.DQMSample.expand(this,\"${thetype}\", \"${dshtml}\", \"dataset\")'"
				    + " style='cursor:pointer; ${style}' > ${label}: (#TOBEREPLACED) </div>",
				    { dshtml: _sanitise(dataset),
					style: (selds ? _selemstyle : ''),
					label: (type == "online_data" ? "Online Runs" :_sanitise(dataset)),
					thetype: type});
	if (expanded)
	  singleDatasetContent += "<div>";
      }

      // Update this element in case this sample is in the expanded's list
      if (run > 1 || version != "")
	if (expanded)
	  singleDatasetContent += this.addContent(cur, s);
      runsInSingleDS++;
    }

    /* Last element inside a type does not trigger any replace action
       to properly display the number of items it contains: trigger
       it by default here. */

    content += singleDatasetContent.replace(/#TOBEREPLACED/, runsInSingleDS);

    if (prev != null)
      content += "</div>";

    // In case the last element inside a type was expanded, we have to
    // manually take care of closing the additional <div> element that
    // contains its links.
    if (expanded)
      content += "</div>";

    return content;
  };

  this.byrun = function(cur, items, type)
  {
    var content = "";
    var singleRunContent = "";
    var prev = null;
    var expanded = false;
    var prevexpanded = false;
    var itemsInKey = 0;

    for (var j = 0, k = items.length; j < k; ++j)
    {
      var s = items[j];
      var dataset = dataset;
      var run = s.run;
      var version = s.version;
      var ctype = s.type;

      var key = (type == "offline_data" ? run
		 : (type == "offline_relval" ? version : type));
      var selrun = (cur.type == ctype && cur.run == run);
      var selver = (cur.type == ctype && cur.version == version);
      var selds = (cur.type == ctype
		   && cur.dataset == dataset
		   && ((run > 1 && selrun)
		       || (version != "" && selver)
		       || (run == 1 && version == "")));

      expanded = false;
      prevexpanded = false;
      for (var i = 0, e = _expanded.run.length; i < e; ++i)
      {
	if (key == _expanded.run[i])
	  expanded = true;
	if (prev != null && prev == _expanded.run[i])
	  prevexpanded = true;
      }

      if (prev != null && key != prev)
      {
	singleRunContent += prevexpanded ? "</div></div>" : "</div>";
	content += singleRunContent.replace(/#TOBEREPLACED/, itemsInKey);
	singleRunContent = "";
	itemsInKey = 0;
      }

      if (prev == null || key != prev)
      {
	prev = key;
	itemsInKey = 0;
	singleRunContent = "";

	if (type == "live")
	  prev = null;
	else
	  singleRunContent += _("<div style='clear:both;margin:0 2em'>"
				    + "<div onclick='return"
				    + " GUI.Plugin.DQMSample.expand(this,\"${thetype}\", \"${ckey}\", \"run\")'"
				    + " style='cursor:pointer ${style}' >${label} (#TOBEREPLACED):</div>",
				    { ckey: _sanitise(key),
					style: (selrun ? _selemstyle : ''),
					label: (type == 'online_data' ? 'Online Runs' :
						( type == "offline_mc" ? 'Monte Carlo' : key)) ,
					thetype: type});
	if (expanded)
	  singleRunContent += "<div>";
      }

      // Update this element in case this key is in the expanded's list
      if (expanded)
	singleRunContent += this.addContentByRun(cur, s, type);
      itemsInKey++;
    }

    /* Last element inside a type does not trigger any replace action
       to properly display the number of items it contains: trigger
       it by default here. */

    content += singleRunContent.replace(/#TOBEREPLACED/, itemsInKey);

    if (prev != null)
      content += "</div>";

    /* In case the last element inside a type was expanded, we have to
       manually take care of closing the additional <div> element that
       contains its links.  */
    if (expanded)
      content += "</div>";

    return content;
  };

  this.expand = function(el,type,key, by)
  {
    var cur = _data.current;

    for (var i = 0, k = _expanded[by].length; i < k; ++i)
    {
      if (key == _expanded[by][i])
      {
	if (el.parentNode.lastChild)
	  el.parentNode.removeChild(el.parentNode.lastChild);
	// Remove elements from the list of expanded ones.
	_expanded[by].splice(i,1);
	return;
      }
    }

    // Add ket to the list of expanded ones.
    _expanded[by].push(key);

    var divContainer = document.createElement("div");
    var container = el.parentNode.appendChild(divContainer);
    container.style.display = 'visible';
    var content = '';
    for (i = 0, k = _data.items.length; i < k; ++i)
    {
      if (type == _data.items[i].type)
      {
	for (var j = 0, l = _data.items[i].items.length; j < l; ++j)
        {
	  var s = _data.items[i].items[j];
	  var compareTo = (by == 'dataset' ? s.dataset
			   : (type == "offline_data" ? s.run
			   : (type == "offline_relval" ? s.version : type)));
	  if (key == compareTo)
	    content += (by == 'run' ? this.addContentByRun(cur, s, type) : this.addContent(cur, s));
	}
      }
    }
    container.innerHTML = content;
  };

  this.addContent = function(cur, s)
  {
    var result = '';
    var template = "<a href='#' style='display:block;float:left;"
      + "margin-right:1em;min-width:5em; ${style}' onclick='return"
      + " GUI.Plugin.DQMSample.select"
      + "(\"${type}\", \"${dsname}\", ${run})'>"
      + "${val}</a>";
    var selds = (cur.type == s.type && cur.dataset == s.dataset);
    var selrun = (selds && cur.run == s.run);
    var selver = (selds && cur.version == s.version);
    result += _(template,
 	    	{ type: s.type, run: s.run, dsname: s.dataset,
		  val: (s.type.match(/.*relval.*/) ? s.version.replace(/^CMSSW_/, "") : s.run),
 		  style: (selrun ? _selemstyle : "")});
    return result;
  };

  this.addContentByRun = function(cur, s, type)
  {
    var result = '';
    var key = (type == "offline_data" ? s.run
	       : (type == "offline_relval" ? s.version : type));
    var selrun = (cur.type == s.type && cur.run == s.run);
    var selver = (cur.type == s.type && cur.version == s.version);
    var selds = (cur.type == s.type
		 && cur.dataset == s.dataset
		 && ((s.run > 1 && selrun)
		     || (s.version != "" && selver)
		     || (s.run == 1 && s.version == "")));
    var template
      = (type == "live" ? ""
	 : (type == "online_data"
	    ? "<a href='#' style='display:block;float:left;"
	    + "margin-right:1em;min-width:5em' onclick='return GUI.Plugin"
	    + ".DQMSample.select(\"${type}\", \"${dsname}\", ${run})'>"
	    + "${begin}${val}${end}</a>"
	    : "<div style='margin-left:1em'><a href='#'"
	    + " onclick='return GUI.Plugin.DQMSample.select"
	    + "(\"${type}\", \"${dsname}\", ${run})'>${begin}${val}${end}"
	    + "</a></div>"));

    var opts = { type: s.type, run: s.run, dsname: s.dataset,
		 val: (type.indexOf("offline_") >= 0 ? _sanitise(s.dataset) : s.run),
		 begin: (selds ? _selem : ""),
		 end: (selds ? "</em>" : "") };
    result += _(template, opts);

    return result;
  };

  return this;
}();
