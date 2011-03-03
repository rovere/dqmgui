GUI.Plugin.CompCore = new function()
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
      + "<h1>Cern/Core Infrastructure</h1>"
      + "IT SBB <a href='http://it-support-servicestatus.web.cern.ch/it-support-servicestatus/' target='_blank'>IT Service Status Board - all Computing Services</a>"
      + "<br />"
      + "Schedulled Downtimes <a href='https://twiki.cern.ch/twiki/bin/view/CMS/ScheduledInterventions?sortcol=3;table=1;up=1' target='_blank'>Central CMS Services</a>"
      + "<br />"
      + "CASTOR <a href='http://sls.cern.ch/sls/service.php?id=CASTORCMS' target='_blank'>CMS instances at CERN</a>"
      + "<br />"
      + "<iframe src='http://servicemap.cern.ch/ccrc08/servicemap1.html?vo=CMS&site=' width='900' height='740' />"

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();
