GUI.Plugin.CompTier0Summary = new function()
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
      + "<div style='margin-top: 15px;'>"
      + "<a style='font-size: 1.4em;' href='http://cmsonline.cern.ch/portal/page/portal/CMS%20online%20system/Storage%20Manager' target='_blank'>Storage manager</a>"
      + "<br />"
      + "<a style='font-size:1.4em;' href='#' onclick='popup(\"https://cmsweb.cern.ch/T0Mon/\")'>T0Mon</a>"
      + "</div>"

      // tier 0 export
      + "<h3>Information for c2cms subcluster t0export group of hosts</h3>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/t0exp_load_average' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/t0exp_cpu_utilization' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/t0exp_net_utilization' />"
      + "</div>"
      + "<div style='clear:both; border: 3px solid;'></div>"

      //tier 1 transfer
      + "<h3>Information for c2cms subcluster t1transfer group of hosts</h3>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/t1transfer_load_average' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/t1transfer_cpu_utilization' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/t1transfer_net_utilization' />"
      + "</div>"
      + "<div style='clear:both; border: 3px solid;'></div>"

      //cmst0
      + "<h3>Information for c2cms subcluster cmst0 group of hosts</h3>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/cmst0_load_average' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/cmst0_cpu_utilization' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/cmst0_net_utilization' />"
      + "</div>"
      + "<div style='clear:both; border: 3px solid;'></div>"

//          + "<h3>Total space on t0export</h3>"
//          + "<div class='item' style='float:left; padding: 0.5em'>"
//          + "<img src='"
//          + ROOTPATH + "/plotfairy/tier0/t0exportservice.png' />"
//          + "</div>"
//          + "<div style='clear:both; border: 3px solid;'></div>"

      //cmsproc
      + "<h3>Information for lxbatch subcluster cmsproc group of hosts</h3>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/cmsproc_load_average' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/cmsproc_cpu_utilization' />"
      + "</div>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/cmsproc_net_utilization' />"
      + "</div>"
      + "<div style='clear:both; border: 3px solid;'></div>"

      + "<h3>Tier-0 Jobs</h3>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/tier0_jobs' />"
      + "</div>"
      + "<div style='clear:both; border: 3px solid;'></div>"

      + "<h3>Total space on t0export</h3>"
      + "<div class='item' style='float:left; padding: 0.5em'>"
      + "<img src='"
      + ROOTPATH + "/plotfairy/tier0/tier0export_space' />"
      + "</div>"
      + "<div style='clear:both; border: 3px solid;'></div>";

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();

GUI.Plugin.Tier0Storage = new function()
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
      + "<div>"
      + "<a href='http://cmsonline.cern.ch/portal/page/portal/CMS%20online%20system/Storage%20Manager' target='_blank'>Storage manager</a>"
      + "<br />"
      + "<a href='#' onclick='window.open(\"https://cmsweb.cern.ch/T0Mon/\", \"popup\"); return false'>T0Mon</a>"
      + "</div>";

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();
