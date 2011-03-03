GUI.Plugin.CompPhEDExLinkBase = new function()
{
  var _self = this;
  var _gui = null;
  var _canvas = $('canvas');

  this._optPeriodTime = null;
  this._optPeriodSpan = null;
  this._optPeriodUpto = null;
  this._optQuantity = null;
  this._optGrouping = null;
  this._optSrcGrouping = null;
  this._optLinks = null;
  this._optFilterFrom = null;
  this._optFilterTo = null;
  this._optDBProd = null;
  this._optDBDebug = null;
  this._optDBDev = null;
  this._optDBTestbed = null;
  this._optDBTestbed2 = null;

  this.attach = function(gui, linkcontent, preopts, postopts)
  {
    _gui = gui;
    var content = ""
      + "<table border='0' style='font-size:85%; margin:0 0'><tr valign='top'>"
      + "<td id='link-content'>" + linkcontent + "</td>"

      + "<td style='padding:6em 1em 0 0'>"
      + "<table id='link-options' border='0' style='white-space: nowrap'>"
      + preopts
      + "<tr id='opt-period-time' valign='top'><td>Period</td><td>"
      + "<input style='font-size:100%' id='period-time' type='text' size='4' /> "
      + "<select style='font-size:100%' id='period-span'>"
      + "<option value='hour' selected='selected'>Hours</option>"
      + "<option value='day'>Days</option>"
      + "<option value='week'>Weeks</option>"
      + "<option value='month'>Months</option>"
      + "<option value='year'>Years</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-period-upto' valign='top'><td>Up to</td><td>"
      + "<input style='font-size:100%' id='period-upto' type='text' size='16' />"
      + "</tr>"
      + "<tr id='opt-qty' valign='top'><td>Quantity</td><td>"
      + "<select style='font-size:100%' id='qty'>"
      + "<option value='destination' selected='selected'>Destination</option>"
      + "<option value='source'>Source</option>"
      + "<option value='link'>Link</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-src-grouping' valign='top'><td>Source grouping</td><td>"
      + "<select style='font-size:100%' id='src-grouping' disabled='disabled'>"
      + "<option value='same' selected='selected'>As destination</option>"
      + "<option value='db'>Database instance</option>"
      + "<option value='node'>Storage node</option>"
      + "<option value='site'>Storage site</option>"
      + "<option value='country'>Country</option>"
      + "<option value='region'>Region</option>"
      + "<option value='tier'>Tier</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-grouping' valign='top'><td>Grouping</td><td>"
      + "<select style='font-size:100%' id='grouping'>"
      + "<option value='db'>Database instance</option>"
      + "<option value='node' selected='selected'>Storage node</option>"
      + "<option value='site'>Storage site</option>"
      + "<option value='country'>Country</option>"
      + "<option value='region'>Region</option>"
      + "<option value='tier'>Tier</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-links' valign='top'><td>Tape storage</td><td>"
      + "<select style='font-size:100%' id='links'>"
      + "<option value='nonmss' selected='selected'>Exclude</option>"
      + "<option value='mss'>Just tape</option>"
      + "<option value='all'>Include</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-from-filter' valign='top'><td>From</td><td>"
      + "<input style='font-size:100%' id='from-filter' type='text' size='16' />"
      + "</tr>"
      + "<tr id='opt-to-filter' valign='top'><td>To</td><td>"
      + "<input style='font-size:100%' id='to-filter' type='text' size='16' />"
      + "</tr>"
      + "<tr id='opt-db-sel' valign='top'><td>Databases</td><td>"
      + "Production:<br />"
      + "<input style='font-size:100%; margin-left:2em' id='db-prod' type='checkbox' /> Production<br />"
      + "<input style='font-size:100%; margin-left:2em' id='db-debug' type='checkbox' /> Debug<br />"
      + "Development:<br />"
      + "<input style='font-size:100%; margin-left:2em' id='db-dev' type='checkbox' /> Dev<br />"
      + "<input style='font-size:100%; margin-left:2em' id='db-testbed' type='checkbox' /> Testbed<br />"
      + "<input style='font-size:100%; margin-left:2em' id='db-testbed2' type='checkbox' /> Testbed 2<br />"
      + "</tr>"
      + postopts
      + "</table>"
      + "</td></tr></table>";

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;

    this._optPeriodTime  = $('period-time');
    this._optPeriodSpan  = $('period-span');
    this._optPeriodUpto  = $('period-upto');
    this._optQuantity    = $('qty');
    this._optGrouping    = $('grouping');
    this._optSrcGrouping = $('src-grouping');
    this._optLinks       = $('links');
    this._optFilterFrom  = $('from-filter');
    this._optFilterTo    = $('to-filter');
    this._optDBProd      = $('db-prod');
    this._optDBDebug     = $('db-debug');
    this._optDBDev       = $('db-dev');
    this._optDBTestbed   = $('db-testbed');
    this._optDBTestbed2  = $('db-testbed2');

    this._optPeriodTime.onchange  = function() { _self.setPeriod(); return false; }
    this._optPeriodSpan.onchange  = function() { _self.setPeriod(); return false; }
    this._optPeriodUpto.onchange  = function() { _self.setPeriod(); return false; }
    this._optQuantity.onchange    = function() { _self.setOption(this.id, this.value); return false; }
    this._optGrouping.onchange    = function() { _self.setOption(this.id, this.value); return false; }
    this._optSrcGrouping.onchange = function() { _self.setOption(this.id, this.value); return false; }
    this._optLinks.onchange       = function() { _self.setOption(this.id, this.value); return false; }
    this._optFilterFrom.onchange  = function() { _self.setOption('from', this.value); return false; }
    this._optFilterTo.onchange    = function() { _self.setOption('to', this.value); return false; }
    this._optDBProd.onchange      = function() { _self.setDatabases(this.id, this.checked); return false; }
    this._optDBDebug.onchange     = function() { _self.setDatabases(this.id, this.checked); return false; }
    this._optDBDev.onchange       = function() { _self.setDatabases(this.id, this.checked); return false; }
    this._optDBTestbed.onchange   = function() { _self.setDatabases(this.id, this.checked); return false; }
    this._optDBTestbed2.onchange  = function() { _self.setDatabases(this.id, this.checked); return false; }
  }

  this.detach = function()
  {
    this._optPeriodTime = null;
    this._optPeriodSpan = null;
    this._optPeriodUpto = null;
    this._optQuantity = null;
    this._optGrouping = null;
    this._optSrcGrouping = null;
    this._optLinks = null;
    this._optFilterFrom = null;
    this._optFilterTo = null;
    this._optDBProd = null;
    this._optDBDebug = null;
    this._optDBDev = null;
    this._optDBTestbed = null;
    this._optDBTestbed2 = null;
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.setOption = function(option, value)
  {
    _gui.makeCall(_url()
		  + "/alter?opt=" + encodeURIComponent(option)
		  + ";val=" + encodeURIComponent(value));
  }

  this.setPeriod = function()
  {
    _gui.makeCall(_url() + "/period"
		  + "?span=" + encodeURIComponent(this._optPeriodSpan.value)
		  + ";time=" + encodeURIComponent(this._optPeriodTime.value)
		  + ";upto=" + encodeURIComponent(this._optPeriodUpto.value));
  }

  this.setDatabases = function(changed, checked)
  {
    _gui.makeCall(_url() + "/databases"
		  + "?prod="     + (this._optDBProd.checked ? "yes" : "no")
		  + ";debug="    + (this._optDBDebug.checked ? "yes" : "no")
		  + ";dev="      + (this._optDBDev.checked ? "yes" : "no")
		  + ";testbed="  + (this._optDBTestbed.checked ? "yes" : "no")
		  + ";testbed2=" + (this._optDBTestbed2.checked ? "yes" : "no"));
  }

  this.selectOption = function(sel, item, def)
  {
    for (var i = 0, n = sel.firstChild; n; n = n.nextSibling, ++i)
      if (n.value == item)
      {
	sel.selectedIndex = i;
	return;
      }

    sel.selectedIndex = def;
  }

  this.optionValue = function(sel)
  {
    return encodeURIComponent(sel.childNodes[sel.selectedIndex].value);
  }

  this.update = function(data, info)
  {
    if (data['period.time'] != null)
      this._optPeriodTime.value = data['period.time'];

    if (data['period.upto'] != null)
      this._optPeriodUpto.value = data['period.upto'];

    this.selectOption(this._optPeriodSpan, data['period.span'], 0);
    this.selectOption(this._optQuantity, data['qty'], 0);
    this.selectOption(this._optGrouping, data['grouping'], 1);
    this.selectOption(this._optSrcGrouping, data['src-grouping'], 0);
    this.selectOption(this._optLinks, data['links'], 0);

    if (this._optQuantity.selectedIndex != 2)
    {
      this._optGrouping.parentNode.previousSibling.innerHTML = "Grouping";
      this._optSrcGrouping.disabled = true;
    }
    else
    {
      this._optGrouping.parentNode.previousSibling.innerHTML = "Destination grouping";
      this._optSrcGrouping.disabled = false;
    }

    if (data['filter.from'] != null)
      this._optFilterFrom.value = data['filter.from'];

    if (data['filter.to'] != null)
      this._optFilterTo.value = data['filter.to'];

    this._optDBProd.checked = (data['db.prod'] == 'yes');
    this._optDBDebug.checked = (data['db.debug'] == 'yes');
    this._optDBDev.checked = (data['db.dev'] == 'yes');
    this._optDBTestbed.checked = (data['db.testbed'] == 'yes');
    this._optDBTestbed2.checked = (data['db.testbed2'] == 'yes');

    info.dbsel = (data['db.prod'] == 'yes' ? "prod" : "")
	         + (data['db.debug'] == 'yes' ? "+debug" : "")
	         + (data['db.dev'] == 'yes' ? "+dev" : "")
	         + (data['db.testbed'] == 'yes' ? "+testbed" : "")
	         + (data['db.testbed2'] == 'yes' ? "+testbed2" : "");
    if (info.dbsel == "")
      info.dbsel = "none";

    info.suffix = ""
      + "?session=" + SESSION_ID
      + ";version=" + data.version
      + ";span=" + this.optionValue(this._optPeriodSpan)
      + ";end=" + data['end']
      + ";start=" + data['start']
      + ";qty=" + this.optionValue(this._optQuantity)
      + ";grouping=" + this.optionValue(this._optGrouping)
      + ";src-grouping=" + this.optionValue(this._optSrcGrouping)
      + ";links=" + this.optionValue(this._optLinks)
      + ";from=" + encodeURIComponent(this._optFilterFrom.value)
      + ";to=" + encodeURIComponent(this._optFilterTo.value)
  }

  return this;
}();

GUI.Plugin.CompPhEDExLinkImage = new function()
{
  var _self = this;
  var _imgPlot = null;
  var _imgLegend = null;
  var _optPlotSize = null;

  this.base = GUI.Plugin.CompPhEDExLinkBase;
  this.attach = function(gui)
  {
    var content = ""
      + "<img id='plot' width='800' height='600' style='margin:1em 0 0' /><br />"
      + "<img id='legend' width='800' style='margin:0 0 1em' />"
    var preopts = ""
      + "<tr id='opt-plot-size' valign='top'><td>Plot format</td><td>"
      + "<select style='font-size:100%' id='plot-size'>"
      + "<option value='1600 1200 72'>Extra large</option>"
      + "<option value='800 600 72' selected='selected'>Large</option>"
      + "<option value='600 450 60'>Medium</option>"
      + "<option value='400 300 42'>Small</option>"
      + "</select>"
      + "</tr>";
    var postopts = ""
      + "<tr id='opt-download' valign='top'><td>Download</td><td>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExLinkImage.download(this.textContent)'>PDF</a>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExLinkImage.download(this.textContent)'>EPS</a>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExLinkImage.download(this.textContent)'>SVG</a>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExLinkImage.download(this.textContent)'>JSON</a>"
      + "</tr>";

    this.base.attach(gui, content, preopts, postopts);

    _imgPlot       = $('plot');
    _imgLegend     = $('legend');
    _imgPlot.src   = ROOTPATH + "/static/blank.gif";
    _imgLegend.src = ROOTPATH + "/static/blank.gif";
    _optPlotSize   = $('plot-size');
    _optPlotSize.onchange = function() { _self.base.setOption('size', this.value); return false; }
  }

  this.detach = function()
  {
    _imgPlot = null;
    _imgLegend = null;
    _optPlotSize = null;
    this.base.detach();
  }

  this.download = function(format)
  {
    // FIXME: Plot and legend?
    window.location.href = _imgPlot.src + ";type=" + format.toLowerCase();
    return false;
  }

  this.update = function(data, image, info)
  {
    this.base.update(data, info);

    var w = data['size.width'];
    var h = data['size.height'];
    var dpi = data['size.dpi'];
    var whd = '800 600 72';
    if (w && h && dpi)
    {
      info.suffix += ';width=' + w + ";height=" + h + ";dpi=" + dpi;
      _imgLegend.width = w;
      _imgPlot.width = w;
      _imgPlot.height = h;
      whd = w + ' ' + h + ' ' + dpi;
    }

    this.base.selectOption(_optPlotSize, whd, 1);

    GUI.ImgLoader.load("plot", _imgPlot, ROOTPATH + "/plotfairy/phedex/"
		       + info.dbsel + "/" + image + ":plot" + info.suffix);
    GUI.ImgLoader.load("legend", _imgLegend, ROOTPATH + "/plotfairy/phedex/"
		       + info.dbsel + "/" + image + ":legend" + info.suffix);
  }

  return this;
}();

GUI.Plugin.CompPhEDExLinkRate = new function()
{
  this.base = GUI.Plugin.CompPhEDExLinkImage;
  this.attach = function(gui) { this.base.attach(gui); }
  this.detach = function() { this.base.detach(); }
  this.update = function(data) { this.base.update(data, "link-rate", {}); }
  return this;
}();

GUI.Plugin.CompPhEDExLinkBytes = new function()
{
  this.base = GUI.Plugin.CompPhEDExLinkImage;
  this.attach = function(gui) { this.base.attach(gui); }
  this.detach = function() { this.base.detach(); }
  this.update = function(data) { this.base.update(data, "link-bytes", {}); }
  return this;
}();

GUI.Plugin.CompPhEDExRateQuality = new function()
{
  this.base = GUI.Plugin.CompPhEDExLinkImage;
  this.attach = function(gui) { this.base.attach(gui); }
  this.detach = function() { this.base.detach(); }
  this.update = function(data) { this.base.update(data, "rate-vs-quality", {}); }
  return this;
}();

GUI.Plugin.CompPhEDExBlockLatency = new function()
{
  var _self = this;
  var _imgPlot = null;
  var _optPlotSize = null;
  var _optPlotDetail = null;
  var _optRangeUnit = null;
  var _optRangeLinLog = null;
  var _optRangeBins = null;
  var _optRangeYMin = null;
  var _optRangeYMax = null;

  this.base = GUI.Plugin.CompPhEDExLinkBase;
  this.attach = function(gui)
  {
    var content = ""
      + "<img id='plot' width='800' height='600' style='margin:1em 0 0' />"
    var preopts = ""
      + "<tr id='opt-plot-size' valign='top'><td>Plot format</td><td>"
      + "<select style='font-size:100%' id='plot-size'>"
      + "<option value='1600 1200 72'>Extra large</option>"
      + "<option value='800 600 72' selected='selected'>Large</option>"
      + "<option value='600 450 60'>Medium</option>"
      + "<option value='400 300 42'>Small</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-plot-detail' valign='top'><td>Detail</td><td>"
      + "<select style='font-size:100%' id='plot-detail'>"
      + "<option value='completed-block-latency-summary' selected='selected'>Block completion latency</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-range' valign='top'><td>Units</td><td>"
      + "<select style='font-size:100%' id='range-unit'>"
      + "<option value='time' selected='selected'>Time only</option>"
      + "<option value='time-vs-size'>Time vs. size</option>"
      + "</select> "
      + "<select style='font-size:100%' id='range-linlog'>"
      + "<option value='lin'>Linear</option>"
      + "<option value='log' selected='selected'>Logarithmic</option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-range-bins' valign='top'><td>Bins</td><td>"
      + "<input style='font-size:100%' id='range-bins' type='text' size='4' />"
      + "</tr>"
      + "<tr id='opt-range-minmax' valign='top'><td style='padding-bottom:2em'>Y limits</td><td>"
      + "<input style='font-size:100%' id='range-ymin' type='text' size='4' /> - "
      + "<input style='font-size:100%' id='range-ymax' type='text' size='4' />"
      + "</tr>";
    var postopts = ""
      + "<tr id='opt-download' valign='top'><td>Download</td><td>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExBlockLatency.download(this.textContent)'>PDF</a>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExBlockLatency.download(this.textContent)'>EPS</a>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExBlockLatency.download(this.textContent)'>SVG</a>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExBlockLatency.download(this.textContent)'>JSON</a>"
      + "</tr>";

    this.base.attach(gui, content, preopts, postopts);

    _imgPlot        = $('plot');
    _imgPlot.src    = ROOTPATH + "/static/blank.gif";
    _optPlotSize    = $('plot-size');
    _optPlotDetail  = $('plot-detail');
    _optRangeUnit   = $('range-unit');
    _optRangeLinLog = $('range-linlog');
    _optRangeBins   = $('range-bins');
    _optRangeYMin   = $('range-ymin');
    _optRangeYMax   = $('range-ymax');

    _optPlotSize.onchange    = function() { _self.base.setOption('size', this.value); return false; }
    _optPlotDetail.onchange  = function() { _self.base.setOption('detail', this.value); return false; }
    _optRangeUnit.onchange   = function() { _self.base.setOption('range-unit', this.value); return false; }
    _optRangeLinLog.onchange = function() { _self.base.setOption('range-linlog', this.value); return false; }
    _optRangeBins.onchange   = function() { _self.base.setOption('range-bins', this.value); return false; }
    _optRangeYMin.onchange   = function() { _self.base.setOption('range-ymin', this.value); return false; }
    _optRangeYMax.onchange   = function() { _self.base.setOption('range-ymax', this.value); return false; }

    $('opt-qty').style.display = 'none';
    $('opt-src-grouping').style.display = 'none';
    $('opt-from-filter').style.display = 'none';
    this.base._optQuantity.selectedIndex = 0;
    this.base._optSrcGrouping.selectedIndex = 0;
    this.base._optFilterFrom.value = '';
  }

  this.detach = function()
  {
    _imgPlot = null;
    _optPlotSize = null;
    _optPlotDetail = null;
    this.base.detach();
  }

  this.download = function(format)
  {
    // FIXME: Plot and legend?
    window.location.href = _imgPlot.src + ";type=" + format.toLowerCase();
    return false;
  }

  this.update = function(data)
  {
    var info = {};
    this.base.update(data, info);

    var w = data['size.width'];
    var h = data['size.height'];
    var dpi = data['size.dpi'];
    var whd = '800 600 72';
    if (w && h && dpi)
    {
      info.suffix += ';width=' + w + ";height=" + h + ";dpi=" + dpi;
      _imgPlot.width = w;
      _imgPlot.height = h;
      whd = w + ' ' + h + ' ' + dpi;
    }

    this.base.selectOption(_optPlotSize, whd, 1);
    this.base.selectOption(_optPlotDetail, data['detail'], 0);
    this.base.selectOption(_optRangeUnit, data['range.unit'], 0);
    this.base.selectOption(_optRangeLinLog, data['range.linlog'], 1);

    if (data['range.bins'] != null)
      _optRangeBins.value = data['range.bins'];
    if (data['range.ymin'] != null)
      _optRangeYMin.value = data['range.ymin'];
    if (data['range.ymax'] != null)
      _optRangeYMax.value = data['range.ymax'];

    info.suffix += ""
      + ";unit=" + this.base.optionValue(_optRangeUnit)
      + ";timerep=" + this.base.optionValue(_optRangeLinLog)
      + ";bins=" + encodeURIComponent(_optRangeBins.value)
      + ";ymin=" + encodeURIComponent(_optRangeYMin.value)
      + ";ymax=" + encodeURIComponent(_optRangeYMax.value);

    GUI.ImgLoader.load("plot", _imgPlot, ROOTPATH + "/plotfairy/phedex/"
		       + info.dbsel + "/" + _optPlotDetail.value
		       + ":plot" + info.suffix);
  }

  return this;
}();

GUI.Plugin.CompPhEDExRegionCrossRate = new function()
{
  var _self = this;
  var _header = null;
  var _body = null;
  var _data = null;

  this.base = GUI.Plugin.CompPhEDExLinkBase;
  this.attach = function(gui)
  {
    var content = ""
      + "<iframe id='phedex-data'"
      + " onload='GUI.Plugin.CompPhEDExRegionCrossRate.refill()'"
      + " style='visibility:hidden;position:fixed;top:-5px;left:-5px'"
      + " width='0' height='0'></iframe>"
      + "<table style='margin:1em 0;padding-right:1em;border-right:1px solid #ddd' border='0' cellpadding='3'>"
      + "<thead><tr id='phedex-table-header'></tr></thead>"
      + "<tbody id='phedex-table-body'></tbody>"
      + "</table>";
    var postopts = ""
      + "<tr valign='top'><td>Download</td><td>"
      + " <a href='#' onclick='return GUI.Plugin.CompPhEDExRegionCrossRate.download()'>JSON</a>"
      + "</tr>";

    this.base.attach(gui, content, "", postopts);
    $('link-options').parentNode.style.padding = '1.4em 0 0 0';
    $('link-options').style.marginLeft = '1em';

    _header = $('phedex-table-header');
    _body = $('phedex-table-body');
    _data = $('phedex-data');
  }

  this.detach = function()
  {
    _header = null;
    _body = null;
    _data = null;
    this.base.detach();
  }

  this.download = function()
  {
    window.location.href = _data.src;
    return false;
  }

  this.refill = function()
  {
    var body = "";
    var header = "<th>To \\ From</th>";
    var frame = (_data.contentDocument || document.frames[_data.id].document);
    var data = eval(frame.body.firstChild.innerHTML);
    for (var i = 0, e = data.regions.length; i < e; ++i)
    {
      header += "<th>" + data.regions[i] + "</th>";
      body += "<tr><th>" + data.regions[i] + "</th>";
      for (var j = 0; j < e; ++j)
	body += sprintf("<td align='right' bgcolor='%s'>%.1f</td>",
			(i == j ? "#ffc" : "#fff"),
			data.table[data.regions[i]][data.regions[j]])
      body += "</tr>";
    }

    if (_header.innerHTML != header)
      _header.innerHTML = header;
    if (_body.innerHTML != body)
      _body.innerHTML = body;
  }

  this.update = function(data)
  {
    var info = {};
    this.base.update(data, info);
    var frames = document.getElementsByTagName("iframe");
    for (var i = 0, e = frames.length; i < e; ++i)
      frames[i].src = ROOTPATH + "/plotfairy/phedex/" + info.dbsel + "/region-cross-rate:info" + info.suffix;
  }

  return this;
}();
