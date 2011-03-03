/** The menu on the top of the screen in the DQM GUI.

    DQMHeaderRow shows basic interactive status information and
    messages.  It provides short cut access to the other DQM GUI
    instances in the same "ring", the workspaces, and to the data
    samples by run and by dataset.  It shows the current run, lumi
    section and event numbers and various messages.  It provides
    general options such as a search and an alarm filter.

    Note the "current run number" shown at the top of the menu should
    not be assumed to be the "run number" of the current data sample.
    For one the "current numbers" are pretty-formatted to include
    thousand separators and cannot be compared with numerical values.
    For another the values contain text such as "(None)" when the
    values could not be determined or "(Simulated)" for Monte Carlo
    samples which do not have run numbers.  Lastly, the online live
    does not have an intrinsic run number, and the "current run" may
    be text such as "(Waiting)" if the run number could not be
    determined. */
GUI.Plugin.DQMHeaderRow = new function() {
  /** Reference for this, for use in callbacks where "this" is
      ambiguous or at least not this closure. */
  var _self = this;

  /** Reference to the GUI main object, for making server calls. */
  var _gui = null;

  /** Data from the last update.  Used to fill in the interaction area
      with a menu of the user's choice (data, services or workspaces)
      without having to make another round trip server call. */
  var _data = null;

  /** The DOM element for the main header. */
  var _header = $('header');

  /** The DOM element for the sub-header below the main header, where
      we show three interaction menus: data, services or
      workspaces. */
  var _subheader = $('subheader');

  /** The DOM element for the service main menu.  Points to the 'span'
      element to be filled in with the current service name. */
  var _service = null;

  /** The DOM element for the current run number in the main menu.
      Points to the 'span' element to be filled in with the value. */
  var _runNr = null;

  /** The DOM element for the current lumi number in the main menu.
      Points to the 'span' element to be filled in with the value. */
  var _lumiNr = null;

  /** The DOM element for the current event number in the main menu.
      Points to the 'span' element to be filled in with the value. */
  var _eventNr = null;

  /** The DOM element for the current run start in the main menu.
      Points to the 'span' element to be filled in with the value. */
  var _runStart = null;

  /** The DOM element for the current workspace in the main menu.
      Points to the 'span' element to be filled in with the name. */
  var _workspace = null;

  /** The DOM element for data operations sub-header menu.  Points to
      the 'div' element containing the menu.  The menu is shown by
      setting its style.display to '' and style.display of the other
      menus to 'none'. */
  var _menuData = null;

  /** The DOM element for service choice sub-header menu.  Points to
      the 'div' element containing the menu.  The menu is shown by
      setting its style.display to '' and style.display of the other
      menus to 'none'. */
  var _menuService = null;

  /** The DOM element for workspace choice sub-header menu.  Points to
      the 'div' element containing the menu.  The menu is shown by
      setting its style.display to '' and style.display of the other
      menus to 'none'. */
  var _menuWorkspace = null;

  /** The DOM element for the search input field in the data menu. */
  var _search = $('subhead-search');

  /** The DOM element for the search result in the data menu. */
  var _searchResult = $('subhead-searchres');

  /** A time-out timer for auto-search.  Started when a key press or
      paste is received on the search field.  Once the timer expires
      will check if the search input field has changed and if so
      issues an incremental server side search. */
  var _searchDelay = null;

  /** The last search input field value seen.  Used by the search
      timer function to determine if a new search is needed. */
  var _searchString = "";

  /** A time-out timer for reference changes.  Started when a key
      press or paste is received on reference input fields.  Issues
      server request to update settings once the timer expires. */
  var _refDelay		= null;

  /** The DOM element for strip chart type select field. */
  var _striptype        = $('subhead-strip-type');

  /** The DOM element for strip chart run count input field. */
  var _stripn           = $('subhead-strip-n');

  /** The DOM element for strip chart axis field. */
  var _stripaxis        = $('subhead-strip-axis');

  /** The DOM element for strip chart omit field. */
  var _stripomit        = $('subhead-strip-omit');

  /** Mappings from string value to option index. */
  var _STRIPTYPE	= _optionmap(_striptype);
  var _STRIPAXIS	= _optionmap(_stripaxis);
  var _STRIPOMIT	= _optionmap(_stripomit);
  var _REFERENCEPOS     = _optionmap($('subhead-ref-position'));
  var _REFERENCESHOW    = _optionmap($('subhead-ref-show'));

  /** A time-out timer for strip chart changes.  Started when a key
      press or paste is received on strip chart run field, or the
      type of strip object is changed.  Issues server request to
      change update settings once the timer expires. */
  var _stripDelay	= null;

  /** The DOM element for the header tools (view settings) panel. */
  var _toolsgroup	= $('header-tools-group');

  /** The DOM element for the actual body of the tools panel. */
  var _toolsarea	= $('header-tools');

  /** The YUI panel widget for tools panel. */
  var _toolspanel	= null;

  /** Flag to indicate that in-browser view of the tools panel is
      more accurate than servers; server-provided panel updates as
      part of the plug-in state update are ignored if this is true. */
  var _toolslocal	= false;

  /** Currently displayed extra menu in the sub-header area. */
  var _curmenu          = null;

  // ----------------------------------------------------------------
  /** Return list of current workspaces, for use by other modules. */
  this.workspaces = function()
  {
    return _data.workspaces;
  };

  // ----------------------------------------------------------------
  /** Utility method to create a new top level heading with @a label
      as the title above and @a content as the contents below.

      If @a link is non-empty, the whole heading element is made a
      link which will call DQMHeaderRow with the method name and
      parameters specified in link as a text string; a link
      'foo("arg")' would call DQMHeaderRow.foo("arg") when the header
      element is clicked.  As usual with links, the method should
      return @c false to prevent link traversal and should not make
      assumptions about @c this.

      Returns the 'h2' DOM element which contains the @a contents.  */
  this.heading = function(label, content, link)
  {
    var linkattr = (link ? " onclick='return GUI.Plugin.DQMHeaderRow." + link + "'" : "");
    var linkstart = (link ? ("<a href='#'" + linkattr + ">") : "");
    var linkend = (link ? ("<img src='" + ROOTPATH + "/static/down-menu.gif'"
      + "style='margin-left:5px;padding-bottom:1px' width='7' height='7' /></a>") : "");

    var div = document.createElement("div");
    _header.insertBefore(div, _subheader);

    div.className = "heading";
    if (link)
      div.style.cursor = "pointer";

    div.innerHTML = ("<div class='label'>"
		     + linkstart + label + linkend + "</div>"
		     + "<h2" + linkattr + ">" + content + "</h2>");

    return div.lastChild;
  };

  // ----------------------------------------------------------------
  /** Attach this plug-in to the GUI display. Fills the header and
      sub-header document elements with the main and secondary
      menus. */
  this.attach = function(gui)
  {
    _gui = gui;
    _gui.setIdleMessage
      ("<span style='font-size: 95%'>"
       + "Please file any feature requests and any bugs you find in <a href='"
       + "https://savannah.cern.ch/bugs/?group=iguana'>Savannah</a>. Find "
       + "<a href='https://twiki.cern.ch/twiki/bin/view/CMS/DQMShiftInstructions'>"
       + "shift instructions here</a>.</span>");
    _service = this.heading("Service", "<span>_</span>:", "pickMenu(\"service\")").firstChild;
    _workspace = this.heading("<img src='" + ROOTPATH + "/static/previous.png'"
			      + "style='margin-left:5px;padding-bottom:1px; height: 8px' "
			      + "onclick = 'return GUI.Plugin.DQMHeaderRow.loopWorkspaces(event,\"previous\")' />"
			      + "&nbsp;Workspace"
			      + "<img src='" + ROOTPATH + "/static/next.png'"
			      + "style='margin-left:5px;padding-bottom:1px; height: 8px' "
			      + "onclick = 'return GUI.Plugin.DQMHeaderRow.loopWorkspaces(event,\"next\")' />",
			      "<span>_</span><span class='delimiter'>&nbsp;&nbsp;.</span>",
			      "pickMenu(\"workspace\")").firstChild;

    _runNr = this.heading("Run #", "<span>_</span><span class='delimiter'>"
		          + "&nbsp;&nbsp;.</span>", "pickSample(\"run\", \"dataset\")").firstChild;
    _lumiNr = this.heading("LS #", "<span>_</span><span class='delimiter'>"
			   + "&nbsp;&nbsp;.</span>").firstChild;
    _eventNr = this.heading("Event #", "<span>_</span><span class='delimiter'>"
			    + "&nbsp;&nbsp;.</span>").firstChild;
    _runStart = this.heading("Run started, UTC time", "<span>_</span>").firstChild;

    _subheader.appendChild(_menuData = document.createElement("span"));
    _menuData.style.display = "none";

    _subheader.appendChild(_menuService = document.createElement("div"));
    _menuService.style.display = "none";

    _subheader.appendChild(_menuWorkspace = document.createElement("div"));
    _menuWorkspace.style.display = "none";

    // Set up actions for search changes.
    _search.onkeyup = _search.paste = function(e) {
      if (_searchDelay) clearTimeout(_searchDelay);
      _searchDelay = setTimeout(_self.search, 150);
    };

    // Set up actions for strip chart changes.
    _striptype.onchange = _stripaxis.onchange
      = _stripomit.onchange = _stripn.onkeyup = _stripn.paste
      = function(e) {
        var obj = this;
        if (_stripDelay) clearTimeout(_stripDelay);
         _stripDelay = setTimeout(function() { _self.updateStripChart(obj); }, 150); };

    // Set up actions for reference changes.
    $('subhead-ref-show').onchange
      = $('subhead-ref-position').onchange
      = function(e) { _self.updateRefView(); };

    var refupdate = function(e) {
      var obj = this;
      if (_refDelay) clearTimeout(_refDelay);
      _refDelay = setTimeout(function() { _self.updateRefData(obj); }, 150);
    };

    for (var i = 0; i < 4; ++i)
    {
      var reftype = $('subhead-ref-' + i + '-type');
      var refrun  = $('subhead-ref-' + i + '-run');
      var refds   = $('subhead-ref-' + i + '-ds');
      reftype.onchange = refupdate;
      refrun.onkeyup = reftype.paste = refupdate;
      refds.onkeyup = refds.paste = refupdate;
    }

    // Set up the view settings floating panel.
    var ww = (window.innerWidth || document.documentElement.clientWidth);
    var wh = (window.innerHeight || document.documentElement.clientHeight);
    _toolsgroup.style.display = '';
    _toolspanel = new YAHOO.widget.Panel(_toolsgroup, {
      visible: true, draggable: true, close: true, zindex: 2,
      x: 10, y: 80, width: parseInt(0.40 * ww) + "px",
      constraintoviewport: true, autofillheight: "body" });
    var resize = new YAHOO.util.Resize(_toolsgroup, {
      handles: ["br"], autoRatio: false, minWidth: 200,
      minHeight: 50, status: false });
    resize.on("startResize", function(args) {
      var O = YAHOO.widget.Overlay;
      var D = YAHOO.util.Dom;
      var clreg = D.getClientRegion();
      var elreg = D.getRegion(this.element);
      resize.set("maxWidth", clreg.right - elreg.left - O.VIEWPORT_OFFSET);
      resize.set("maxHeight", clreg.bottom - elreg.top - O.VIEWPORT_OFFSET);
    }, _toolspanel, true);
    resize.on("resize", function(args) {
      this.cfg.setProperty("height", args.height + "px");
    }, _toolspanel, true);
    _toolspanel.render();
    _toolspanel.hide();
    _toolspanel.moveEvent.subscribe(function(type, args) {
      _toolslocal = true;
      _gui.asyncCall(_url() + "/panel?n=tools;show=yes;x="
		     + args[0][0] + ";y=" + args[0][1]);
    });
    _toolspanel.hideEvent.subscribe(function(type, args) {
      _toolslocal = true;
      _gui.asyncCall(_url() + "/panel?n=tools;show=no");
    });
    _toolspanel.showEvent.subscribe(function(type, args) {
      _toolslocal = true;
      _gui.asyncCall(_url() + "/panel?n=tools;show=yes");
    });

    // Clicking on the CMS logo toggles panel view.
    $('cmslogo').onclick = function() { return _self.togglePanel(); };
  };

  /** Detach this plug-in from the GUI display.  Clears the header and
      sub-header document elements from the objects #attach() added
      and clears closure references for garbage collection. */
  this.detach = function()
  {
    $('header').replaceChild(_toolsgroup, _toolsgroup.parentNode);
    while (_toolsgroup.childNodes.length > 3)
      _toolsgroup.removeChild(_toolsgroup.lastChild);

    // Remove all 'heading' elements we added.
    for (var m, n = _header.firstChild; n; n = m)
    {
      m = n.nextSibling;
      if (n.className == "heading")
	_header.removeChild(n);
    }

    // Clear subheader entirely.
    while (_subheader.firstChild)
      _subheader.removeChild(_subheader.firstChild);

    // Null out our references so objects are freed.
    _service = null;
    _workspace = null;
    _runNr = null;
    _lumiNr = null;
    _eventNr = null;
    _runStart = null;
    _menuData = null;
    _menuService = null;
    _menuWorkspace = null;
    _searchString = "";

    _toolspanel = null;
    _toolslocal = false;
    _toolsgroup.style.display = 'none';

    // If the search delay is activated, clear it.
    if (_searchDelay)
    {
      clearTimeout(_searchDelay);
      _searchDelay = null;
    }

    if (_refDelay)
    {
      clearTimeout(_refDelay);
      _refDelay = null;
    }

    if (_stripDelay)
    {
      clearTimeout(_stripDelay);
      _stripDelay = null;
    }

    _gui.setIdleMessage(null);
  };

  // ----------------------------------------------------------------
  /** Response callback for activating a particular sub-header menu.
      Switches the menu in place and lets the server know the current
      choice.  We don't wait for a new server state here as we already
      have all the data needed to show the menu, we just want the
      session to have the right state for the next scheduled
      update. */
  this.pickMenu = function(which)
  {
    which = _self.displayMenu(which, true);
    _self.updateCanvasPosition();
    YAHOO.util.Connect.asyncRequest("GET", _url() + "/setSubMenu?n=" + which, {});
    return false;
  };

  /** Response callback for selecting another sample.  The @a vary
      parameter specifies which part should be varied, and must
      be either "run" or "dataset".  Sends user to a special hidden
      "data selector" workspace where one can choose another data
      sample. */
  this.pickSample = function(vary, order)
  {
    _gui.makeCall(_url() + "/chooseSample?vary=" + vary + ";order=" + order);
    return false;
  };

  /** Response callback for changing workspace.  The @a choice
      parameter specifies the label of the workspace to issue a server
      call for.  The server automatically switches the sub-header menu
      back to "data" as a part of the workspace switch. */
  this.setWorkspace = function(choice)
  {
    _gui.makeCall(_url() + "/workspace?name=" + encodeURIComponent(choice));
    return false;
  };

  /** Respone callback for changing the alarm filter.  The @a choice
      parameter specifies the filter choice, one of three hard-coded
      values "all", "alarms" or "nonalarms". */
  this.setFilter = function(choice)
  {
    _gui.makeCall(_url() + "/setFilter?n=" + encodeURIComponent(choice));
    return false;
  };

  /** Respone callback for changing the global reference display.
      Passes the request to server which will prompt for details. */
  this.updateStripChart = function(obj)
  {
    if (obj == _striptype && _striptype.selectedIndex > 0 && ! _stripn.value)
      _stripn.value = 100;

    _gui.makeCall(_url() + _("/setStripChart?type=${type};n=${n}"
			     + ";omit=${omit};axis=${axis}",
			     {type: encodeURIComponent(_striptype.value),
			      axis: encodeURIComponent(_stripaxis.value),
			      omit: encodeURIComponent(_stripomit.value),
			      n: encodeURIComponent(_stripn.value)}));
    return false;
  };

  /** Respone callback for changing the global reference display.
      Passes the request to server which will prompt for details. */
  this.updateRefView = function()
  {
    _gui.makeCall(_url() + _("/setReference?show=${show};position=${pos}",
			     {show: $('subhead-ref-show').value,
			      pos: $('subhead-ref-position').value}));
    return false;
  };

  /** Respone callback for changing the global reference display.
      Passes the request to server which will prompt for details. */
  this.updateRefData = function(obj)
  {
    var opt = "?", sep = "";
    for (var i = 0; i < 4; ++i, sep = ";")
    {
      var reftype = $('subhead-ref-' + i + '-type');
      var refrun  = $('subhead-ref-' + i + '-run');
      var refds   = $('subhead-ref-' + i + '-ds');

      if (obj == reftype && reftype.selectedIndex != 1)
	refrun.value = refds.value = "";

      if ((refrun.value || refds.value) && reftype.selectedIndex != 1)
	reftype.selectedIndex = 1;

      if (reftype.selectedIndex == 0)
	opt += sep + "r" + (i+1) + "=refobj";
      else if (reftype.selectedIndex == 1 && (refrun.value || refds.value))
	opt += sep + "r" + (i+1) + "=other:" + encodeURIComponent(refrun.value)
	       + ":" + encodeURIComponent(refds.value);
      else
	opt += sep + "r" + (i+1) + "=none";
    }

    _gui.makeCall(_url() + "/setReference" + opt);
    return false;
  };

  /** Response callback triggered 50ms after the last change to the
      search input.  Checks if the search input has changed value and
      if so issues a server side incremental search. */
  this.search = function()
  {
    _searchDelay = null;
    if (_searchString != _search.value)
    {
      _searchString = _search.value;
      _searchResult.innerHTML = "(searching)";
      _gui.makeCall(_url() + "/setSearch?rx=" + encodeURIComponent(_search.value));
    }
  };

  // ----------------------------------------------------------------
  /** Update this plug-in display.  Updates menu areas and document
      title according to the current state. */
  this.update = function(data)
  {
    _data = data;
    _service.innerHTML = data.service;
    _workspace.innerHTML = data.workspace;
    _runNr.innerHTML = data.run;
    _lumiNr.innerHTML = data.lumi;
    _eventNr.innerHTML = data.event;
    _runStart.innerHTML = "<span title='UTC time'>" + data.runstart + "</span>";

    var title = data.service + " - " + data.workspace + " - " + HEADING;
    if (document.title != title)
      document.title = title;

    data.workspaces.sort(function(a, b) {
      if (a.rank != b.rank)
        return a.rank - b.rank;
      else if (a.title < b.title)
        return -1;
      else if (a.title > b.title)
        return 1;
      else
        return 0;
    });

    this.displayMenu(data.view.show, false);
    this.updatePanel(_toolslocal, _toolspanel, data.view.tools);

    // Update canvas-group position according to header dynamic height
    this.updateCanvasPosition();
  };

  this.updateCanvasPosition = function()
  {
    document.getElementById('canvas-group').style.top = _header.offsetHeight + 'px';
  };

  /** Switch to displaying sub-header menu @a which, one of
      "workspace", "service" or "data". */
  this.displayMenu = function(which, toggle)
  {
    if (toggle && _curmenu == which)
      which = 'data';

    // Switch the sub-header to the workspace menu.  Update the div to
    // a multi-column table workspaces, colum by category.  Workspaces
    // other than the current one are links to setWorkspace() response
    // callback.  Add a "(Hide)" link after the table so the user can
    // go back to data menu without switching workspaces.  Skip
    // workspaces with a negative ranking, these are hidden internal
    // implementation workspaces added automatically by the server.
    if (which == 'workspace')
    {
      var category = null;
      var cellattr = "<td style='border-right:1px dotted #ddd;padding:0 1em 0 ";
      var subhead = "<table id='subhead-workspace' class='item'><tbody valign='top'><tr>";
      for (var c = 0, i = 0, e = _data.workspaces.length; i < e; ++i)
      {
	var item = _data.workspaces[i];
	if (item.rank < 0)
	  continue;

	if (item.category != category)
	{
	  subhead += (category != null ? "</td>" : "")
		     + cellattr
		     + (category != null ? "10px'>" : "0'>")
		     + "<b style='color:#b66'>"
		     + item.category + "</b><br />";
          category = item.category;
	  ++c;
	}

	if (item.title != _data.workspace)
	  subhead += "<a href='#' onclick=\"return GUI.Plugin.DQMHeaderRow"
		     + ".setWorkspace('" + item.label + "')\">" + item.title
		     + "</a>";
	else
	  subhead += item.title;

	if (i < e-1)
          subhead += "<br />";
      }

      subhead += "</td>" + cellattr + "10px;border:none'><a href='#' onclick='"
		 + "return GUI.Plugin.DQMHeaderRow.pickMenu(\"data\")'>"
		 + "(Hide)</a></td></tr></tbody></table>";

      if (_menuWorkspace.innerHTML != subhead)
	_menuWorkspace.innerHTML = subhead;

      _menuData.style.display = "none";
      _menuService.style.display = "none";
      _menuWorkspace.style.display = "";
    }

    // Switch the sub-header to the service menu.  Update the div to a
    // list of links to DQM GUI services, except leave the current one
    // as a non-link name.  Add a "(Hide)" link after the list so the
    // user can go back to data menu without switching services.  Note
    // that these are real external links, not AJAX call links.
    else if (which == 'service')
    {
      var subhead = "";
      for (var i = 0, e = _data.services.length; i < e; ++i)
      {
	var item = _data.services[i];
	subhead += "<span class='item'>"
		   + (item.title == _data.service ? item.title :
		      ("<a href='" + item.href + "'>" + item.title + "</a>"))
		   + "</span>";
      }

      subhead += "<span class='item'><a href='#' onclick='"
		 + "return GUI.Plugin.DQMHeaderRow.pickMenu(\"data\")'>"
		 + "(Hide)</a></span>";

      if (_menuService.innerHTML != subhead)
	_menuService.innerHTML = subhead;

      _menuData.style.display = "none";
      _menuService.style.display = "";
      _menuWorkspace.style.display = "none";
    }

    // Show the type of data the user is looking at and the name of the
    // dataset as a link to the sample selection menu.
    else if (which == 'data')
    {
      // Update dataset.
      var subhead = "";
      var dslink = "<a href='#' onclick='return GUI.Plugin.DQMHeaderRow.pickSample(\"dataset\", \"run\")'>";
      var s = _data.view.sample;
      if (s.type == "live")
        dslink += "Live";
      else if (s.type == "online_data")
        dslink += "Online";
      else if (s.type == "offline_data")
        subhead += "Data: ", dslink += _sanitise(s.dataset);
      else if (s.type == "offline_relval")
        subhead += "RelVal: ", dslink += _sanitise(s.dataset);
      else if (s.type == "offline_mc")
        subhead += "MC: ", dslink += _sanitise(s.dataset);
      else
        subhead += "Other: ", dslink += _sanitise(s.dataset);

      subhead += dslink + "</a>";

      if (_menuData.innerHTML != subhead)
        _menuData.innerHTML = subhead;

      _menuData.style.display = "";
      _menuService.style.display = "none";
      _menuWorkspace.style.display = "none";
    }

    // Hide sub-header menu entirely.
    else
    {
      _menuData.style.display = "none";
      _menuService.style.display = "none";
      _menuWorkspace.style.display = "none";
    }

    // Always update the data menu in tools panel.

    // Update the all/alarm/non-alarm filter.
    var filter = $('subhead-filter');
    if (_data.view.filter == "all")
      filter.selectedIndex = 0;
    else if (_data.view.filter == "alarms")
      filter.selectedIndex = 1;
    else if (_data.view.filter == "nonalarms")
      filter.selectedIndex = 2;

    // Update the strip charting settings.
    if (_striptype.selectedIndex != _STRIPTYPE[_data.view.strip.type])
      _striptype.selectedIndex = _STRIPTYPE[_data.view.strip.type];
    if (_stripomit.selectedIndex != _STRIPOMIT[_data.view.strip.omit])
      _stripomit.selectedIndex = _STRIPOMIT[_data.view.strip.omit];
    if (_stripaxis.selectedIndex != _STRIPAXIS[_data.view.strip.axis])
      _stripomit.selectedIndex = _STRIPAXIS[_data.view.strip.axis];
    if (_stripn.value != _data.view.strip.n && !_stripDelay)
      _stripn.value = _data.view.strip.n;

    // Update the overlay/on-side/customise reference switch.
    var refpos = $('subhead-ref-position');
    var refshow = $('subhead-ref-show');
    if (refpos.selectedIndex != _REFERENCEPOS[_data.view.reference.position])
      refpos.selectedIndex = _REFERENCEPOS[_data.view.reference.position];
    if (refshow.selectedIndex != _REFERENCESHOW[_data.view.reference.show])
      refshow.selectedIndex = _REFERENCESHOW[_data.view.reference.show];

    for (var i = 0; !_refDelay && i < 4; ++i)
    {
      var index    = 0;
      var refparam = _data.view.reference.param[i];
      var reftype  = $('subhead-ref-' + i + '-type');
      var refrun   = $('subhead-ref-' + i + '-run');
      var refds    = $('subhead-ref-' + i + '-ds');

      if (refparam.type == 'refobj')     index = 0;
      else if (refparam.type == 'other') index = 1;
      else if (refparam.type == 'none')  index = 2;

      if (reftype.selectedIndex != index)
	reftype.selectedIndex = index;

      if (refrun.value != refparam.run)
        refrun.value = refparam.run;

      if (refds.value != refparam.dataset)
        refds.value = refparam.dataset;
    }

    // Update the search info.
    var search = _data.view.search;
    var word = (search.pattern == '' ? " object" : " match");
    var plural = (search.pattern == '' ? "s" : "es");
    var searchres = (! search.valid ? "(invalid regexp)"
      : (search.nmatches > 1 ? "(" + search.nmatches + word + plural + ")"
      : search.nmatches == 1 ? "(1 " + word + ")"
      : "(no " + word + plural + ")"));
    if (! _searchDelay && _search.value != search.pattern)
      _search.value = _searchString = search.pattern;
    if (_searchResult.innerHTML != searchres)
      _searchResult.innerHTML = searchres;

    // Report what we decided to show.
    _curmenu = which;
    return which;
  };

  this.togglePanel = function()
  {
    _toolspanel.cfg.setProperty("visible", !_toolspanel.cfg.getProperty("visible"));
    return false;
  };

  this.updatePanel = function(local, panel, data)
  {
    if (! local)
    {
      var show = Boolean(data.show);
      var x = data.x;
      var y = data.y;

      if (x >= 0 && panel.cfg.getProperty("x") != x)
	panel.cfg.setProperty("x", x);

      if (y >= 0 && panel.cfg.getProperty("y") != y)
	panel.cfg.setProperty("y", y);

      if (panel.cfg.getProperty("visible") != show)
	panel.cfg.setProperty("visible", show);
    }
  };

  /** Loop over all workspaces
   * */
  this.loopWorkspaces = function(e,direction)
  {
    // Prevent bubbling of onclick event which is already registered for the
    // header part. See http://www.quirksmode.org/js/events_access.html
    e = e || window.event;
    e.cancelBubble = true;
    if (e.stopPropagation)
      e.stopPropagation();
    actual = _data.workspace;
    actualIndex = -1;
    for (i = 0, e = _data.workspaces.length; i < e; ++i)
    {
      var item = _data.workspaces[i];
      if (item.rank >= 0)
      {
        if (actual == item.title)
	{
	  actualIndex = i;
	  break;
	}
      }
    }
    switch(direction)
    {
     case 'next':
      if (++actualIndex < _data.workspaces.length)
	if (_data.workspaces[actualIndex].rank >= 0)
	  return this.setWorkspace(_data.workspaces[actualIndex].label);
      break;
     case 'previous':
      if (--actualIndex >= 0)
	if (_data.workspaces[actualIndex].rank >= 0)
	  return this.setWorkspace(_data.workspaces[actualIndex].label);
      break;
    default:
      return false;
    }
  };

  /** Expose private variables needed to assemble the link-me hyperlink.
   *  @returns {Object} The parameters are assemble in a unique object
   *  whose keys are formatted in a way already readable by the start
   *  method exposed by the GUI. */
  this.linkMe = function()
  {
    function makerefobj(r) {
      if (r.type == 'refobj')
	return 'refobj';
      else if (r.type == 'none')
	return 'none';
      else if (r.type == 'other')
	return r.type + ':' + r.run + ':' + r.dataset;
      else
	return 'none';
    }

    var result = {
      runnr:         _data.view.sample.run,
      dataset:       _data.view.sample.dataset,
      sampletype:    _data.view.sample.type,
      filter:        _data.view.filter,
      referencepos:  _data.view.reference.position,
      referenceshow: _data.view.reference.show,
      referenceobj1: makerefobj(_data.view.reference.param[0]),
      referenceobj2: makerefobj(_data.view.reference.param[1]),
      referenceobj3: makerefobj(_data.view.reference.param[2]),
      referenceobj4: makerefobj(_data.view.reference.param[3]),
      search:        _data.view.search.pattern,
      striptype:     _data.view.strip.type,
      stripruns:     _data.view.strip.n,
      stripaxis:     _data.view.strip.axis,
      stripomit:     _data.view.strip.omit,
      workspace:     _data.workspace
    };

    return result;
  };

  return this;
}();
