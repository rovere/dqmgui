GUI.Plugin.EVD = new function()
{
  var _gui		   = null;
  var _canvas              = this;
  var _gcanvas             = document.getElementById('canvas-area');

  this.attach = function(gui)
  {
    _gui = gui;
    _gcanvas.style.bottom = 0;
  }

  this.detach = function()
  {
    while (_gcanvas.firstChild)
      _gcanvas.removeChild(_gcanvas.firstChild);
  }

  this.image = function(v)
  {
    _gui.makeCall(_url() + "/setPage?page=" + encodeURIComponent(v));
    return false;
  }

  this.date = function(v)
  {
    _gui.makeCall(_url() + "/set?date=" + encodeURIComponent(v));
    return false;
  }

  this.update = function(data)
  {
    var content = "";

    if (data.dates.length)
    {
      content += "<div style='margin:.25em 2em .25em 0'>Dates: ";
      for (var i = 0, e = data.dates.length; i < e; ++i)
        if (data.dates[i] == data.date)
          content += " " + _sanitise(data.dates[i]);
        else
          content += " <a href='#' onclick='return GUI.Plugin.EVD.date(this.textContent)'>"
	    + _sanitise(data.dates[i]) + "</a>";
      content += "</div>";
    }
    else
      content += "<div style='margin:.25em 2em .25em 0'>(No images currently available)</div>";

    if (data.images)
    {
      content += "<div style='margin:.25em 2em .25em 0'>Images: "
        + " <a href='#' onclick='return GUI.Plugin.EVD.image(\"\")'>(Latest)</a>";
      for (var i = data.images; i >= 1; --i)
        if (i == data.curimage)
          content += " " + i.toString();
        else
          content += " <a href='#' onclick='return GUI.Plugin.EVD.image(this.textContent)'>"
	    + i.toString() + "</a>";
      content += "</div>";
    }

    if (data.image)
      content += "<img style='padding:2em 2em 0 2em' src='"
        + FULLROOTPATH + "/plotfairy/iguana-snapshot/"
	+ encodeURIComponent(data.date) + "/"
	+ encodeURIComponent(data.image) + "'>"

    if (_gcanvas.innerHTML != content)
      _gcanvas.innerHTML = content;
  }

  this.qimage = function(label, stamp)
  {
    var hasqplots = (_qavail.length > 0);
    if (label && _qmap[label])
    {
      var relurl = _qrender + "/" + label + "/EventInfo/reportSummaryMap";
      var url = FULLROOTPATH + "/plotfairy/"
	       + encodeURIComponent(relurl).replace(/%2F/g, "/")
	       + "?session=" + SESSION_ID + ";w=532;h=400" + stamp;
      return "<img src='" + url + "' title='" + label + " quality'"
	     + " width='532' height='400'/>";
    }
    else if (hasqplots)
      return "<span style='font-size:95%; color:#888'>(Click on subdetector"
	     + " to show quality data.)</span>";
    else
      return "<span style='font-size:95%; color:#888'>(No further quality"
	     + " data available.)</span>";
  }

  return this;
}();
