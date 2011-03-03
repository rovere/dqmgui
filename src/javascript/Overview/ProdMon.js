var _IMG_BORDER_NONE  = "1px solid transparent";
var _IMG_BORDER_NEW   = "1px solid #4a4";

GUI.Plugin.ProdMonSummary = new function()
{
  var _canvas = $('canvas');
  var _gui = null;
  var _self = this;

  this._divSite = null;
  this._divStats = null;
  this._divWF = null;
  this._divAlert = null;

  this._imageSite = null;
  this._imageWF = null;
  this._legendSite = null;
  this._legendWF = null;

  this._mapSite = null;
  this._mapWF = null;

  this.attach = function(gui, linkcontent, preopts, postopts)
  {
    _gui = gui;

    var content = ""
      //+ "<div class='item' style='float:left;clear:both;padding:.5em'>"
      //+ "<h2>Alerts</h2>"
      //+ "<div id='prodmon-alert-table' style='float:left'></div>"
      //+ "</div>"
      + "<table><tr><td>"
      + "<div class='item'>"
      + "<h3>Statistics</h3>"
      + "<div id='prodmon-stats-table'></div>"
      + "</div>"
      + "</td><td>"
      + "<div class='item'>"
      + "<h3>Sites:</h3>"
      + "<img id='prodmon-site-image' usemap='#prodmon-site-map' src='"+ROOTPATH+"/static/blank.gif' width=400>"
      + "<map name='prodmon-site-map' id='prodmon-site-map'></map>"
      + "</div>"
      + "</td></tr><tr><td>"
      + "<div class='item'>"
      + "<h3>Workflows:</h3>"
      + "<img id='prodmon-workflow-image' usemap='#prodmon-workflow-map' src='"+ROOTPATH+"/static/blank.gif' width=400>"
      + "<map name='prodmon-workflow-map' id='prodmon-workflow-map'></map>"
      + "</div>"
      + "</td><td>"
      + "<div class='item'>"
      + "<h3>Teams:</h3>"
      + "<img id='prodmon-team-image' usemap='#prodmon-team-map' src='"+ROOTPATH+"/static/blank.gif' width=400>"
      + "<map name='prodmon-team-map' id='prodmon-team-map'></map>"
      + "</div>"
      + "</td></tr></table>";


    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;

    if (_canvas.style.display != '')
      _canvas.style.display = '';

    this._divStats = document.getElementById('prodmon-stats-table');
    this._imageSite = document.getElementById('prodmon-site-image');
    this._imageWF = document.getElementById('prodmon-workflow-image');
    this._imageTeam = document.getElementById('prodmon-team-image');
    this._mapSite = document.getElementById('prodmon-site-map');
    this._mapWF = document.getElementById('prodmon-workflow-map');
    this._mapTeam = document.getElementById('prodmon-team-map');


    this._imageSite.src = ROOTPATH + "/plotfairy/prodmon/summary/site/plot+legend";
    this._imageWF.src = ROOTPATH + "/plotfairy/prodmon/summary/wf/plot+legend";
    this._imageTeam.src = ROOTPATH + "/plotfairy/prodmon/summary/team/plot+legend";
    GUI.ImgLoader.load('prodmon-summary-site', this._imageSite, ROOTPATH + "/plotfairy/prodmon/summary/site/plot+legend", _IMG_BORDER_NONE, _IMG_BORDER_NEW);
    GUI.ImgLoader.load('prodmon-summary-wf', this._imageWF, ROOTPATH + "/plotfairy/prodmon/summary/wf/plot+legend", _IMG_BORDER_NONE, _IMG_BORDER_NEW);
    GUI.ImgLoader.load('prodmon-summary-team', this._imageTeam, ROOTPATH + "/plotfairy/prodmon/summary/team/plot+legend", _IMG_BORDER_NONE, _IMG_BORDER_NEW);

    var c1 = YAHOO.util.Connect.asyncRequest('GET',ROOTPATH + "/plotfairy/prodmon/summary/site/map",{success:GUI.Plugin.ProdMonSummary.receiveText,argument:'site-map'});
    var c2 = YAHOO.util.Connect.asyncRequest('GET',ROOTPATH + "/plotfairy/prodmon/summary/wf/map",{success:GUI.Plugin.ProdMonSummary.receiveText,argument:'wf-map'});
    var c3 = YAHOO.util.Connect.asyncRequest('GET',ROOTPATH + "/plotfairy/prodmon/summary/team/map",{success:GUI.Plugin.ProdMonSummary.receiveText,argument:'team-map'});

  }

  this.receiveText = function(obj) {
    if (obj.argument=='site-map')
      _self._mapSite.innerHTML=obj.responseText;
    if (obj.argument=='wf-map')
      _self._mapWF.innerHTML=obj.responseText;
    if (obj.argument=='team-map')
      _self._mapTeam.innerHTML=obj.responseText;
  }

  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }



  this.update = function(data,info)
  {

    var stats = "<table>"
      + "<tr><td><b>Total workflows</b></td><td>"+data['workflow_count']+"</td></tr>"
      + "<tr><td><b>Total merged events</b></td><td>"+data['total_merged']+"</td></tr>"
      + "<tr><td><b>Total unmerged events</b></td><td>"+data['total_unmerged']+"</td></tr>"
      + "<tr><td><b>Average time/event</b></td><td>"+data['avg_time_per_event']+"</td></tr>"
      + "<tr><td><b>Minimum time/event</b></td><td>"+data['min_time_per_event']+"</td></tr>"
      + "<tr><td><b>Maximum time/event</b></td><td>"+data['max_time_per_event']+"</td></tr>"
      + "</table>";
    _self._divStats.innerHTML=stats;

  }
}();

GUI.Plugin.ProdMonPlot = new function()
{
  var _canvas = $('canvas');
  var _gui = null;
  var _self = this;

  this._div_plotmove = null;
  this._div_plotselect = null;
  this._div_open = null;
  this._div_plot = null;
  this._div_time = null;
  this._div_group = null;
  this._div_filter = null;
  this._div_image = null;
  this._div_download = null;
  this.filtercount=0;
  this.current=0;

  this.attach = function(gui, linkcontent, preopts, postopts)
  {
    _gui = gui;
    _canvas.style.top = 0;
    _canvas.style.bottom = 0;

    var content = ""
      + "<div id='prodmon-topbar'>"
      + "<div id='prodmon-plotmove'></div>"
      + "<div id='prodmon-plotselect'></div>"
      + "</div>"
      + "<div style='width:100%;position:relative;clear:left'>"
      + "<div id='prodmon-leftbar'>"
      + "<div id='prodmon-open'></div>"
      + "</div>"
      + "<div id='prodmon-plot'></div>"
      + "<div id='prodmon-rightbar'>"
      + "<div id='prodmon-time' class='slide'></div>"
      + "<div id='prodmon-group' class='slide'></div>"
      + "<div id='prodmon-filter' class='slide'></div>"
      + "<div id='prodmon-image' class='slide'></div>"
      + "<div id='prodmon-download' class='slide'></div>"
      + "</div>"
      + "</div>";




    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;

    if (_canvas.style.display != '')
      _canvas.style.display = '';

    this._div_plotmove = $("prodmon-plotmove");
    this._div_plotselect = $("prodmon-plotselect");
    this._div_open = $("prodmon-open");
    this._div_plot = $("prodmon-plot");
    this._div_time = $("prodmon-time");
    this._div_group = $("prodmon-group");
    this._div_filter = $("prodmon-filter");
    this._div_image = $("prodmon-image");
    this._div_download = $("prodmon-download");

    this._div_plotmove.innerHTML = ""
      + "<img class='button' href='#' onclick='return GUI.Plugin.ProdMonPlot.movePlotRel(-1)' alt='Previous' src='"+ROOTPATH+"/static/left.gif'>"
      + "<img class='button' href='#' onclick='return GUI.Plugin.ProdMonPlot.closePlot()' alt='Close' src='"+ROOTPATH+"/static/x.gif'>"
      + "<img class='button' href='#' onclick='return GUI.Plugin.ProdMonPlot.movePlotRel(1)' alt='Next' src='"+ROOTPATH+"/static/right.gif'>";


    this._div_plotselect.innerHTML = ""
      + "Show "
      + "<select id='prodmon-what' onchange='return GUI.Plugin.ProdMonPlot.updateWhat(this.value)'>"
      + "<option value='evts_read'>Events Read</option>"
      + "<option value='evts_written'>Events Written</option>"
      + "<option value='time'>Time</option>"
      + "<option value='jobs'>Jobs</option>"
      + "<option value='CMSSW_fail_codes'>CMSSW Fail Codes</option>"
      + "<option value='time_by_span'>Approximate Batch Slot Usage</option>"
      + "<option value='reco_rate'>RECO Rate</option>"
      + "<option value='dts_timings'>Dataset Timings</option>"
      + "<option value='jobs_success_rates'>Job Success Rates</option>"
      + "<option value='jobs_failure_types'>Job Failure Types</option>"
      + "</select>"
      + "<select id='prodmon-what-what' onchange='return GUI.Plugin.ProdMonPlot.updateWhatWhat(this.value)'>"
      + "<option value='_any'>All</option>"
      + "<option value='_failed'>Failed</option>"
      + "<option value='_success'>Succeeded</option>"
      + "</select>"
      + " grouped by "
      + "<select id='prodmon-sortby' onchange='return GUI.Plugin.ProdMonPlot.updateSortBy(this.value)'>"
      + "<option value='site'>Sites</option>"
      + "<option value='wf'>Workflows</option>"
      + "<option value='dts'>Datasets</option>"
      + "<option value='prod_team'>Production Team</option>"
      + "<option value='prod_agent'>ProdAgent</option>"
      + "<option value='exit_code'>CMSSW Exit Code</option>"
      + "</select>"
      + " for job types "
      + "<select id='prodmon-job_type' onchange='return GUI.Plugin.ProdMonPlot.updateJobType(this.value)'>"
      + "<option value='Any'>Any</option>"
      + "<option value='Merge'>Merge</option>"
      + "<option value='Processing'>Processing</option>"
      + "</select>"
      + " using "
      + "<select id='prodmon-graph' onchange='return GUI.Plugin.ProdMonPlot.updateGraph(this.value)'>"
      + "<option value='bar'>Bar</option>"
      + "<option value='pie'>Pie</option>"
      + "<option value='cumulative'>Cumulative</option>"
      + "<option value='baobab'>Baobab</option>"
      + "</select>"
      +	" <a href='#' onclick='GUI.Plugin.ProdMonPlot.updateTitle(-1)'>Rename</a>"
      + " <a href='#' onclick='return GUI.Plugin.ProdMonPlot.addPlot(0)'>New Plot</a>";


    this._div_open.innerHTML="open";
    this._div_plot.innerHTML=""
      + "<div>"
      + "<img id='prodmon-plot-image' usemap='#prodmon-plot-map' src='"+ROOTPATH+"/static/blank.gif' width=800>"
      + "<map name='prodmon-plot-map' id='prodmon-plot-map'></map>"
      + "</div>";

    this._div_time.innerHTML=""
      + "<table><tr><td class='slide-tab'><a title='Time Options' href='#' onclick='return GUI.Plugin.ProdMonPlot.toggleVisible(\"prodmon-time-area\")'><img src='"+ROOTPATH+"/static/time.gif' alt='Time Options'></a></td><td>"
      + "<div id='prodmon-time-area' class='slide-area'>"
      + "<div><h3>Time Options</h3></div>"
      + "<div><h4>Interval</h4></div>"
      + "<div>"
      + "<select id='prodmon-period' onchange='return GUI.Plugin.ProdMonPlot.updateTime(this.value)'>"
      + "<option value='12'>12 Hours</option>"
      + "<option value='24'>1 Day</option>"
      + "<option value='48' selected='selected'>2 Days</option>"
      + "<option value='96'>4 Days</option>"
      + "<option value='168'>1 Week</option>"
      + "<option value='336'>2 Weeks</option>"
      + "<option value='672'>1 Month</option>"
      + "<option value='2016'>3 Months</option>"
      + "</select>"
      + "</div>"
      + "<div><h4>Absolute</h4></div>"
      + "<div>"
      + "Start <input id='prodmon-time-start' value='YYYY-MM-DD HH:MM:SS' size=20> "
      + "Finish <input id='prodmon-time-end' value='YYYY-MM-DD HH:MM:SS' size=20> "
      + "<a href='#' onclick='return GUI.Plugin.ProdMonPlot.resetTime()'>Now</a>"
      + " <a href='#' onclick='return GUI.Plugin.ProdMonPlot.updateTime2()'>Submit</a>"
      + "</div>"
      + "</div></td></tr></table>";
    this._div_group.innerHTML=""
      + "<table><tr><td class='slide-tab'><a title='Group Options' href='#' onclick='return GUI.Plugin.ProdMonPlot.toggleVisible(\"prodmon-group-area\")'><img src='"+ROOTPATH+"/static/group.gif' alt='Group Options'></a></td><td>"
      + "<div id='prodmon-group-area' class='slide-area'>"
      + "<div><h3>Group Options</h3></div>"
      + "<div><h4>Additional data grouping</h4></div>"
      + "<div>"
      + "<select id='prodmon-group-select' onchange='return GUI.Plugin.ProdMonPlot.updateGroup(this.value)'>"
      + "<option value='null'>None</option>"
      + "</select>"
      + "</div>"
      + "</div></td></tr></table>";
    this._div_filter.innerHTML=""
      + "<table><tr><td class='slide-tab'><a title='Filter Options' href='#' onclick='return GUI.Plugin.ProdMonPlot.toggleVisible(\"prodmon-filter-area\")'><img src='"+ROOTPATH+"/static/filter.gif' alt='Filter Options'></a></td><td>"
      + "<div id='prodmon-filter-area' class='slide-area'>"
      + "<div><h3>Filter Options</h3></div>"
      + "<div><h4>Regex (on names)</h4></div>"
      + "<div>"
      + "Regex <input id='prodmon-regex' value=''> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.updateRegex()'>Submit</a>"
      + "</div>"
      + "<div><h4>Filter List</h4></div>"
      + "<div>"
      + "<a href='#' onclick='return GUI.Plugin.ProdMonPlot.addFilter()'>Add</a>"
      + " <a href='#' onclick='return GUI.Plugin.ProdMonPlot.clearFilters()'>Clear</a>"
      + " <a href='#' onclick='return GUI.Plugin.ProdMonPlot.updateFilters()'>Apply</a>"
      + "</div>"
      + "<div id='prodmon-filters'>"
      + "</div>"
      + "</div></td></tr></table>";
    this._div_image.innerHTML=""
      + "<table><tr><td class='slide-tab'><a title='Image Options' href='#' onclick='return GUI.Plugin.ProdMonPlot.toggleVisible(\"prodmon-image-area\")'><img src='"+ROOTPATH+"/static/image.gif' alt='Image Options'></a></td><td>"
      + "<div id='prodmon-image-area' class='slide-area'>"
      + "<div><h3>Image Options</h3></div>"
      + "<div><h4>Preset</h4></div>"
      + "<div>"
      + "<select id='prodmon-resolution' onchange='return GUI.Plugin.ProdMonPlot.updateImage(this.value)'>"
      + "<option value='320'>320x240 QVGA</option>"
      + "<option value='640'>640x480 VGA</option>"
      + "<option value='800' selected='true'>800x600 SVGA</option>"
      + "<option value='1024'>1024x768 XGA</option>"
      + "<option value='1280'>1280x1024 SXGA</option>"
      + "<option value='1600'>1600x1200 UXGA</option>"
      + "</select>"
      + "</div>"
      + "<div><h4>Custom</h4></div>"
      + "<div>"
      + "Width <input id='prodmon-image-width' value='800' size=4> "
      + "Height <input id='prodmon-image-height' value='600' size=4> "
      + "DPI <input id='prodmon-image-dpi' value='72' size=4> "
      + "<a href='#' onclick='return GUI.Plugin.ProdMonPlot.updateImage3()'>Submit</a>"
      + "</div>"
      + "</div></td></tr></table>";

    this._div_download.innerHTML=""
      + "<table><tr><td class='slide-tab'><a title='Download Options' href='#' onclick='return GUI.Plugin.ProdMonPlot.toggleVisible(\"prodmon-download-area\")'><img src='"+ROOTPATH+"/static/download.gif' alt='Download Options'></a></td><td>"
      + "<div id='prodmon-download-area' class='slide-area'>"
      + "<div><h3>Download</h3></div>"
      + "<div>Plot: <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot\",\"png\")'>PNG</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot\",\"pdf\")'>PDF</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot\",\"svg\")'>SVG</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot\",\"ps\")'>PS</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot\",\"json\")'>JSON</a></div>"
      + "<div>Legend: <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"legend\",\"png\")'>PNG</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"legend\",\"pdf\")'>PDF</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"legend\",\"svg\")'>SVG</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"legend\",\"ps\")'>PS</a></div>"
      + "<div>Plot+Legend: <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot+legend\",\"png\")'>PNG</a> <a href='#' onclick='return GUI.Plugin.ProdMonPlot.download(\"plot+legend\",\"svg\")'>SVG</a></div>"
      + "</div></td></tr></table>";

  }
  this.toggleVisible=function(id) {
    elem = $(id);
    if (elem.style.display=='block') {
      elem.style.display='none';
    } else {
      elem.style.display='block';
    }
  }
  this.movePlotRel = function(rel) {
    _gui.makeCall(_url()+"/move?index="+encodeURIComponent(_self.current+rel));
  }
  this.movePlotAbs = function(index) {
    _gui.makeCall(_url()+"/move?index="+encodeURIComponent(index));
  }
  this.closePlot = function(index) {
    if (index==null)
      index = _self.current;
    _gui.makeCall(_url()+"/close?index="+encodeURIComponent(index));
  }
  this.addPlot = function(copy) {
    _gui.makeCall(_url()+"/add?copy="+encodeURIComponent(copy)+";index="+encodeURIComponent(_self.current));
  }
  this.download = function(what,format) {
    window.location.href = $('prodmon-plot-image').src.replace(/plot\+legend/,what) + ";type=" + format.toLowerCase();
    return false;
  }
  this.addFilter = function() {
    //alert("addfilter");
    var elem = $("prodmon-filters");
    var index = _self.filtercount;
    _self.filtercount += 1;
    var newhtml = ""
      //+ "<div id='prodmon-filter"+index+"-div'>"
      + "<div>"
      + "<a href='#' onclick='return GUI.Plugin.ProdMonPlot.removeFilter("+index+")'>-</a>"
      + "<select id='prodmon-filter"+index+"-select'>"
      + "<option value='null'></option>"
      + "<option value='wf'>Workflow</option>"
      + "<option value='dts'>Dataset</option>"
      + "<option value='site'>Site</option>"
      + "<option value='prod_team'>Production Team</option>"
      + "<option value='prod_agent'>ProdAgent</option>"
      + "<option value='exit_code'>CMSSW Exit Code</option>"
      + "</select>"
      //+ "<input id='prodmon-filter"+index+"-input' value='' onkeypress='return GUI.Plugin.ProdMonPlot.filterAutocomplete("+index+",this.value)'>"
      + "<input id='prodmon-filter"+index+"-input' value=''>"
      + "</div>"
      + "<div id='prodmon-filter"+index+"-suggest'>"
      + "</div>";
      //+ "</div>";
    var newdiv = document.createElement('div');
    newdiv.setAttribute('id',"prodmon-filter"+index+"-div");
    newdiv.innerHTML = newhtml;

    //elem.innerHTML = elem.innerHTML + newhtml;
    elem.appendChild(newdiv);
  }
  this.clearFilters = function() {
    _self.rebuildFilters([],-1);
    _gui.makeCall(_url()+"/filters?clear=true");
  }
  this.removeFilter = function(index) {
    _self.rebuildFilters(_self.filterList(),index);
  }
  this.updateWhat=function(what) {
    var elem = $('prodmon-what-what');
    if ((what=='evts_read') || (what=='evts_written') || (what=='time') || (what=='jobs')) {
      elem.disabled=false;
      var val = what+elem.value;
      _gui.makeCall(_url()+"/graph?what="+encodeURIComponent(val));
    } else {
      elem.disabled=true;
      _gui.makeCall(_url()+"/graph?what="+encodeURIComponent(what));
    }
  }
  this.updateWhatWhat=function(whatwhat) {
    var elem = document.getElementById('prodmon-what');
    var val = elem.value+whatwhat;
    _gui.makeCall(_url()+"/graph?what="+encodeURIComponent(val));
  }
  this.updateSortBy=function(sortby) {
    _gui.makeCall(_url()+"/graph?sortby="+encodeURIComponent(sortby));
  }
  this.updateJobType=function(job_type) {
    _gui.makeCall(_url()+"/graph?job_type="+encodeURIComponent(job_type));
  }
  this.updateGraph=function(graph) {
    _gui.makeCall(_url()+"/graph?graph="+encodeURIComponent(graph));
  }
  this.updateTime=function(time) {
    _gui.makeCall(_url()+"/graph?use_period=true;period="+encodeURIComponent(time));
  }
  this.updateTime2=function() {
    var start = $('prodmon-time-start').value;
    var end = $('prodmon-time-end').value;
    var pattern = /^(\d{4})-(\d{1,2})-(\d{1,2}) (\d{2}):(\d{2}):(\d{2})$/;
    var startmatch = start.match(pattern);
    if (startmatch==null) {
      alert("Start time does not match required format (YYYY-MM-DD HH:MM:SS)");
      return;
    }
    var endmatch = end.match(pattern);
    if (endmatch==null) {
      alert("Finish time does not match required format (YYYY-MM-DD HH:MM:SS)");
      return;
    }
    _gui.makeCall(_url()+"/graph?use_period=false;starttime="+encodeURIComponent(start)+";endtime="+encodeURIComponent(end));
  }
  this.resetTime=function() {
    $('prodmon-time-start').value = _self.formatTime(-172800000);
    $('prodmon-time-end').value = _self.formatTime(0);
  }
  this.updateGroup=function(group) {
    _gui.makeCall(_url()+"/graph?group="+encodeURIComponent(group));
  }
  this.updateRegex=function() {
    var elem = $('prodmon-regex');
    var regex = elem.value;
    if (regex=='') regex='null';
    _gui.makeCall(_url()+"/graph?regex="+encodeURIComponent(regex));
  }
  this.updateFilters=function() {
    var flist = _self.filterList();
    if (flist.length==0) {
      _gui.makeCall(_url()+"/filters?clear=true");
    } else {
      var ftype=[];
      var fval=[];
      for (i in flist) {
        if (flist[i][0]!='null' && flist[i][1]!='') {
          ftype.push(flist[i][0]);
          fval.push(flist[i][1]);
        }
      }
      _gui.makeCall(_url()+"/filters?name="+encodeURIComponent(ftype.join('|'))+";value="+encodeURIComponent(fval.join('|')));
    }
  }
  this.updateImage=function(w) {
    switch(w) {
      case '320':
        _gui.makeCall(_url()+"/graph?image_width=320;image_height=240;image_dpi=72");
        break;
      case '640':
        _gui.makeCall(_url()+"/graph?image_width=640;image_height=480;image_dpi=72");
        break;
      case '800':
        _gui.makeCall(_url()+"/graph?image_width=800;image_height=600;image_dpi=72");
        break;
      case '1024':
        _gui.makeCall(_url()+"/graph?image_width=1024;image_height=768;image_dpi=72");
        break;
      case '1280':
        _gui.makeCall(_url()+"/graph?image_width=1280;image_height=1024;image_dpi=72");
        break;
      case '1600':
        _gui.makeCall(_url()+"/graph?image_width=1600;image_height=1200;image_dpi=72");
        break;
    }
  }
  this.updateImage3=function() {
    var w = $('prodmon-image-width').value;
    var h = $('prodmon-image-height').value;
    var dpi = $('prodmon-image-dpi').value;
    _gui.makeCall(_url()+"/graph?image_width="+encodeURIComponent(w)+";image_height="+encodeURIComponent(h)+";image_dpi="+encodeURIComponent(dpi));
  }
  this.filterAutocomplete=function(index,text) {
    var context = $("prodmon-filter"+index+"-select").value;
    var url = ROOTPATH + "/plotfairy/prodmon/autocomplete/"+encodeURIComponent(context)+"/"+encodeURIComponent(text);
    var c = YAHOO.util.Connect.asyncRequest('GET',url,{success:GUI.Plugin.ProdMonPlot.receiveText,argument:"filter"+index});
  }
  this.receiveText = function(obj) {
      //alert('receivetext');
      if (obj.argument=='plot-map') {
        document.getElementById("prodmon-plot-map").innerHTML = obj.responseText;
      }
      if (obj.argument.substr(0,6)=='filter') {
        var index = parseInt(obj.argument.substr(6));
        var suggestlist = eval('('+obj.responseText+')');
        if (suggestlist.length==1) {
          document.getElementById("prodmon-filter"+index+"-input").value=suggestlist[0];
          document.getElementById("prodmon-filter"+index+"-suggest").innerHTML="";
        } else {
          var newhtml = "<ul>";
          for (i in suggestlist) {
            newhtml += "<li><a href='#' onclick='return GUI.Plugin.ProdMonPlot.applySuggest("+index+",\""+suggestlist[i]+"\")'>"+suggestlist[i]+"</a></li>";
            if (i==10) {
              newhtml += "<li>...</li>";
              break;
            }
          }
          newhtml += "</ul>";
          document.getElementById("prodmon-filter"+index+"-suggest").innerHTML=newhtml;
        }
      }
      //alert('done receivetext');
  }
  this.formatTime = function(msoffset) {
    var now = new Date();
    now.setTime(now.getTime()+msoffset);
    //return now.getUTCFullYear()+"-"+(now.getUTCMonth()+1)+"-"+now.getUTCDate()+" "+now.getUTCHours()+":"+now.getUTCMinutes()+":"+now.getUTCSeconds();
    return sprintf("%04d-%02d-%02d %02d:%02d:%02d",now.getUTCFullYear(),now.getUTCMonth()+1,now.getUTCDate(),now.getUTCHours(),now.getUTCMinutes(),now.getUTCSeconds());
  }
  this.setSelection = function(listobj,which) {
    for (var i=0;i<listobj.length;i++){
      if (listobj.options[i].value==which){
        listobj.selectedIndex = i;
      }
    }
  }
  this.filterList = function() {
    var flist = [];
    for (i=0;i<_self.filtercount;i++) {
      var name = document.getElementById("prodmon-filter"+i+"-select").value;
      var val = document.getElementById("prodmon-filter"+i+"-input").value;
      flist.push([name,val]);
    }
    return flist;
  }
  this.rebuildFilters = function(filterlist,remove) {
    //alert('rebuildfilters');
    _self.filtercount=0;
    var elem = document.getElementById('prodmon-filters');
    while(elem.firstChild) elem.removeChild(elem.firstChild);
    if (remove>=0) {
      filterlist.splice(remove,1);
    }
    for (i in filterlist) { _self.addFilter(); }
    for (i in filterlist) {
      //alert('filter '+i+' select='+filterlist[i][0]+' value='+filterlist[i][1]);
      _self.setSelection(document.getElementById("prodmon-filter"+i+"-select"),filterlist[i][0]);
      document.getElementById("prodmon-filter"+i+"-input").value=filterlist[i][1];
    }
    //alert('done rebuildfilters');
  }
  this.setGroupChoices = function(sortby) {
    //alert('setgroupchoices');
    var choices = [];
    if (sortby=='site') {
       choices = [['null',''],['tier','Tier'],['country','Country'],['tiercountry','Tier, Country'],['top5site','Top 5']];
    }
    if (sortby=='wf') {
      choices = [['null',''],['1','Primary'],['2','Secondary'],['3','Tertiary'],['top5wf','Top 5']];
    }
    if (sortby=='dts') {
      choices = [['null',''],['generator','Generator/Dataset'],['campaign','Production Campaign'],['conditions','Conditions'],['tiers','Data Tiers'],['merged','Merge Status']];
    }
    if (sortby=='exit_code') {
      choices = [['null',''],['10k','10k']];
    }
    var newhtml="";
    for (i in choices) {
      newhtml += "<option value='"+choices[i][0]+"'>"+choices[i][1]+"</option>";
    }
    document.getElementById('prodmon-group-select').innerHTML=newhtml;
  }
  this.detach = function() {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }
  this.buildOpen=function(open) {
    while(this._div_open.firstChild) this._div_open.removeChild(this._div_open.firstChild);
    for (o in open) {
      var index = open[o]['index'];
      var title = open[o]['title'];
      var baseurl = open[o]['baseurl'];
      var optstr = open[o]['optstr'];
      var thedate = new Date();
      optstr += ";seq=" + thedate.valueOf();
      var url = ROOTPATH+baseurl+'thumbnail?'+optstr;
      var divhtml = "<div class='open-plot'>";
      if (index==this.current) {
        divhtml = "<div class='open-plot selected'>";
      }
      divhtml=divhtml
        + "<div style='position:relative'>"
        + "<img class='open-plot' src='"+url+"' onclick='return GUI.Plugin.ProdMonPlot.movePlotAbs("+index+")' alt='"+title+"'>"
        + "<a class='topright' href='#' onclick='return GUI.Plugin.ProdMonPlot.closePlot("+index+")'>X</a>"
        + "</div>"
        + "<div>"
        + "<a class='below-plot' href='#' onclick='return GUI.Plugin.ProdMonPlot.movePlotAbs("+index+")'>"+title+"</a>"
        + "</div>"
        + "</div>";
      this._div_open.innerHTML += divhtml;
    }
  }

  this.updateTitle=function(index) {
    if (index==null) index=_self.current;
    var newtitle = window.prompt("New title?","");
    _gui.makeCall(_url()+"/graph?title="+encodeURIComponent(newtitle));
  }

  this.update=function(data,info) {

    var what = data['what'];
    var sortby = data['sortby'];
    var job_type = data['job_type'];
    var group = data['group'];
    var period = parseInt(data['period']);
    var use_period = data['use_period'];
    var starttime = data['starttime'];
    var endtime = data['endtime'];
    var graph = data['graph'];
    var regex = data['regex'];
    var filters = data['filters'];
    var group = data['group'];
    var image_width = parseInt(data['image_width']);
    var image_height = parseInt(data['image_height']);
    var image_dpi = parseInt(data['image_dpi']);
    var open = data['open'];
    var current = data['current'];
    var baseurl = data['baseurl'];
    var optstr = data['optstr'];
    var title = data['title'];

    if ((what.substr(0,4)=='evts') || (what.substr(0,4)=='jobs' && !(what == 'jobs_success_rates' || what=='jobs_failure_types')) || (what.substr(0,4)=='time' && !(what == 'time_by_span'))) {
      this.setSelection($('prodmon-what'),what.substr(0,4));
      $('prodmon-what-what').disabled=false;
      this.setSelection($('prodmon-what-what'),what.substr(4));
    } else {
      this.setSelection($('prodmon-what'),what);
      $('prodmon-what-what').disabled=true;
    }
    this.setSelection($('prodmon-sortby'),sortby);
    this.setSelection($('prodmon-job_type'),job_type);
    this.setSelection($('prodmon-graph'),graph);
    this.setGroupChoices(sortby);
    this.setSelection($('prodmon-group-select'),group);
    this.rebuildFilters(filters,-1);
    if (use_period) {
      this.setSelection($('prodmon-period'),period);
    } else {
      $('prodmon-time-start').value = starttime;
      $('prodmon-time-end').value = endtime;
    }
    $('prodmon-regex').value = regex;
    this.setSelection($('prodmon-resolution'),image_width);
    $('prodmon-image-width').value=image_width;
    $('prodmon-image-height').value=image_height;
    $('prodmon-image-dpi').value=image_dpi;

    $('prodmon-plot-image').width=image_width;
    this.current = current;

    this.buildOpen(open);
    $('prodmon-plot-image').width = image_width;
    //$('prodmon-plot-image').height = image_height;
    var thedate = new Date();
    optstr += ";seq=" + thedate.valueOf();
    GUI.ImgLoader.load('prodmon-freeplot-plot', $('prodmon-plot-image'), ROOTPATH+baseurl+'plot+legend?'+optstr, _IMG_BORDER_NONE, _IMG_BORDER_NEW);

  }

  this.handleMapClick = function(url) {
      GUI.makeCall(_url() + "/"+ url);
  }


  return this;
}();
