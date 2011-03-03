GUI.Plugin.CompCAFBase = new function()
{
  var _canvas = $('canvas');

  this.attach = function(gui, content)
  {
    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.update = function(data)
  {
    var imgs = $$(_canvas, 'IMG');
    for (var i = 0, e = imgs.length; i < e; ++i)
    {
      var img = imgs[i];
      if (img.src.indexOf("/plotfairy/") >= 0)
      {
        var baseurl = img.src.replace(/\?.*/, "");
        GUI.ImgLoader.load("img" + i, img, baseurl + "?session="
                           + SESSION_ID + ";version=" + data.version);
      }
    }

    var objs = $$(_canvas, 'OBJECT');
    for (var i = 0, e = objs.length; i < e; ++i)
    {
      var obj = objs[i];
      if (obj.data.indexOf("/plotfairy/") >= 0)
        obj.data = obj.data.replace(/\?.*/, "?session=" + SESSION_ID
				    + ";version=" + data.version);
    }
  }

  return this;
}();

GUI.Plugin.CompCAFCastor = new function()
{
  _base = GUI.Plugin.CompCAFBase;
  this.attach = function(gui)
  {
    _base.attach(gui, ""
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSCAF cluster</h3>"
      + "<img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/c2cms-cmscaf-cpu' /><br />"
      + "<img style='margin:-.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/c2cms-cmscaf-net' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSCAFUSER cluster</h3>"
      + "<img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/c2cms-cmscafuser-cpu' /><br />"
      + "<img style='margin:-.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/c2cms-cmscafuser-net' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>Metrics distribution of CMSCAF machines' loads </h3>"
      + "<img class='icon' src='"
      + ROOTPATH + "/plotfairy/caf-plot/c2cms-cmscaf-loadavg' /><br />"
      + "<h3>Metrics distribution of CMSCAFUSER machines' loads </h3>"
      + "<img class='icon' src='"
      + ROOTPATH + "/plotfairy/caf-plot/c2cms-cmscafuser-loadavg' /><br />"
      + "</div>");
  }

  this.detach = _base.detach;
  this.update = _base.update;
  return this;
}();

GUI.Plugin.CompCAFBatch = new function()
{
  var _base = GUI.Plugin.CompCAFBase;
  this.attach = function(gui)
  {
    _base.attach(gui, ""
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSCAF</h3>"
      + "<img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmscaf-cpu' /><br />"
      + "<img style='margin:-.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmscaf-net' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSEXPRESS</h3>"
      + "<img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmsexpress-cpu' /><br />"
      + "<img style='margin:-.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmsexpress-net' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSINTER</h3>"
      + "<img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmsinter-cpu' /><br />"
      + "<img style='margin:-.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmsinter-net' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>Metrics distribution of CMSCAF machines' loads </h3>"
      + "<img class='icon' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmscaf-loadavg' /><br />"
      + "<h3>Metrics distribution of CMSEXPRESS machines' loads </h3>"
      + "<img class='icon' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmsexpress-loadavg' /><br />"
      + "<h3>Metrics distribution of CMSINTER machines' loads </h3>"
      + "<img class='icon' src='"
      + ROOTPATH + "/plotfairy/caf-plot/lxbatch-cmsinter-loadavg' /><br />"
      + "</div>");
  }

  this.detach = _base.detach;
  this.update = _base.update;
  return this;
}();

GUI.Plugin.CompCAFJobs = new function()
{
  var _base = GUI.Plugin.CompCAFBase;
  this.attach = function(gui)
  {
    _base.attach(gui, ""
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSCAF1NH</h3><img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/job-queue-caf1nh' />"
      + "<h3>CMSCAF1ND</h3><img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/job-queue-caf1nd' />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>CMSCAF1NW</h3><img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/job-queue-caf1nw' />"
      + "<h3>CMSEXPRESS</h3><img style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/caf-plot/job-queue-cmsexpress' />"
      + "</div>");
  }

  this.detach = _base.detach;
  this.update = _base.update;
  return this;
}();

GUI.Plugin.CompCAFDisk = new function()
{
  var _base = GUI.Plugin.CompCAFBase;
  this.attach = function(gui)
  {
    _base.attach(gui, ""
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>Free space CMSCAF pool</h3><br><br>"
      + "<img style='margin:-0.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/disk-free' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>Free space CMSCAFUSER pool</h3><br><br>"
      + "<img style='margin:-0.75em 0 1em' src='"
      + ROOTPATH + "/plotfairy/caf-plot/diskuser-free' /><br />"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<table width=100%><tr><th>CAF groups</th>"
      + "<th align='center'>Disk utilization by CAF data registered in PhEDEx</th>"
      + "<th align='center'>PhEDEx-registered datasets</th></tr>"
      + "<tr><td valign='top'>u_CMSALCA</td>"
      + "<td align='center' valign='top'><object data='" + ROOTPATH
      + "/plotfairy/caf-disk/alcapercent' type='text/plain'></object></td>"
      + "<td valign='top' align='center'><a href='" + ROOTPATH
      + "/plotfairy/caf-disk/alcabydate'>ALCA datasets organized by date</a>"
      + "<br/><a href='" + ROOTPATH + "/plotfairy/caf-disk/alcabysize"
      + "'>ALCA datasets organized by size</a></td></tr>"
      + "<tr><td valign='top'>u_CMSCOMM</td>"
      + "<td align='center' valign='top'><object data='" + ROOTPATH
      + "/plotfairy/caf-disk/commpercent' type='text/plain'></object></td>"
      + "<td valign='top' align='center'><a href='" + ROOTPATH
      + "/plotfairy/caf-disk/commbydate'>CMSCOMM datasets organized by date</a>"
      + "<br/><a href='" + ROOTPATH + "/plotfairy/caf-disk/commbysize"
      + "'>CMSCOMM datasets organized by size</a></td></tr>"
      + "<tr><td valign='top'>u_CMSPHYS</td>"
      + "<td align='center' valign='top'><object data='" + ROOTPATH
      + "/plotfairy/caf-disk/physpercent' type='text/plain'></object></td>"
      + "<td valign='top' align='center'><a href='" + ROOTPATH
      + "/plotfairy/caf-disk/physbydate'>CMSPHYS datasets organized by date</a>"
      + "<br/><a href='" + ROOTPATH + "/plotfairy/caf-disk/physbysize"
      + "'>CMSPHYS datasets organized by size</a></td></tr></table>"
      + "</div>");
  }

  this.detach = _base.detach;
  this.update = _base.update;
  return this;
}();
