//FileLightGroup, FileLightSite
var _IMG_BORDER_NONE  = "1px solid transparent";
var _IMG_BORDER_NEW   = "1px solid #4a4";

GUI.Plugin.FileLightSite = new function()
{
  var _canvas = $('canvas');
  var _gui = null;
  var _self = this;

  this.img = null;
  this.select_site = null;
  this.select_size = null;
  this.in_filter = null;
  this.span_scheme = null;
  this.check_incomplete = null;
  this.check_subscribed = null;
  this.check_custodial = null;
  this.site_list = null;
  this.site = null;
  this.scheme_array = [];
  this.scheme_valid = {
    complete:"IsComplete",
    custodial:"IsCustodial",
    subscribed:"IsSubscribed",
    group:"Physics Group",
    dataset:"MC/Data Type",
    prod:"Production",
    cond:"Conditions",
    tiers:"Data Tiers",
    id:"Block ID"
  };

  this.attach = function(gui, linkcontent, preopts, postopts)
  {
    _gui = gui;

    var scheme_opts = "<option value='null'></option>";
    for (v in this.scheme_valid) {
      scheme_opts += "<option value="+v+">"+this.scheme_valid[v]+"</option>";
    }

    var content = ""
      + "<div class='item' id='filelight-options' style='width:100%'>"
      + "<span><img style='cursor:pointer' href='#' onclick='return GUI.Plugin.FileLightSite.moveRel(-1)' alt='Previous' src='"+ROOTPATH+"/static/left.gif'>"
      + "<img style='cursor:pointer' href='#' onclick='return GUI.Plugin.FileLightSite.moveRel(1)' alt='Previous' src='"+ROOTPATH+"/static/right.gif'></span> "
      + "<span>Site: "
      + "<select id='filelight-site-select' onchange='return GUI.Plugin.FileLightSite.updateSite(this.value)'>"
      + "<option value='null'></option>"
      + "</select></span> "
      + "<span>Show incomplete: <input type='checkbox' id='filelight-site-incomplete' onclick='return GUI.Plugin.FileLightSite.updateCheck(\"incomplete\",this.checked)'></span> "
      + "<span>Only subscribed: <input type='checkbox' id='filelight-site-subscribed' onclick='return GUI.Plugin.FileLightSite.updateCheck(\"subscribed\",this.checked)'></span> "
      + "<span>Only custodial: <input type='checkbox' id='filelight-site-custodial' onclick='return GUI.Plugin.FileLightSite.updateCheck(\"custodial\",this.checked)'></span> "
      + "<span>Size: <select id='filelight-size' onchange='return GUI.Plugin.FileLightSite.updateSize(this.value)'><option value='400'>400x400</option><option value='800'>800x800</option><option value='1200' selected>1200x1200</option><option value='1600'>1600x1600</option><option value='2000'>2000x2000</option></select></span>"
      + "</div>"
      + "<div class='item' id='filelight-options'>"
      + "Dataset Filter: <input size=20 id='filelight-site-filter' onkeypress='return GUI.Plugin.FileLightSite.updateFilter(this,event)'></div>"
      + "<div class='item' id='filelight-options'>Split:<span id='filelight-site-scheme'></span><select onchange='return GUI.Plugin.FileLightSite.addScheme(this.value)'>"+scheme_opts+"</select> <a href='#' onclick='return GUI.Plugin.FileLightSite.sendScheme()'>update</a></div>"
      //+ "<div id='filelight-image-area' style='background-image:url(\""+ROOTPATH+"/static/progress.gif\");background-repeat:no-repeat;background-position:center;width:1200px'>"
      + "<div id='filelight-image-area' style='float:left;clear:left'>"
      + "<img id='filelight-image' src='"+ROOTPATH+"/static/blank.gif' width=1200 height=1200>"
      + "</div>";


    if (_canvas.innerHTML != content) _canvas.innerHTML = content;

    if (_canvas.style.display != '') _canvas.style.display = '';

    this.img = $("filelight-image");
    this.select_site = $("filelight-site-select");
    this.select_size = $("filelight-site-size");
    this.in_filter = $("filelight-site-filter");
    this.span_scheme = $("filelight-site-scheme");
    this.check_incomplete = $("filelight-site-incomplete");
    this.check_subscribed = $("filelight-site-subscribed");
    this.check_custodial = $("filelight-site-custodial");

  }

  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.moveRel = function(n)
  {
    for (i in _self.site_list) {
      if (_self.site_list[i]==_self.site) {
        var index = (i+n) % _self.site_list.length;
        return this.updateSite(_self.site_list[index]);
      }
    }
    return false;
  }

  this.updateSize = function(value) {
    _gui.makeCall(_url()+"/opt?width="+encodeURIComponent(value)+";height="+encodeURIComponent(value));
    return false;
  }

  this.updateSite = function(site)
  {
    if (site != "null") {
      _gui.makeCall(_url()+"/opt?site="+encodeURIComponent(site));
    }
    return false;
  }

  this.updateCheck = function(what,value) {
    if (value) {
      _gui.makeCall(_url()+"/opt?"+what+"=1");
    } else {
      _gui.makeCall(_url()+"/opt?"+what+"=0");
    }
    return false;
  }

  this.updateFilter = function(input,event) {
    if (event.which==13) {
      var value = _self.in_filter.value;
      if (value=="") {
        _gui.makeCall(_url()+"/opt?filter=null");
      } else {
        _gui.makeCall(_url()+"/opt?filter="+encodeURIComponent(value));
      }
      return false;
    } else {
      return true;
    }

  }
  this.addScheme = function(value) {
    if (value=="null") {
      return false;
    }
    if (this.scheme_array.length>=10) {
      alert("The scheme is too long! Please remove some elements before adding to it.");
      return false;
    }
    this.scheme_array.push(value);
    this.rebuildScheme();
    return false;
  }
  this.removeScheme = function(index) {
    if (index>=0 && index<this.scheme_array.length) {
      this.scheme_array.splice(index,1);
      this.rebuildScheme();
    }
    return false;
  }
  this.rebuildScheme = function() {
    this.span_scheme.innerHTML = "";
    for (s in this.scheme_array) {
      this.span_scheme.innerHTML += "<span style='border-style:solid;border-width:2px;background-color:LightCyan;margin-left:2px;margin-right:2px'>"+this.scheme_valid[this.scheme_array[s]]+" <img style='cursor:pointer' href='#' onclick='return GUI.Plugin.FileLightSite.removeScheme("+s+")' alt='Remove' src='"+ROOTPATH+"/static/x.gif'></span>";
    }
    return false;
  }
  this.sendScheme = function() {
    var scheme_str = this.scheme_array.join(":");
    _gui.makeCall(_url()+"/opt?scheme="+encodeURIComponent(scheme_str));
    return false;
  }
  this.update = function(data)
  {

    var width = data['width'];
    var height = data['height'];
    var type = data['type'];
    var site = data['site'];
    var sites = data['sites'];
    var group = data['group'];

    var incomplete = data['incomplete'];
    var subscribed = data['subscribed'];
    var custodial = data['custodial'];

    var filter = data['filter'];
    var scheme = data['scheme'];

    this.site_list = sites;
    this.site = site;

    var select_html = "<option value='null'></option>";
    for (i in sites) {
      if (sites[i]==site) {
        select_html += "<option value='"+sites[i]+"' selected>"+sites[i]+"</option>";
      } else {
        select_html += "<option value='"+sites[i]+"'>"+sites[i]+"</option>";
      }
    }
    this.select_site.innerHTML = select_html;

    var optstr = "?width="+width+";height="+height+";type="+type;
    if (group == "1") {
      optstr += ";group=1";
    }


    this.check_incomplete.checked = incomplete=="1";
    this.check_subscribed.checked = subscribed=="1";
    this.check_custodial.checked = custodial=="1";

    if (incomplete=="1") optstr += ";incomplete=1";
    if (subscribed=="1") optstr += ";subscribed=1";
    if (custodial=="1") optstr += ";custodial=1";

    if (filter=="null") {
      this.in_filter.value="";
    } else {
      this.in_filter.value = filter;
      optstr += ";filter="+encodeURIComponent(filter);
    }

    if (scheme=="null") {

    } else {
      this.scheme_array = scheme.split(":");
      this.rebuildScheme();
      optstr += ";scheme="+encodeURIComponent(scheme);
    }

    if (site != "null") {
      this.img.width = parseInt(width);
      this.img.height = parseInt(height);
      GUI.ImgLoader.load('filelight-site', this.img, ROOTPATH + '/plotfairy/filelight/site/'+encodeURIComponent(site)+optstr, _IMG_BORDER_NONE, _IMG_BORDER_NEW);
    }
  }
}






GUI.Plugin.FileLightGroup = new function()
{
  var _canvas = $('canvas');
  var _gui = null;
  var _self = this;

  this.img = null;
  this.select_group = null;
  this.group_list = null;
  this.group = null;
  this.in_width = null;
  this.in_height = null;
  this.in_type = null;

  this.attach = function(gui, linkcontent, preopts, postopts)
  {
    _canvas.style.top = 0;
    _canvas.style.bottom = 0;
    _gui = gui;

    var content = ""
      + "<div id='filelight-options' class='item'>"
      + "<img style='cursor:pointer' href='#' onclick='return GUI.Plugin.FileLightGroup.moveRel(-1)' alt='Previous' src='"+ROOTPATH+"/static/left.gif'>"
      + "<img style='cursor:pointer' href='#' onclick='return GUI.Plugin.FileLightGroup.moveRel(1)' alt='Previous' src='"+ROOTPATH+"/static/right.gif'>"
      + "Group: "
      + "<select id='filelight-group-select' onchange='return GUI.Plugin.FileLightGroup.updateGroup(this.value)'>"
      + "<option value='null'></option>"
      + "</select>"
      + "Size: <select id='filelight-size' onchange='return GUI.Plugin.FileLightGroup.updateSize(this.value)'><option value='400'>400x400</option><option value='800' selected>800x800</option><option value='1200'>1200x1200</option><option value='1600'>1600x1600</option><option value='2000'>2000x2000</option></select>"
      + "</div>"
      //+ "<div id='filelight-image-area' style='background-image:url(\""+ROOTPATH+"/static/progress.gif\");background-repeat:no-repeat;background-position:center;width:800px'>"
      + "<div id='filelight-image-area' style='float:left;clear:left'>"
      + "<img id='filelight-image' src='"+ROOTPATH+"/static/blank.gif' width=800 height=800>"
      + "</div>";


    if (_canvas.innerHTML != content) _canvas.innerHTML = content;

    if (_canvas.style.display != '') _canvas.style.display = '';

    this.img = $("filelight-image");
    this.select_group = $("filelight-group-select");

  }

  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.moveRel = function(n)
  {
    for (i in _self.group_list) {
      if (_self.group_list[i]==_self.group) {
        var index = (i+n) % _self.group_list.length;
        return this.updateGroup(_self.group_list[index]);
      }
    }
  }

  this.updateSize = function(value) {
    _gui.makeCall(_url()+"/opt?width="+encodeURIComponent(value)+";height="+encodeURIComponent(value));
  }

  this.updateGroup = function(group)
  {
    if (group != 'null') {
      _gui.makeCall(_url()+"/opt?group="+encodeURIComponent(group));
    }
  }


  this.update = function(data)
  {

    var width = data['width'];
    var height = data['height'];
    var type = data['type'];
    var group = data['group'];
    var groups = data['groups'];

    this.group = group;
    this.group_list = groups;

    var select_html = "<option value='null'></option>";
    for (i in groups) {
      if (groups[i]==group) {
        select_html += "<option value='"+groups[i]+"' selected>"+groups[i]+"</option>";
      } else {
        select_html += "<option value='"+groups[i]+"'>"+groups[i]+"</option>";
      }
    }
    this.select_group.innerHTML = select_html;

    var optstr = "?width="+width+";height="+height+";type="+type;

    if (group != "null") {
      this.img.width = parseInt(width);
      this.img.height = parseInt(height);
      GUI.ImgLoader.load('filelight-group', this.img, ROOTPATH + '/plotfairy/filelight/group/'+encodeURIComponent(group)+optstr, _IMG_BORDER_NONE, _IMG_BORDER_NEW);
    }
  }
}
