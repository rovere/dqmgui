var _IMG_BORDER_NONE     = "1px solid transparent";
var _IMG_BORDER_NEW      = "1px solid #4a4";
var _IMG_BORDER_ALARM    = "1px solid #a33";
var _IMG_BORDER_SELECTED = "1px solid #f57900";
var _IMG_BORDER_ALARM_SELECTED = "1px dashed #f57900";
var _AXES             = { "def": 0, "lin": 1, "log": 2 };
var _SIZES            =
[ { index: 0, title: "Tiny",  label: "XS", className: "size-xsmall", width: 67,  height: 50 },
  { index: 1, title: "Small", label: "S",  className: "size-small",  width: 133, height: 100 },
  { index: 2, title: "Medium",label: "M",  className: "size-medium", width: 266, height: 200 },
  { index: 3, title: "Large", label: "L",  className: "size-large",  width: 532, height: 400 },
  { index: 4, title: "Fill",  label: "XL", className: "size-xlarge", width: -1,  height: -1 } ];
var _SIZESARRAY       = _map(_SIZES, function(x) { return [x.title, x.label]; });
var _SIZEMAP	      = {};
_map(_SIZES, function(x) { _SIZEMAP[x.label] = x; });

// Resize an image to fit the canvas.
function setsize(canvas, img, size, row, rows, col, cols)
{
  var ww = (window.innerWidth || document.documentElement.clientWidth);
  var wh = (window.innerHeight || document.documentElement.clientHeight);
  var maxheight = wh - YAHOO.util.Dom.getXY(canvas)[1] - 50;
  var maxwidth = ww - 25;
  if (maxwidth/1.33 > maxheight)
    maxwidth = Math.floor(1.33 * maxheight);
  else
    maxheight = Math.floor(maxwidth / 1.33);

  var width = (size.width > 0 ? size.width : maxwidth);
  var height = (size.height > 0 ? size.height : maxheight);
  var w = Math.round((width+2-2*cols)/cols);
  var h = Math.round((height+2-2*rows)/rows);
  if (col == cols-1)
    w += (width+2) - (w+2)*cols;
  if (row == rows-1)
    h += (height+2) - (h+2)*rows;
  if (img.width != w)
    img.width = w;
  if (img.height != h)
    img.height = h;
  return ";w=" + w + ";h=" + h;
}

function setsizeclass(item, title, size, ref)
{
  var suffix = (ref == "on-side" ? " double" : "");
  if (item.className.indexOf("size-") < 0)
    item.className += " " + size.className + suffix;
  else
    item.className = item.className.replace(/size-[a-z]+( double)?/,
		                            size.className + suffix);

  var titlew = size.width * (suffix ? 2 : 1) + "px";
  if (size.width > 0 && title.style.width != titlew)
    title.style.width = titlew;
  else if (size.width < 0 && title.style.width != '')
    title.style.width = '';
}

function layoutimg(img, container, focus, onclick, ref, size, ob,
		   rowdiv, nrows, row, ncols, col, n, xargs)
{
  var sizeparam = setsize(container, img, size, row, nrows, col, ncols);
  var nameref = (ob.nukename
	         ? encodePathComponent(ob.location)
	         : encodePathComponent(ob.location) + "/"
		   + encodePathComponent(ob.name));
  var imgref = (! ob.name ? FULLROOTPATH + "/static/blank.gif?"
		: FULLROOTPATH + "/plotfairy/" + nameref + "?");
  var param = "session=" + SESSION_ID
	      + ";v=" + ob.version
	      + (ob.xaxis.type != "def" ? ";xtype=" + ob.xaxis.type : "")
	      + (ob.xaxis.min != null ? ";xmin=" + ob.xaxis.min : "")
	      + (ob.xaxis.max != null ? ";xmax=" + ob.xaxis.max : "")
	      + (ob.yaxis.type != "def" ? ";ytype=" + ob.yaxis.type : "")
	      + (ob.yaxis.min != null ? ";ymin=" + ob.yaxis.min : "")
	      + (ob.yaxis.max != null ? ";ymax=" + ob.yaxis.max : "")
	      + (ob.zaxis.type != "def" ? ";ztype=" + ob.zaxis.type : "")
	      + (ob.zaxis.min != null ? ";zmin=" + ob.zaxis.min : "")
	      + (ob.zaxis.max != null ? ";zmax=" + ob.zaxis.max : "")
	      + (ob.drawopts ? ";drawopts=" + ob.drawopts : "")
	      + (ob.withref != "def" ? ";withref=" + ob.withref : "")
	      + (ref != "object" ? ";ref=" + ref : "")
	      + xargs;
  var url = imgref + param + sizeparam;
  img.setAttribute('alarm', ob.alarm); // attach the alarm property directly to the HTML img tag
  var border = ((ob.alarm && (nrows > 1 || ncols > 1))
                ? (focus && ob.name == focus ? _IMG_BORDER_ALARM_SELECTED :_IMG_BORDER_ALARM)
		: (focus && ob.name == focus ? _IMG_BORDER_SELECTED : _IMG_BORDER_NONE));

  if (! ob.name && img.src != url)
    img.src = url;
  else if (img.src != url)
  {
    img.dqmsrc = url;
    GUI.ImgLoader.load(n+"."+row+"."+col, img, url, border, _IMG_BORDER_NEW);
    if (img.src.replace(/\?.*/, "?") != imgref)
      img.src = ROOTPATH + "/static/blank.gif";
    border = img.style.border;
  }

  if (img.style.border != border)
    img.style.border = border;

  var title = ob.title || ob.name;
  if (img.title != title)
      img.title = title;

  if (img.getAttribute('dqmhelp') != ob.desc)
    img.setAttribute('dqmhelp', ob.desc);

  if (! img.onclick && onclick)
    img.onclick = onclick;

  if (! img.ondblclick && onclick)
    img.ondblclick = onclick;

  if (ob.name == focus && onclick)
    onclick({ target: img });
}

// Layout one canvas item.
function layout(type, container, item, obj, sz, ref, strip, focus, onclick, xstyle, n)
{
  var size = _SIZEMAP[sz];
  var title;

  // Insert new object if one didn't exist previously.
  if (! item)
  {
    item = document.createElement(type);
    title = document.createElement("h3");
    item.appendChild(title);
    container.appendChild(item);
  }

  // Otherwise overwrite the existing one.
  else
  {
    title = item.firstChild;
  }

  // If it's a directory, show in compact form: title only.  The
  // server always gives directories first, only then plots.
  if (obj.dir)
  {
    var style = "item folder" + xstyle + (obj.dir.nalarm ? " alarm" : "");
    var ref = "<a href='#' onclick='return GUI.Plugin.DQMCanvas.rebase(this)' dqmto='"
	      + obj.name + "'><img src='" + ROOTPATH + "/static/folder.png'"
	      + " width='16' height='16' /> " + obj.name.replace(/.*\//, "")
	      + "</a>";
    _settitle(item, style, obj.name);
    _setvaltitle(title, "", ref,
		 obj.dir.nleaves + " object" + (obj.dir.nleaves > 1 ? "s" : "")
		 + ", " + obj.dir.nlive + " live, "
		 + (obj.dir.nalarm > 1 ? obj.dir.nalarm + " alarms"
		    : (obj.dir.nalarm ? "1 alarm" : "no alarms")));

    while (item.childNodes.length > 1)
      item.removeChild(item.lastChild);

    setsizeclass(item, title, size, ref);
    return item;
  }

  // Now update the item; it's a plot.
  var alarm = 0;
  var dead = 0;
  for (var row = 0, nrows = obj.items.length; row < nrows; ++row)
    for (var col = 0, ncols = obj.items[row].length; col < ncols; ++col)
    {
      var ob = obj.items[row][col];
      alarm += ob.alarm;
      dead += ! ob.live;
    }

  var prevfolder = (item.previousSibling
		    && item.previousSibling
		    .className.indexOf("folder") >= 0);

  var style = ("item"
	       + xstyle
	       + (alarm ? " alarm" : "")
	       + (dead ? " dead" : "")
	       + (prevfolder ? " break" : ""));
  _settitle(item, style, obj.name);
  _setval(title, "", obj.name.replace(/.*\//, ""));

  // Make sure layout in place conforms to the new structure.
  // Reinitialise and remove only where necessary, and reuse
  // old objects otherwise.  Give the image a URL to the
  // server call that generates the image, but don't touch
  // the URL unnecessarily.
  var nrows = obj.items.length;
  var dostrip = (strip.type != "object"
	         && strip.n
		 && parseInt(strip.n) > 0);

  for (var row = 0; row < nrows; ++row)
  {
    var rowdiv = (row < item.childNodes.length - 1
		  ? item.childNodes[row+1]
		  : document.createElement("div"));
    var rowparent = rowdiv.parentNode;
    var ncols = obj.items[row].length;
    var maxcols = ncols;

    if (! rowparent)
      item.appendChild(rowdiv);

    if (rowdiv.className != "row")
      rowdiv.className = "row";

    for (var col = 0; col < ncols; ++col)
    {
      var showref = "object";
      var ob = obj.items[row][col];
      var img = (col < rowdiv.childNodes.length
	         ? rowdiv.childNodes[col]
	         : document.createElement("img"));
      if (! img.parentNode)
        rowdiv.appendChild(img);

      var overlay = false;
      var xargs = "";
      if (ref.position == "overlay"
	  && (ref.show == "all"
	      || (ref.show == "customise" && ob.withref == "yes")))
      {
        for (var refidx = 0, refcnt = 0; refidx < ref.param.length; ++refidx)
        {
          var refdef = ref.param[refidx];
	  if (refdef.dataset && ! refdef.dataset.match(/^(\/[-A-Za-z0-9_]+){3}$/))
	    refdef.dataset = "";
	  if (refdef.type == "none"
	      || (refdef.type == "other"
		  && ! refdef.run
		  && ! refdef.dataset))
	    continue;

	  if (refdef.type == "refobj")
	     overlay = true;
	  else
	  {
	    var parts = ob.location.split("/");
	    var dataset = "/" + parts.splice(2, 3).join("/");
	    if (parts[0] == "live" || parts[0] == "unknown")
	      parts[0] = "archive";
	    if (refdef.run)
	      parts[1] = refdef.run;
	    if (refdef.dataset)
	      dataset = refdef.dataset;

	    xargs += ";obj=" + encodeURIComponent
	      (parts.join("/") + dataset + "/" + ob.name);
	    if (refdef.ktest)
	      xargs += ';ktest='+refdef.ktest;
          }
	}
      }

      var showref = (overlay ? "overlay" : "object");
      if (! dostrip && xargs)
      {
	ob = _clone(ob);
	xargs = ";obj=" + encodeURIComponent(ob.location+"/"+ob.name) + xargs;
	ob.nukename = 1;
	ob.location = "overlay";
      }
      else if (dostrip)
      {
        ob = _clone(ob);
	xargs = ";trend=" + encodeURIComponent(strip.type)
		+ ";n=" + encodeURIComponent(strip.n)
		+ ";omit=" + encodeURIComponent(strip.omit)
		+ ";axis=" + encodeURIComponent(strip.axis)
		+ ";current=" + encodeURIComponent(ob.location);
	ob.location = "stripchart/" + ob.name;
	ob.xaxis = ob.yaxis = ob.zaxis = {type: "def", min: null, max: null};
	ob.drawopts = "";
	ob.withref = "def";
	ob.nukename = 1;
	showref = "object";
      }

      layoutimg(img, container, focus, onclick, showref, size, ob,
		rowdiv, nrows, row, ncols, col, n, xargs);
    }

    if (! dostrip && ref.position == "on-side" && ref.show != "none")
    {
      for (var refidx = 0, refcnt = 0; refidx < ref.param.length; ++refidx)
      {
        var refdef = ref.param[refidx];
	if (refdef.dataset && ! refdef.dataset.match(/^(\/[-A-Za-z0-9_]+){3}$/))
	  refdef.dataset = "";
	if (refdef.type == "none"
	    || (refdef.type == "other"
		&& ! refdef.run
		&& ! refdef.dataset))
	  continue;

	refcnt++;
        maxcols += ncols;
        for (var col = 0; col < ncols; ++col)
        {
	  var showref;
          var realcol = ncols * refcnt + col;
          var ob = _clone(obj.items[row][col]);
          var img = (realcol < rowdiv.childNodes.length
	             ? rowdiv.childNodes[realcol]
	             : document.createElement("img"));
          if (! img.parentNode)
            rowdiv.appendChild(img);

	  ob.alarm = 0;
	  if (ref.show == "customise" && ob.withref != "yes")
          {
	    // customise, side-by-side, without reference = blank
	    showref = "object";
	    ob.name = null;
	  }
	  else
          {
	    // All or customise with reference = select object to show.
	    // Somewhat tricky and hackish as we need to munge location
	    // path based on what sort of a reference was chosen for
	    // display. We only support switching to same object in
	    // another (archived) run/dataset of the same kind. For more
	    // flexibility the core code would have to turn reference
	    // selection to full blown sample selection.

	    if (refdef.type == "refobj")
	    {
	      showref = "reference";
	      ob.title = "Ref #" + (refidx+1) + " Standard; " + ob.name;
	    }
	    else
	    {
	      var parts = ob.location.split("/");
	      var dataset = parts.splice(2, 3).join("/");
	      if (parts[0] == "live")
		parts[0] = "archive";
	      if (refdef.run)
		parts[1] = refdef.run;
	      if (refdef.dataset)
		dataset = refdef.dataset;

	      ob.location = parts.join("/") + "/" + dataset;
	      ob.title = sprintf("Ref #%d, run %s, dataset %s; %s",
				refidx+1, parts[1], dataset, ob.name);
	      showref = "object";
	    }
          }
          layoutimg(img, container, focus, null /* omit onclick for ref */,
		    showref, size, ob, rowdiv, nrows, row, ncols, col,
		    "ref_" + refcnt + "_" + n, "");
        }
      }
    }

    while (rowdiv.childNodes.length > maxcols)
      rowdiv.removeChild (rowdiv.lastChild);
  }

  while (item.childNodes.length > nrows+1)
    item.removeChild (item.lastChild);

  setsizeclass(item, title, size, ref);
  return item;
}

GUI.Plugin.DQMCanvas = new function()
{
  Ext.QuickTips.init();

  var _HELP_IDLE_MSG	= "(Click on a histogram for details)";
  var _SLIDER_UNIT	= 20;
  var _size		= "M";

  var _gui		= null;
  var _self		= this;
  var _canvas		= $('canvas');
  var _optsarea		= $('canvas-opts');

  var _helpgroup	= $('canvas-help-group');
  var _helparea		= $('canvas-help');
  var _helppanel	= null;
  var _helplocal	= false;

  var _customgroup      = $('canvas-custom-group');
  var _customarea       = $('canvas-custom');
  var _custompanel      = null;
  var _customlocal	= false;
  var _focus		= null;
  var _focusURL         = '';

  var _linkMe           = null;
  var _zoomWin          = null;
  var _zoomlocal        = false;
  var _rootPath         = null;

  var _imgXType		= null;
  var _imgXMin		= null;
  var _imgXMax		= null;
  var _imgYType		= null;
  var _imgYMin		= null;
  var _imgYMax		= null;
  var _imgZType		= null;
  var _imgZMin		= null;
  var _imgZMax		= null;
  var _imgDrawOpts	= null;
  var _imgWithRef	= null;

  var _shown		= null;
  var _showzoom         = null;
  var _layoutroot	= null;
  var _click		= { event: null, timeout: null, timeClick: 0, timeDoubleClick: 0 };

  this.onresize = function()
  {
    var h = _optsarea.clientHeight;
    var top = _canvas.offsetTop;
    var newtop = h+2;
    var size = _SIZEMAP["XL"];
    for (var i = 0, obj = _canvas.firstChild; obj; ++i, obj = obj.nextSibling)
      if (obj.className.indexOf("size-xlarge") >= 0)
      {
        var nrows = obj.childNodes.length-1;
        for (var row = 0; row < nrows; ++row)
        {
	  var ncols = obj.childNodes[row+1].childNodes.length;
	  for (var col = 0; col < ncols; ++col)
	  {
	    var img = obj.childNodes[row+1].childNodes[col];
	    var suffix = setsize(_canvas, img, size, row, nrows, col, ncols);
	    var url = img.src.replace(/;w=.*/, suffix);
	    if (img.src != url)
	    {
	      img.dqmsrc = url;
	      GUI.ImgLoader.load("div"+i+"."+row+"."+col, img, url);
	    }
	  }
        }
      }
  };

  this.dofocus = function(e)
  {
    // Set help message.
    e = e || window.event;
    var hit = (e.srcElement || e.target);
    var help = hit.getAttribute('dqmhelp');
    var titleabbrev = _sanitise(hit.title ? " " + hit.title.replace(/.*\//, "") : "");
    if (help)
    {
      help = help.replace(/<\s*a\s+/, '<a target="helpWindow "');
      _helparea.innerHTML
	= "<div><span style='color:#555;margin:0 0 5px'>"
	+ titleabbrev + ": </span>" + help + "</div>";
    }
    else if (hit.title)
      _helparea.innerHTML
	= "<div><span style='color:#888'>(No description available for "
	+ titleabbrev + ".)</span></div>";
    else
      _helparea.innerHTML = _HELP_IDLE_MSG;

    // Update customisation panel.
    if (hit.dqmsrc)
    {
      var url   = hit.dqmsrc;
      var opts  = url.indexOf("drawopts=") >= 0 ? url.replace(/.*drawopts=([^;]*).*/, "$1") : "";
      var ref   = url.indexOf("withref=") >= 0  ? url.replace(/.*withref=([^;]*).*/, "$1")  : "";
      var xtype = url.indexOf("xtype=") >= 0    ? url.replace(/.*xtype=([^;]*).*/, "$1")    : "";
      var xmin  = url.indexOf("xmin=") >= 0     ? url.replace(/.*xmin=([^;]+).*/, "$1")     : "";
      var xmax  = url.indexOf("xmax=") >= 0     ? url.replace(/.*xmax=([^;]+).*/, "$1")     : "";
      var ytype = url.indexOf("ytype=") >= 0    ? url.replace(/.*ytype=([^;]*).*/, "$1")    : "";
      var ymin  = url.indexOf("ymin=") >= 0     ? url.replace(/.*ymin=([^;]+).*/, "$1")     : "";
      var ymax  = url.indexOf("ymax=") >= 0     ? url.replace(/.*ymax=([^;]+).*/, "$1")     : "";
      var ztype = url.indexOf("ztype=") >= 0    ? url.replace(/.*ztype=([^;]*).*/, "$1")    : "";
      var zmin  = url.indexOf("zmin=") >= 0     ? url.replace(/.*zmin=([^;]+).*/, "$1")     : "";
      var zmax  = url.indexOf("zmax=") >= 0     ? url.replace(/.*zmax=([^;]+).*/, "$1")     : "";

      if (ref == "def" || ref == "")
	ref = 0;
      else if (ref == "yes")
	ref = 1;
      else if (ref == "no")
	ref = 2;

      if (_imgDrawOpts.value != opts)
	_imgDrawOpts.value = opts;

      if (_imgWithRef.selectedIndex != ref)
	_imgWithRef.selectedIndex = ref;

      if (_imgXType.selectedIndex != _AXES[xtype])
	_imgXType.selectedIndex = _AXES[xtype];
      if (_imgXMin.value != xmin)
	_imgXMin.value = xmin;
      if (_imgXMax.value != xmax)
	_imgXMax.value = xmax;

      if (_imgYType.selectedIndex != _AXES[ytype])
	_imgYType.selectedIndex = _AXES[ytype];
      if (_imgYMin.value != ymin)
	_imgYMin.value = ymin;
      if (_imgYMax.value != ymax)
	_imgYMax.value = ymax;

      if (_imgZType.selectedIndex != _AXES[ztype])
	_imgZType.selectedIndex = _AXES[ztype];
      if (_imgZMin.value != zmin)
	_imgZMin.value = zmin;
      if (_imgZMax.value != zmax)
	_imgZMax.value = zmax;

      _imgDrawOpts.disabled = false;
      _imgWithRef.disabled = false;
      _imgXType.disabled = false;
      _imgXMin.disabled = false;
      _imgXMax.disabled = false;
      _imgYType.disabled = false;
      _imgYMin.disabled = false;
      _imgYMax.disabled = false;
      _imgZType.disabled = false;
      _imgZMin.disabled = false;
      _imgZMax.disabled = false;

      _map(_customarea.getElementsByTagName("LABEL"), function(node) {
	if (node.className.indexOf("disabled") >= 0)
	  node.className = node.className.replace(/ *disabled/g, "");
      });

      _map(_canvas.childNodes, function(node) {
        var thishit = (node == hit.parentNode.parentNode);
	if (thishit && node.className.indexOf("picked") < 0)
	  node.className += " picked";
	else if (! thishit && node.className.indexOf("picked") >= 0)
	  node.className = node.className.replace(/ *picked/g, "");
      });

      _map(_canvas.getElementsByTagName('img'), function(node) {
        var thishit = (node == hit);
        var border = '';
        if (thishit)
          border += node.getAttribute('alarm')=='1' ? _IMG_BORDER_ALARM_SELECTED : _IMG_BORDER_SELECTED;
        else
          border += node.getAttribute('alarm')=='1' ? _IMG_BORDER_ALARM : _IMG_BORDER_NONE;
        node.style.border = border;
      });

      _custompanel.setHeader("Customise" + titleabbrev);
      _custompanel.render();
      _focusURL = url;
    }
    else
    {
      _imgDrawOpts.disabled = true;
      _imgWithRef.disabled = true;
      _imgXType.disabled = true;
      _imgXMin.disabled = true;
      _imgXMax.disabled = true;
      _imgYType.disabled = true;
      _imgYMin.disabled = true;
      _imgYMax.disabled = true;
      _imgZType.disabled = true;
      _imgZMin.disabled = true;
      _imgZMax.disabled = true;

      _map(_customarea.getElementsByTagName("LABEL"), function(node) {
	if (node.className.indexOf("disabled") < 0)
	  node.className += " disabled";
      });

      _map(_canvas.childNodes, function(node) {
	if (node.className.indexOf("picked") >= 0)
	  node.className = node.className.replace(/ *picked/g, "");
      });

      _custompanel.setHeader("Customise");
      _custompanel.render();
      _focusURL = '';
    }

    _focus = hit.title || null;

    /* Update the content of the zoomWindow: this is mandatory to be always
       in sync with respect to whatever changes the end user made using
       the custom panel. This piece of code is called regardless of how
       we call this function, i.e. by click on a histograms or by the
       method update. */
    _self.updateZoomPlot();

    // Update LinkMe window, in case there is one
    if (_linkMe)
      if (_linkMe.isVisible())
	this.setLinkMe();

    return hit;
  };

  this.focus = function(e)
  {
    var hit = _self.dofocus(e);
    /* Update server if this was user-originated focus event.  Don't call back
       if we are setting focus internally, e.g. through server update. */
    if (e.type == 'click')
      _gui.asyncCall(_url() + "/setFocus" + (hit.title ? "?n=" + encodeURIComponent(hit.title) : "") );
    else if (e.type == 'dblclick')
      _gui.makeCall(_url() + "/setZoom?show=yes");

    return false;
  };


  this.setLinkMe = function()
  {
    var myattrs = _self.linkMe();
    var hdrattrs = _gui.Plugin.DQMHeaderRow.linkMe();
    var url = _url().replace(/session.*/, '') + 'start?';

    for (prop in hdrattrs)
      url += prop + '=' + encodePathComponent(hdrattrs[prop]) + ';';
    for (prop in myattrs)
      url += prop + '=' + encodePathComponent(myattrs[prop]) + ';';

    _linkMe = Ext.MessageBox.show({ title: 'Cut and Paste this link:', msg: '',
				    width: 400, prompt: true, value: url,
				    modal: false });
  };

  this.attach = function(gui)
  {
    _gui = gui;
    _optsarea.style.display = '';

    var linkmeAction = new Ext.Action({ text: 'Link-Me',
					handler: _self.setLinkMe,
					iconCls: 'linkme',
					scope:    _self,
					id: 'canvas-linkMe' });

    var tb = new Ext.Toolbar({ id : 'canvas-submenu' });
    var store = new Ext.data.ArrayStore({ fields: ['label', 'abbr'],
					  data : _SIZESARRAY });
    var combo = new Ext.form.ComboBox({ id: 'canvas-size',
					width: 80,
					store: store,
					displayField:'label',
					valueField:'abbr',
					typeAhead: true,
					mode: 'local',
					forceSelection: true,
					triggerAction: 'all' });
    combo.on('change', function(obj, value, oldvalue) {
      if (_size != value)
      {
	_size = value;
	_self.resize();
	_gui.makeCall(_url() + "/setSize?sz=" + value);
      }
    });

    combo.on('collapse', function(obj) {
      var value = obj.getValue();
      if (_size != value)
      {
	_size = value;
	_self.resize();
	_gui.makeCall(_url() + "/setSize?sz=" + value);
      }
    });

    tb.add({ text          : 'Size:' ,
	     xtype         : 'tbtext' },
	   combo, '-' ,
	   { text          : 'Play',
	     enableToggle  : true,
	     toggleHandler : function() { _gui.makeCall(_url() + "/play"); return false; },
	     pressed       : false
	   }, '-',
	   { text          : 'Reset Workspace',
	     enableToggle  : false,
	     handler       : function() {
	       _customlocal = _helplocal = _zoomlocal = false;
	       _gui.makeCall(_url() + "/resetWorkspace");
	       return false;
	     }
	   },'-',
	   { text          : 'Describe',
	     id            : 'canvas-describe',
	     enableToggle  : true,
	     pressed       : false,
	     toggleHandler : function() {
	       if (_helppanel)
		 _helppanel.cfg.setProperty('visible', !_helppanel.cfg.getProperty('visible'));
	       else
		 _helppanel.show();
	       return false; }
	   }, '-',
	   { text          : 'Customise',
	     id            : 'canvas-customize',
	     enableToggle  : true,
	     pressed       : false,
	     toggleHandler : function() {
	       if (_custompanel)
		 _custompanel.cfg.setProperty('visible', !_custompanel.cfg.getProperty('visible'));
	       else
		 _custompanel.show();
	       return false; }
	   } , '-',
	   { id            : 'canvas-layouts',
	     scope         : this,
	     text          : 'Layouts',
	     enableToggle  : false
	   }, '-',
	   { id            : 'canvas-dqmroot',
	     text          : '(Top)',
	     xtype         : 'tbtext'
	   }, '->',
	   linkmeAction
	  );

    tb.render('canvas-opts');
    tb.doLayout();
    Ext.getCmp('canvas-layouts').on('click', function(btn, event) {
      if (_layoutroot != "")
	  _gui.makeCall(_url() + "/setRoot?n=" + encodeURIComponent(_layoutroot));
      return false; }, this);

    _customarea.innerHTML =
      "<div class='group'><label for='m-obj-x-type'>X:</label>"
      + "<select id='m-obj-x-type'>"
      + "<option value='def'>Default</option><option value='lin'>Linear</option><option value='log'>Log</option>"
      + "</select><label for='m-obj-x-min'>Min</label>"
      + "<input id='m-obj-x-min' size='4' type='text' />"
      + "<label>...</label>"
      + "<input id='m-obj-x-max' size='4' type='text' />"
      + "<label for='m-obj-x-max'>Max</label></div>"

      + "<div class='group'><label for='m-obj-y-type'>Y:</label>"
      + "<select id='m-obj-y-type'>"
      + "<option value='def'>Default</option><option value='lin'>Linear</option><option value='log'>Log</option>"
      + "</select><label for='m-obj-y-min'>Min</label>"
      + "<input id='m-obj-y-min' size='4' type='text' />"
      + "<label>...</label>"
      + "<input id='m-obj-y-max' size='4' type='text' />"
      + "<label for='m-obj-y-max'>Max</label></div>"

      + "<div class='group'><label for='m-obj-z-type'>Z:</label>"
      + "<select id='m-obj-z-type'>"
      + "<option value='def'>Default</option><option value='lin'>Linear</option><option value='log'>Log</option>"
      + "</select><label for='m-obj-z-min'>Min</label>"
      + "<input id='m-obj-z-min' size='4' type='text' />"
      + "<label>...</label>"
      + "<input id='m-obj-z-max' size='4' type='text' />"
      + "<label for='m-obj-z-max'>Max</label></div>"

      + "<div class='group'><label for='m-obj-draw-opts' class='grid'>Draw options:</label>"
      + "<input id='m-obj-draw-opts' size='8' type='text' /></div>"
      + "<div class='group'><label for='m-obj-with-ref' class='grid'>Reference:</label>"
      + "<select id='m-obj-with-ref'>"
      + "<option value='def'>Default</option><option value='yes'>Yes</option><option value='no'>No</option>"
      + "</select></div>";

    _imgXType    = $('m-obj-x-type');
    _imgXMin     = $('m-obj-x-min');
    _imgXMax     = $('m-obj-x-max');
    _imgYType    = $('m-obj-y-type');
    _imgYMin     = $('m-obj-y-min');
    _imgYMax     = $('m-obj-y-max');
    _imgZType    = $('m-obj-z-type');
    _imgZMin     = $('m-obj-z-min');
    _imgZMax     = $('m-obj-z-max');
    _imgDrawOpts = $('m-obj-draw-opts');
    _imgWithRef  = $('m-obj-with-ref');

    _imgXType.onchange    = function() { _self.alterImage("xtype",    this.value); return false; };
    _imgXMin.onchange     = function() { _self.alterImage("xmin",     this.value); return false; };
    _imgXMax.onchange     = function() { _self.alterImage("xmax",     this.value); return false; };
    _imgYType.onchange    = function() { _self.alterImage("ytype",    this.value); return false; };
    _imgYMin.onchange     = function() { _self.alterImage("ymin",     this.value); return false; };
    _imgYMax.onchange     = function() { _self.alterImage("ymax",     this.value); return false; };
    _imgZType.onchange    = function() { _self.alterImage("ztype",    this.value); return false; };
    _imgZMin.onchange     = function() { _self.alterImage("zmin",     this.value); return false; };
    _imgZMax.onchange     = function() { _self.alterImage("zmax",     this.value); return false; };
    _imgDrawOpts.onchange = function() { _self.alterImage("drawopts", this.value); return false; };
    _imgWithRef.onchange  = function() { _self.alterImage("withref",  this.value); return false; };

    this.setSizeUnit();

    var ww = (window.innerWidth || document.documentElement.clientWidth);
    var wh = (window.innerHeight || document.documentElement.clientHeight);
    _helpgroup.style.display = '';
    _helppanel = new YAHOO.widget.Panel(_helpgroup, {
      visible: true, draggable: true, close: false, zindex: 2,
      x: parseInt(0.75 * ww) - 25,
      y: YAHOO.util.Dom.getXY($('cmslogo'))[1],
      width: parseInt(0.25 * ww) + "px",
      constraintoviewport: true, autofillheight: "body" });
    var resize = new YAHOO.util.Resize(_helpgroup, {
      handles: ["br"], autoRatio: false, minWidth: 200,
      minHeight: 50, status: false });
    resize.on("startResize", function(args) {
      var O = YAHOO.widget.Overlay;
      var D = YAHOO.util.Dom;
      var clreg = D.getClientRegion();
      var elreg = D.getRegion(this.element);
      resize.set("maxWidth", clreg.right - elreg.left - O.VIEWPORT_OFFSET);
      resize.set("maxHeight", clreg.bottom - elreg.top - O.VIEWPORT_OFFSET);
    }, _helppanel, true);
    resize.on("resize", function(args) {
      this.cfg.setProperty("height", args.height + "px");
    }, _helppanel, true);
    _helppanel.render();
    _helppanel.show();
    _helppanel.moveEvent.subscribe(function(type, args) {
      _helplocal = true;
      _gui.asyncCall(_url() + "/panel?n=help;show=yes;x="
		     + args[0][0] + ";y=" + args[0][1]);
    });
    _helppanel.hideEvent.subscribe(function(type, args) {
      _helplocal = true;
      _gui.asyncCall(_url() + "/panel?n=help;show=no");
    });
    _helppanel.showEvent.subscribe(function(type, args) {
      _helplocal = true;
      _gui.asyncCall(_url() + "/panel?n=help;show=yes");
    });

    _customgroup.style.display = '';
    _custompanel = new YAHOO.widget.Panel(_customgroup, {
      visible: true, draggable: true, close: false, zindex: 3,
      x: parseInt(0.75 * ww) - 25,
      y: Math.max(wh - 100, 0),
      width: parseInt(0.25 * ww) + "px",
      constraintoviewport: true, autofillheight: "body" });
    resize = new YAHOO.util.Resize(_customgroup, {
      handles: ["br"], autoRatio: false, minWidth: 200,
      minHeight: 50, status: false });
    resize.on("startResize", function(args) {
      var O = YAHOO.widget.Overlay;
      var D = YAHOO.util.Dom;
      var clreg = D.getClientRegion();
      var elreg = D.getRegion(this.element);
      resize.set("maxWidth", clreg.right - elreg.left - O.VIEWPORT_OFFSET);
      resize.set("maxHeight", clreg.bottom - elreg.top - O.VIEWPORT_OFFSET);
    }, _custompanel, true);
    resize.on("resize", function(args) {
      this.cfg.setProperty("height", args.height + "px");
    }, _custompanel, true);
    _custompanel.render();
    _custompanel.hide();
    _custompanel.moveEvent.subscribe(function(type, args) {
      _customlocal = true;
      _gui.asyncCall(_url() + "/panel?n=custom;show=yes;x="
		     + args[0][0] + ";y=" + args[0][1]);
    });
    _custompanel.hideEvent.subscribe(function(type, args) {
      _customlocal = true;
      _gui.asyncCall(_url() + "/panel?n=custom;show=no");
    });
    _custompanel.showEvent.subscribe(function(type, args) {
      _customlocal = true;
      _gui.asyncCall(_url() + "/panel?n=custom;show=yes");
    });

    this.drawZoomInAttach();
    this.dofocus({ target: _canvas });
  };

  this.setSizeUnit = function()
  {
    Ext.getCmp('canvas-size').setValue(_size);
  };

  this.detach = function()
  {
    $('canvas-group').replaceChild(_helpgroup, _helpgroup.parentNode);
    while (_helpgroup.childNodes.length > 3)
      _helpgroup.removeChild(_helpgroup.lastChild);

    $('canvas-group').replaceChild(_customgroup, _customgroup.parentNode);
    while (_customgroup.childNodes.length > 3)
      _customgroup.removeChild(_customgroup.lastChild);

    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);

    _helppanel = null;
    _custompanel = null;
    _helplocal = false;
    _customlocal = false;
    _optsarea.innerHTML = "";
    _optsarea.style.display = 'none';
    _helpgroup.style.display = 'none';
    _customgroup.style.display = 'none';
    if (_zoomWin)
    {
      _zoomWin.suspendEvents();
      _zoomWin.destroy();
      _zoomWin = null;
    }
    if (_linkMe)
      if (_linkMe.isVisible())
	_linkMe.hide();
  };

  this.alterImage = function(param, value)
  {
    _gui.makeCall(_url() + "/alterImage?"
		  + "o=" + encodeURIComponent(_focus)
		  + ";opt=" + encodeURIComponent(param)
		  + ";value=" + encodeURIComponent(value));
    return false;
  };

  this.rebase = function(link)
  {
    var obj = link.getAttribute('dqmto');
    _gui.makeCall(_url() + "/setRoot?n=" + encodeURIComponent(obj));
    return false;
  };

  this.resize = function()
  {
    var size = _SIZEMAP[_size];
    for (var i = 0; i < _canvas.childNodes.length; ++i)
    {
      var obj = _canvas.childNodes[i];
      var title = obj.childNodes[0];
      var titlew = size.width + "px";
      if (size.width > 0 && title.style.width != titlew)
	title.style.width = titlew;
      else if (size < 0 && title.style.width != '')
	title.style.width = '';

      var nrows = obj.childNodes.length-1;
      for (var row = 0; row < nrows; ++row)
      {
	var ncols = obj.childNodes[row+1].childNodes.length;
	for (var col = 0; col < ncols; ++col)
	{
	  var img = obj.childNodes[row+1].childNodes[col];
	  var suffix = setsize(_canvas, img, size, row, nrows, col, ncols);
	  var url = img.src.replace(/;w=.*/, suffix);
	  if (img.src != url)
	  {
	    img.dqmsrc = url;
	    GUI.ImgLoader.load("div"+i+"."+row+"."+col, img, url);
	  }
	}
      }

      var newClass = obj.className.replace(/size-[a-z]+/, size.className);
      if (obj.className != newClass)
	obj.className = newClass;
    }
  };

  this.update = function(data)
  {
    var oldfocus = _focus;
    _layoutroot = data.layoutroot;
    _shown = data.items;
    _showzoom = data.zoom;
    _size = data.size;
    _focus = null;
    _focusURL = '';
    if (_linkMe)
    {
      if (_linkMe.isVisible())
	this.setLinkMe();
    }

    // Update canvas position taking into account header dynamic size.
    $('canvas-group').style.top = $('header').offsetHeight + 'px';

    // Run a loop to handle new contents.  We modify the contents
    // of "_canvas" in place in order to avoid unpleasant flicker
    // and changes in scroll position.  Fill the canvas with the
    // shown objects.
    var item = null;

    for (var n = 0, item = _canvas.firstChild; n < _shown.length;
	 ++n, item = item.nextSibling)
      item = layout("div", _canvas, item, _shown[n], _size,
		    data.reference, data.strip, data.focus,
		    this.focus, "", "div" + n);

    // Remove whatever is left of old contents.
    while (item)
    {
      var next = item.nextSibling;
      _canvas.removeChild(item);
      item = next;
    }

    // Update root links.
    var root = data.root;
    _rootPath = data.root;
    var rebase = "return GUI.Plugin.DQMCanvas.rebase(this)";
    var rootlink = "<a href='#' onclick='" + rebase + "' dqmto=''>(Top)</a>";
    if (root != "")
    {
      var from = 0;
      while (from < root.length)
      {
	var to = root.indexOf("/", from);
	if (to == -1) to = root.length;

	rootlink += " / ";
	if (to < root.length)
	  rootlink += "<a href='#' onclick='" + rebase + "' dqmto='"
		      + root.substr(0, to) + "'>"
		      + root.substr(from, to-from) + "</a>";
	else
	  rootlink += root.substr(from, to-from);

	from = to+1;
      }
    }

    if (Ext.getCmp('canvas-dqmroot').text != rootlink)
      Ext.getCmp('canvas-dqmroot').setText(rootlink);

    // Make sure help and custom panels are visible as appropriate.
    // In general once the user has interacted with the panels we rely
    // on state local to browser for the panel visibility and position.
    this.updatePanel(_helplocal, _helppanel, data.help);
    Ext.getCmp('canvas-describe').toggle(data.help.show, true);
    this.updatePanel(_customlocal, _custompanel, data.custom);
    Ext.getCmp('canvas-customize').toggle(data.custom.show, true);

    // If we are focused but there is no such object in this update,
    // reset focus entirely.  Also reset panel if we have no focus.
    this.drawZoom(data.zoom);
    if ((data.focus && ! _focus) || (! data.focus && oldfocus))
      this.dofocus({ target: _canvas });

    // Mark layout root enabled / disabled.
    if (_layoutroot == "")
      Ext.getCmp('canvas-layouts').disable();
    else
      Ext.getCmp('canvas-layouts').enable();

    // Update image options.
    this.setSizeUnit();
    this.onresize();
  };

  this.updatePanel = function(local, panel, data)
  {
    if (! local)
    {
      var show = Boolean(data.show);
      var x = data.x;
      var y = data.y;

      if (x >= 0 && panel.cfg.getProperty("x") != x)
	panel.cfg.setProperty("x", x);

      if (y >= 0 && panel.cfg.getProperty("y") != y)
	panel.cfg.setProperty("y", y);

      if (panel.cfg.getProperty("visible") != show)
	panel.cfg.setProperty("visible", show);
    }
  };

  this.updateZoomImage = function(img, w, h, src)
  {
    if (! src)
      src = FULLROOTPATH + "/static/blank.gif";
    if (img.width != w)
      img.width = w;
    if (img.height != h)
      img.height = h;
    if (img.src != src)
      img.src = src;
  };

  this.updateZoomPlot = function()
  {
    var title     = _focus ? _focus : '';
    var current   = _focusURL;
    var winWidth  = _zoomWin.getInnerWidth();
    var winHeight = _zoomWin.getInnerHeight();
    current = current.replace(/;w=\d+/, ';w=' + winWidth);
    current = current.replace(/;h=\d+/, ';h=' + winHeight);
    _self.updateZoomImage(_zoomWin.body.dom.firstChild, winWidth, winHeight, current);
    if (title != _zoomWin.title)
      _zoomWin.setTitle(title);
  };

  this.drawZoom = function(data)
  {
    /* Update zoom window position and size from server, if server owns state.
       We don't update contents of the window here, that is done via focus. */
    if (! _zoomlocal)
    {
      var winWidth  = data.w > 0 ? data.w : _zoomWin.getInnerWidth();
      var winHeight = data.h > 0 ? data.h : _zoomWin.getInnerHeight();
      if (data.x >= 0 && data.y >= 0)
	_zoomWin.setPosition(data.x, data.y);
      _zoomWin.setSize(winWidth+14, winHeight+32);
    }

    /* Always hide or show according to server state, but without events that
       are normally triggered by direct user interaction. */
    _zoomWin.suspendEvents();
    _zoomWin.setVisible(data.show);
    _zoomWin.resumeEvents();
  };

  this.zoomResize = function(panel, w, h)
  {
    _zoomlocal = true;
    var current = _zoomWin.body.dom.innerHTML;
    var newval = current
                 .replace(/width="\d+"/, 'width="' + w + '"')
                 .replace(/height="\d+"/, 'height="' + h + '"')
                 .replace(/w=\d+/, 'w=' + w)
                 .replace(/h=\d+/, 'h=' + h);
    if (newval != current)
    {
      panel.update(newval);
      _gui.asyncCall(_url() + "/setZoom?w=" + w + ';h=' +h);
    }
    return true;
  };

  this.zoomMove = function(el, x, y)
  {
    _zoomlocal = true;
    _gui.asyncCall(_url() + "/setZoom?x=" + x + ';y=' + y);
    return true;
  };

  this.zoomDrag = function(e)
  {
    // Record the x,y position of the drag proxy so that we can
    // position the Panel at end of drag.
    var el = this.proxy.getEl();
    this.x = el.getLeft(true) - _getScrollXY()[0];
    this.y = el.getTop(true) - _getScrollXY()[1];
    this.x = this.x >= 0 ? this.x : 0;
    this.y = this.y >= 0 ? this.y : 0;

    // Keep the Shadow aligned if there is one.
    var s = this.panel.getEl().shadow;
    if (s)
      s.realign(this.x, this.y, el.getWidth(), el.getHeight());
  };

  this.drawZoomInAttach = function()
  {
    var ww = (window.innerWidth || document.documentElement.clientWidth);
    var wh = (window.innerHeight || document.documentElement.clientHeight);
    var w = Math.max(200, 0.75 * ww);
    var h = Math.max(300, 0.75 * wh);
    _zoomWin = null;
    _zoomWin = new Ext.Window(
      { modal       : false,
	id          : 'zoomWin',
  	title       : _focus,
	height      : h,
	width       : w,
	resizable   : true,
	shadow      : false,
	closeAction : 'hide',
	html        : "<img width='" + w + "' height='" + h + "' src='" + FULLROOTPATH + "/static/blank.gif' />",
	style       : { position: 'fixed' },
	tools       : [{ id: 'maximize',
			 qtip: 'maximize',
			 scope: _self,
			 handler: function(event, toolEl, panel, tc) {
			     var sizes = _windowSize();
			     var offsets = _getScrollXY();
			     // the following method(setSize()) automatically fires
			     // the resize method so there is no need to implement
			     // any logic on the image here, since the 'resize'
			     // method is already taking care of it.
			     panel.setSize(sizes.width-10,sizes.height-30);
			     panel.setPagePosition(offsets[0],offsets[1]);
			 }}]});
    _zoomWin.show(this);
    _zoomWin.setVisible(false);
    _zoomWin.on('bodyresize', _self.zoomResize, _self);
    _zoomWin.on('beforehide', function() { _gui.makeCall(_url() + "/setZoom?show=no"); return false; }, _self);
    _zoomWin.on('move', _self.zoomMove, _self);
    _zoomWin.dd = new Ext.Panel.DD(_zoomWin, {
      insertProxy: false, onDrag: _self.zoomDrag,
      endDrag : function(e) { this.panel.setPosition(this.x, this.y); }});
  };

  /** Expose private variables needed to assemble the link-me hyperlink.
   *  @returns {Object} The parameters are assemble in a unique object
   *  whose keys are formatted in a way already readable by the start
   *  method exposed by the GUI. */
  this.linkMe = function()
  {
    return { size: _size, root: _rootPath,
             focus: _focus ? _focus : '',
	     zoom: _showzoom.show ? 'yes' : 'no' };
  };

  return this;
}();
