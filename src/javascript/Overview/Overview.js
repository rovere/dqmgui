/** The menu on the top of the screen in the Overview GUI.

    OverviewHeader shows basic interactive status information and
    messages.  It provides short cut access to the other Overview
    instances in the same "ring", the workspaces and its views.  */
GUI.Plugin.OverviewHeader = new function() {
  /** Reference for this, for use in callbacks where "this" is
      ambiguous or at least not this closure. */
  var _self = this;

  /** Reference to the GUI main object, for making server calls. */
  var _gui = null;

  /** Data from the last update.  Used to fill in the interaction area
      with a menu of the user's choice (none, workspaces or views)
      without having to make another round trip server call. */
  var _data = null;

  /** The DOM element for the main header. */
  var _header = $('header');

  /** The DOM element for the message area. */
  var _messages = $('messages');

  /** The DOM element for the sub-header below the main header, where
      we show three interaction menus: none, workspaces or views. */
  var _subheader = $('subheader');

  /** The DOM element for the current workspace in the main menu.
      Points to the 'span' element to be filled in with the name. */
  var _workspace = null;

  /** The DOM element for the current view in the main menu.
      Points to the 'span' element to be filled in with the name. */
  var _view = null;

  /** The DOM element for the current help link in the main menu.
      Points to the 'span' element to be filled in with the name. */
  var _help = null;

  /** The DOM element for workspace choice sub-header menu.  Points to
      the 'div' element containing the menu.  The menu is shown by
      setting its style.display to '' and style.display of the other
      menus to 'none'. */
  var _menuWorkspace = null;

  /** The DOM element for view choice sub-header menu.  Points to
      the 'div' element containing the menu.  The menu is shown by
      setting its style.display to '' and style.display of the other
      menus to 'none'. */
  var _menuView = null;

  /** Currently displayed extra menu in the sub-header area. */
  var _curmenu          = null;

  // ----------------------------------------------------------------
  /** Utility method to create a new top level heading with @a label
      as the title above and @a content as the contents below.

      If @a link is non-empty, the whole heading element is made a
      link which will call OverviewHeader with the method name and
      parameters specified in link as a text string; a link
      'foo("arg")' would call OverviewHeader.foo("arg") when the header
      element is clicked.  As usual with links, the method should
      return @c false to prevent link traversal and should not make
      assumptions about @c this.

      Returns the 'h2' DOM element which contains the @a contents.  */
  this.heading = function(label, content, link)
  {
    var linkattr = (link ? " onclick='return GUI.Plugin.OverviewHeader." + link + "'" : "")
    var linkstart = (link ? ("<a href='#'" + linkattr + ">") : "");
    var linkend = (link ? ("<img src='" + ROOTPATH + "/static/down-menu.gif'"
      + "style='margin-left:5px;padding-bottom:1px' width='7' height='7' /></a>") : "");

    var div = document.createElement("div");
    _header.insertBefore(div, _header.lastChild);

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
    _workspace = this.heading("Workspace", "<span>_</span><span class='delimiter'>&nbsp;&nbsp;.</span>",
			      "pickMenu(\"workspace\")").firstChild;
    _view = this.heading("View", "<span>_</span><span class='delimiter'>&nbsp;&nbsp;.</span>",
			 "pickMenu(\"view\")").firstChild;
    _help = this.heading("Help", "<span>_</span>").firstChild;

    _subheader.insertBefore(_menuWorkspace = document.createElement("div"), _messages);
    _menuWorkspace.style.display = "none";

    _subheader.insertBefore(_menuView = document.createElement("div"), _messages);
    _menuView.style.display = "none";
  };

  /** Detach this plug-in from the GUI display.  Clears the header and
      sub-header document elements from the objects #attach() added
      and clears closure references for garbage collection. */
  this.detach = function()
  {
    // Remove all 'heading' elements we added.
    for (var m, n = _header.firstChild; n; n = m)
    {
      m = n.nextSibling;
      if (n.className == "heading")
	_header.removeChild(n);
    }

    // Clear subheader from the elements we added and make messages visible.
    while (_subheader.firstChild != _messages)
      _subheader.removeChild(_subheader.firstChild);

    _messages.style.display = "";

    // Null out our references so objects are freed.
    _workspace = null;
    _view = null;
    _help = null;
    _menuWorkspace = null;
    _menuView = null;
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
    YAHOO.util.Connect.asyncRequest("GET", _url() + "/setSubMenu?n=" + which, {});
    return false;
  };

  /** Response callback for changing workspace.  The @a choice
      parameter specifies the label of the workspace to issue a server
      call for.  The server automatically switches the sub-header menu
      back to "view" as a part of the switch. */
  this.setWorkspace = function(choice)
  {
    _gui.makeCall(_url() + "/workspace?name=" + encodeURIComponent(choice));
    return false;
  };

  /** Response callback for changing view.  The @a choice
      parameter specifies the label of the view to issue a server
      call for.  The server automatically switches the sub-header menu
      back to "view" as a part of the switch. */
  this.setView = function(choice)
  {
    _gui.makeCall(_url() + "/view?name=" + encodeURIComponent(choice));
    return false;
  };

  // ----------------------------------------------------------------
  /** Update this plug-in display.  Updates menu areas and document
      title according to the current state. */
  this.update = function(data)
  {
    _data = data;
    _workspace.innerHTML = data.workspace;
    _view.innerHTML = data.view;
    if (data.help)
    {
      _help.parentNode.display = '';
      _help.innerHTML = "<a href='https://twiki.cern.ch/twiki/bin/view/CMS/"
		        + "ComputingShifts#" + data.help + "' target='_blank'>"
		        + "Instructions</a>";
    }
    else
      _help.parentNode.display = 'none';

    var title = data.workspace + " - " + data.view + " - " + HEADING;
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

    this.displayMenu(data.menu, false);
  };

  /** Switch to displaying sub-header menu @a which, one of
      "workspace", "view", or "none". */
  this.displayMenu = function(which, toggle)
  {
    if (toggle && _curmenu == which)
      which = 'none';

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
	  subhead += "<a href='#' onclick=\"return GUI.Plugin.OverviewHeader"
		     + ".setWorkspace('" + item.label + "')\">" + item.title
		     + "</a>";
	else
	  subhead += item.title;

	if (i < e-1)
          subhead += "<br />";
      }

      subhead += "</td>" + cellattr + "10px;border:none'><a href='#' onclick='"
		 + "return GUI.Plugin.OverviewHeader.pickMenu(\"data\")'>"
		 + "(Hide)</a></td></tr></tbody></table>";

      if (_menuWorkspace.innerHTML != subhead)
	_menuWorkspace.innerHTML = subhead;

      _menuWorkspace.style.display = "";
      _menuView.style.display = "none";
      _messages.style.display = "none";
    }

    // Show the type of data the user is looking at and the name of the
    // dataset as a link to the sample selection menu.
    else if (which == 'view')
    {
      var subhead = "";
      for (var i = 0, e = _data.views.length; i < e; ++i)
      {
	var item = _data.views[i];
	subhead += "<span class='item'>"
	           + "<a href='#' onclick=\"return GUI.Plugin.OverviewHeader"
		   + ".setView('" + item.label + "')\">" + item.title
		   + "</a></span>";
      }

      subhead += "<span class='item'><a href='#' onclick='"
		 + "return GUI.Plugin.OverviewHeader.pickMenu(\"none\")'>"
		 + "(Hide)</a></span>";

      if (_menuView.innerHTML != subhead)
	_menuView.innerHTML = subhead;

      _menuWorkspace.style.display = "none";
      _menuView.style.display = "";
      _messages.style.display = "none";
    }

    // Display idle message.
    else
    {
      _menuWorkspace.style.display = "none";
      _menuView.style.display = "none";
      _messages.style.display = "";
    }

    // Report what we decided to show.
    _curmenu = which;
    return which;
  };

  return this;
}();
