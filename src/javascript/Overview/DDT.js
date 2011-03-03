GUI.Plugin.CompDDTSummary = new function()
{
  var _canvas = $('canvas');
  this.attach = function(gui) {}
  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.update = function(data)
  {
    var content = ""
      + "<a href='http://dashb-ssb.cern.ch/dashboard/request.py/siteviewhome' target='_blank'>Site Status for the CMS sites</a>"

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();

GUI.Plugin.CompDDTSite = new function()
{
  var _canvas = $('canvas');
  this.attach = function(gui) {}
  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.update = function(data)
  {
    var content = ""
      + "<h2>Check if requested data is being queued</h2>"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/0' />"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/1' />"
      + "<hr />"
      + "<h2>Check if queued data is failing or not attempted</h2>"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/2' />"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/3' />"
      + "<hr />"
      + "<h2>Check if data is being migrated to tape</h2>"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/4' />"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/5' />"
      + "<hr />"
      + "<h2>Check for large amounts of idle requests</h2>"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/6' />"
      + "<img src='" + ROOTPATH + "/plotfairy/ddt/" + data.name + "/7' />"
      + "<hr />"

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();
