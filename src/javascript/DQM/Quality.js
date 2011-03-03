/** A high-level overview summary of DQM sources.

    DQMQuality displays a high-level summary of the DQM processing
    applications.  It shows the 2D quality map with the source name,
    the report summary and the time of last update, on colour-coded
    background.  Double-clicking on the source "panel" automatically
    transfers to the source workspace, if there is one. */
GUI.Plugin.DQMQuality = new function()
{
  /** Reference to this, for use in callbacks where "this" is
      ambiguous or at least not this closure. */
  var _self	= this;

  /** Reference to the GUI main object, for making server calls. */
  var _gui	= null;

  /** The items cached from the last server update, for clicking. */
  var _items    = null;

  /** The DOM element for the canvas area where we display contents. */
  var _canvas	= $('canvas');

  /** Mouse click disambiguation object, used with #_dblclick(). */
  var _click	= { event: null, timeout: null, timeClick: 0, timeDoubleClick: 0 };

  // ----------------------------------------------------------------
  /** Attach this plug-in to the GUI display.  Remembers the GUI. */
  this.attach = function(gui)
  {
    _gui = gui;
  };

  /** Detach this plug-in from the GUI display.  Clears the canvas. */
  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  };

  // ----------------------------------------------------------------
  /** Response callback for single-clicking on the canvas items.
      Currently does nothing. */
  this.select = function(e)
  {
    return false;
  };

  /** Response callback for double-clicking on the canvas items.
      Finds the item that was clicked on, and tries to find a matching
      workspace.  Transfers there if one is found, otherwise shows a
      'no such workspace' message in the message area. */
  this.jumpto = function(e)
  {
    // Navigate to the 'item' container.
    var hit = (e.srcElement || e.target);
    while (hit.tagName != 'DIV')
      hit = hit.parentNode;

    // Count which item this is.
    var n = 0;
    for (var x = hit; x.previousSibling; x = x.previousSibling)
      ++n;

    // Find the hit object in the workspace list, and if found,
    // transfer control to that workspace.  Match the names
    // case-insensitively for some flexibility (Hcal vs. HCAL).
    var found = false;
    var hitname = _items[n].label.toUpperCase();
    var wslist = GUI.Plugin.DQMHeaderRow.workspaces();
    for (var i = 0, e = wslist.length; i < e; ++i)
    {
      if (wslist[i].title.toUpperCase() == hitname)
      {
	_gui.makeCall(_url() + "/workspace?name=" + encodeURIComponent(wslist[i].label));
	found = true;
	break;
      }
    }

    // If we didn't find a workspace, notify the user.
    if (! found)
      $('messages').innerHTML =
        "<span style='font-size:95%'>Sorry, no workspace defined for '"
	+ _sanitise(_items[n].label) + "'!</span>";

    return false;
  };

  // ----------------------------------------------------------------
  /** Update this plug-in display.  Fills the canvas area with a
      floating image panel for each DQM source for which we have the
      required summary information and 2D map.  Updates the page
      incrementally to avoid flashing. */
  this.update = function(data)
  {
    _items = data.items;
    var status = data.items;
    var now = new Date();
    var content = "";

    // If there is no contents to show, display a simple message.
    if (! status.length)
    {
      var content =
        "<div id='no-contents' align='center'"
	+ " style='width:75%;font-size:2em;line-height:1.6;padding:2em;color:#700'>"
	+ "<p>No data are presently available or the DQM producers have not"
	+ " generated the standard EventInfo summary information.</p></div>";

      if (_canvas.innerHTML != content)
        _canvas.innerHTML = content;
    }

    // If we have items to display, show a compressed floating panel
    // for each DQM source.  The header displays the DQM source name,
    // the overall report summary, and the time of last update.  Make
    // canvas updates incrementally.
    else
    {
      var now = new Date();
      var ets = new Date();
      var item = _canvas.firstChild;

      // If the canvas was previously empty, get rid of the message.
      if (item && item.id == 'no-contents')
      {
        _canvas.removeChild(item);
        item = null;
      }

      // Walk over all the new items and canvas items simultaneously.
      for (var i = 0, e = status.length; i < e; ++i, item = item.nextSibling)
      {
	// Re-use existing canvas item if there is one or create a new one.
        var title;
        if (! item)
	{
	  item = document.createElement("div");
	  title = document.createElement("h3");
	  item.appendChild(title);
	  _canvas.appendChild(item);
	  item.onclick = item.ondblclick = function(e) {
	    return _dblclick(e || window.event, _click, _self.select, _self.jumpto);
	  };
	  item.style.cursor = "pointer";
	}
	else
	  title = item.firstChild;

	// Locate the image of the canvas item, or create a new one.
	var img = title.nextSibling;
	if (! img)
	{
	  img = document.createElement("img");
	  img.width = 266;
	  img.height = 200;
	  item.appendChild(img);
	}

	// Update the canvas item where necessary: color, title, appearance...
        var obj = status[i];
        var color = ((obj.reportSummary == null || obj.reportSummary < 0) ? "#000"
  		     : (obj.reportSummary < 0.85 ? "#a22"
	  		: (obj.reportSummary < 0.95 ? "#c30" : "#272")));
        var bcolor = ((obj.reportSummary == null || obj.reportSummary < 0) ? "#f0f0f0"
  		      : (obj.reportSummary < 0.85 ? "#fff0f0"
		         : (obj.reportSummary < 0.95 ? "#fff0c0" : "#efe")));
	ets.setTime((obj.eventTimeStamp & 0xffffffff) * 1000);

	_settitle(item, "item size-medium", obj.label + " quality report");
	item.style.backgroundColor = bcolor;

	_setval(title, "",
		obj.label + " - "
		+ (obj.reportSummary >= 0 ? sprintf("%.1f%%", obj.reportSummary*100.)
		   : (obj.reportSummary == -1 ? "No DAQ" : "N/A"))
		+ " - " + (obj.eventTimeStamp > 10000 ? _formatTime(ets, now) : "(Never)"));
	title.style.color = color;

	// If the image needs reloading, hand it over to the image loader.
	var imgsrc =
	  FULLROOTPATH + "/plotfairy/"
	  + encodePathComponent(obj.location) + "/"
	  + encodePathComponent(obj.name) + "?"
	  + "session=" + SESSION_ID
	  + ";v=" + obj.version
	  + ";w=266;h=200";
	if (! img.src)
	  img.src = ROOTPATH + "/static/blank.gif";
	if (img.src != imgsrc)
	  GUI.ImgLoader.load('dqmq' + i, img, imgsrc);
      }

      // Remove any remaining excess canvas items.
      while (_canvas.childNodes.length > status.length)
        _canvas.removeChild(_canvas.lastChild);
    }
  };

  return this;
}();
