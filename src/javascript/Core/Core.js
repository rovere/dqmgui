window.onerror = _pageErrorHandler;

var GUI = new function()
{
  //////////////////////////////////////////////////////////////////////
  // Various parameters.
  var _DEFAULT_IDLE_MESSAGE =
    "<span style='font-size: 95%'>"
    + "Please file any feature requests and any bugs you find in <a href='"
    + "https://savannah.cern.ch/bugs/?group=iguana'>Savannah</a>.</span>";
  var _IDLE_MESSAGE = _DEFAULT_IDLE_MESSAGE;

  // Most recent AJAX state.
  var _requestId = 0;
  var _lastRequest = null;
  var _requestQueue = [];
  var _updator;

  // Other state variables.
  var _gui = this;
  var _lasterr = null;
  var _body = $("canvas-group");
  var _errbody = $("errcontent");
  var _messages = $('messages');
  var _progressimg = $("cmslogo");
  var _lefttop = $('sidebar');
  var _timeid = $('time');
  var _latencyBase = new Date().getTime();
  var _latency = 1000;
  var _lastCallDuration = 0;
  var _lastDrawDuration = 0;
  var _lastServerDuration = 0;

  // The set of active plugins.
  var _plugins = [];

  // Notify plugins on window resize.
  var _winsize = _windowSize();
  window.onresize = function() {
    var newsize = _windowSize();
    if (newsize.width == _winsize.width
	&& newsize.height == _winsize.height)
      return;

    _winsize = newsize;
    /** Since the same canvas, namely 'canvas-group', is the main
     container for the content of most of the various plugins, we
     program its dynamic behavior here, instead of replicating the exact
     same piece of code in each and every plugin on the onresize
     functon. */
    var c = $('canvas-group');
    if (c)
      c.style.top = $('header').offsetHeight + 'px';

    for (var i = 0, e = _plugins.length; i < e; ++i)
      if (_plugins[i].obj.onresize)
	_plugins[i].obj.onresize();
  };

  //////////////////////////////////////////////////////////////////////
  // Available plug-ins will hang off this hook.
  this.Plugin = {};

  //////////////////////////////////////////////////////////////////////
  // Background image loader.  Browsers permit only a small number of
  // HTTP requests, usually 4, to be outstanding at any one time.  If
  // all available HTTP slots have been used to fetch images, AJAX XML
  // HTTP requests are no longer possible until the images have been
  // loaded.  This significantly reduces usability in case of network
  // trouble, slow response time, and when server is unable to serve
  // the images immediately.  This image loader fetches new images in
  // small batches on the background to avoid these problems.  It also
  // highlights images that are being updated so users get feedback on
  // which images are being loaded.
  this.ImgLoader = new function()
  {
    var _IMG_BORDER_NONE     = "1px solid transparent";
    var _IMG_BORDER_LOADING  = "1px dotted #431";
    var _IMG_UPDATE_INTERVAL = 250;
    var _IMG_MAX_LOADING     = 3;
    var _loadstyle	     = 1;
    var _pending             = {};
    var _blank               = ROOTPATH + "/static/blank.gif";
    var _missing             = ROOTPATH + "/static/missing.png";
    var _imgdiv		     = $("imgloader");

    this._flash = function()
    {
      _loadstyle = (_loadstyle+1) % 2;
      for (x in _pending)
      {
	var i = _pending[x];
        i.img.style.border = (_loadstyle ? _IMG_BORDER_LOADING : i.flash);
      }
    };

    this._init = function(img, label, info, src)
    {
      info.loader = img;
      info.img.style.border = _IMG_BORDER_LOADING;
      img.onload = function() { GUI.ImgLoader._onload(img, label, info, 1); };
      img.onerror = function() { GUI.ImgLoader._onload(img, label, info, 0); };
      img.src = src;
    };

    this._startnew = function()
    {
      for (x in _pending)
      {
	var i = _pending[x];
	if (i.loader)
	  continue;

	if (_imgdiv.childNodes.length >= _IMG_MAX_LOADING)
	  break;

	var loader = document.createElement("img");
	loader.width = loader.height = 1;
	loader.style.visibility = 'hidden';
        _imgdiv.appendChild(loader);
	this._init(loader, x, i, i.src);
      }
    };

    this._onload = function(img, label, i, ok)
    {
      if (img.parentNode != _imgdiv)
        return;
      i.img.src = ok ? img.src : _missing;
      i.img.style.border = i.border || _IMG_BORDER_NONE;
      _imgdiv.removeChild(img);
      delete _pending[label];
      GUI.ImgLoader._startnew();
    };

    this.reset = function()
    {
      _pending = {};
      while (_imgdiv.firstChild)
      {
	_imgdiv.firstChild.onload = null;
	_imgdiv.firstChild.onerror = null;
	_imgdiv.removeChild(_imgdiv.firstChild);
      }
    };

    this.load = function(label, img, src, border, flash)
    {
      border = border || img.style.border;
      flash = flash || border || _IMG_BORDER_NONE;
      if (label in _pending && _pending[label].loader)
      {
        var i = _pending[label];
	i.loader.onload = null;
	i.loader.onerror = null;
	_imgdiv.removeChild(i.loader);
      }
      _pending[label] = { img: img, src: src, border: border, flash: flash, loader: null };
      img.style.border = flash;
      GUI.ImgLoader._startnew();
    };

    setInterval(this._flash, _IMG_UPDATE_INTERVAL);
    return this;
  }();

  //////////////////////////////////////////////////////////////////////
  // Core GUI functionality.
  this.updateInterval = 15*1000;
  this.makeCall = function(url)
  {
    var id = _requestId++;

    _progressimg.src = ROOTPATH + "/static/cms-progress.gif";
    _lastRequest = YAHOO.util.Connect.asyncRequest
      ("GET", url, { scope: _gui,
		     argument: { id: id, start: new Date().getTime() },
		     success: _gui.updateState,
		     failure: _gui.onFail });
  };

  this.asyncCall = function(url)
  {
    YAHOO.util.Connect.asyncRequest("GET", url, {});
  };

  this.scheduleUpdate = function(multiplier)
  {
    // Keep track of what our latency looks like and report to server.
    var now = new Date().getTime();
    _latency = now - _latencyBase;
    _latencyBase = now;
    clearTimeout(_updator);
    _updator = setTimeout(function() {
	if (! _lastRequest)
	   _gui.makeCall(_url() + "/state?auto="
			 + multiplier*_gui.updateInterval
			 + ";latency=" + _latency
			 + ";call=" + _lastCallDuration
			 + ";server=" + _lastServerDuration
			 + ";render=" + _lastDrawDuration);
      },
      multiplier*_gui.updateInterval);
  };

  this.init = function()
  {
    $('body').style.display = '';
    _progressimg.className = "loading";
    _messages.innerHTML = "Loading content...";
    this.scheduleUpdate(0);
  };

  this.setIdleMessage = function(str)
  {
    var old = _IDLE_MESSAGE;
    _IDLE_MESSAGE = str ? str : _DEFAULT_IDLE_MESSAGE;
    if (_messages.innerHTML == old)
      _messages.innerHTML = str;
    return old;
  };

  this.buildLeftSide = function(id, titleName)
  {
    var group = document.createElement('div');
    _lefttop.appendChild(group);

    var title = document.createElement('h3');
    group.appendChild(title);

    var content = document.createElement('div');
    group.appendChild(content);

    group.id = id;
    group.className = 'group';
    title.className = 'group-title';
    content.className = 'group-content';
    title.innerHTML = titleName;
    return group;
  };

  this.updateServerDuration = function(t)
  {
    _lastServerDuration = t;
  };

  this.updateState = function(o)
  {
    try
    {
      GUI.ImgLoader.reset();
      _gui.performUpdateState(o);
    }
    catch (err)
    {
      var fileName = (err.fileName ? err.fileName.replace(/.*\//, ".../") : "(unknown)");
      var lineNumber = (err.lineNumber ? ":" + err.lineNumber : "");
      _messages.innerHTML
        = "<span class='alert'>Please <a href='https://savannah.cern.ch/bugs/"
        + "?group=iguana'>report</a> the following error updating this page: "
        + err.name + ", " + err.message
	+ " at " + fileName + lineNumber + "</span>";
      _progressimg.className = "internal-error";
    }
  };

  this.performUpdateState = function(o)
  {
    var now = new Date();
    _lastCallDuration = now.getTime() - o.argument.start;
    _lastServerDuration = -1;
    _lastDrawDuration = -1;

    if (o.status != 200)
      return this.onFail (o);

    // If the response is too old, discard it.
    if (o.argument.id < _requestId - 1)
    {
      _progressimg.className = "delayed";
      _messages.innerHTML
	= "(Server responses remaining to catch up with: "
	  + (_requestId - o.argument.id) + ")";
      return;
    }

    // Evaluate the JSON response text.  We get back an array of
    // dictionaries, one per plug-in of content.  We loop over all of
    // these, instantiate the plug-ins if necessary, and then ask each
    // one to refresh its contents.  Once done, clear the message line.
    if (o.responseText.substr(0, 10) == "([{'kind':")
    {
      var response = eval(o.responseText);
      _progressimg.className = "idle";
      _messages.innerHTML = _IDLE_MESSAGE;
      _timeid.innerHTML = sprintf("%s %d, %d at %02d:%02d.%02d UTC",
				  _MONTH[now.getUTCMonth()],
				  now.getUTCDate(),
				  now.getUTCFullYear(),
				  now.getUTCHours(),
				  now.getUTCMinutes(),
				  now.getUTCSeconds());

      // First detach unwanted plugins.
      for (var i = 0, e = _plugins.length; i < e; ++i)
      {
	var plugin = _plugins[i];
	if (plugin && (i >= response.length || response[i].kind != plugin.kind))
	{
	  plugin.obj.detach();
	  _plugins[i] = null;
	}
      }
      _plugins.length = response.length;

      for (var i = 0, e = response.length; i < e; ++i)
      {
	var item = response[i];
	var plugin = _plugins[i];
	if (! plugin)
	{
	  // FIXME: menu ordering?
	  _plugins[i] = plugin = { kind: item.kind, obj: this.Plugin[item.kind] };
	  plugin.obj.attach(this);
	}

	plugin.obj.update(item);
      }
    }
    else
      _messages.innerHTML = "<span class='alert'>Ignored invalid response"
			    + "received from the server starting with '"
			    + _sanitise(o.responseText.substr(0, 20))
			    + "...'. Check for connection problems?</span>";

    _lastRequest = null;
    _progressimg.src = ROOTPATH + "/static/cms.gif";
    _lastDrawDuration = new Date().getTime() - now.getTime();
    this.scheduleUpdate(1);
  };

  this.onFail = function(o)
  {
    // FIXME: Elaborate error message.
    // FIXME: Reinstall (slow) update.
    _lasterr = o;
    this.hideError();

    _lastRequest = null;
    _progressimg.src = ROOTPATH + "/static/cms.gif";
    this.scheduleUpdate(6);
  };

  this.hideError = function()
  {
    _errbody.style.display = 'none';
    _body.style.display = '';

    if (_lasterr.status > 0)
      _progressimg.className = "communication-error";

    _messages.innerHTML
      = "<span class='alert'>Communication failure with the DQM GUI"
	+ " server, HTTP status code was <a href='#' onclick='return GUI.showError()'>"
	+ _lasterr.status + "</a>.</span>";

    return false;
  };

  this.showError = function()
  {
    _body.style.display = 'none';
    _errbody.style.display = '';

    var headers = [];
    for (var i in _lasterr.getResponseHeader)
      headers.push(i);
    headers = headers.sort();

    var content
      = "<div align='center' style='display:block;position:fixed;padding:1em;"
      + " background:#ffffff;left:0;right:0;top:5em;bottom:5em;font-size:1em;"
      + " overflow:auto'>"
      + "<table style='white-space:nowrap'>"
      + "<tr><td colspan='2' style='padding-bottom:2em;font-size:1.5em;"
      + " font-weight:bold'><a href='#' onclick='return GUI.hideError()'>"
      + "Back</a></td></tr>"
      + "<tr valign='top' style='font-size:.85em'><td>Server response</td>"
      + "<td>" + _lasterr.status + " (" + _lasterr.statusText + ")</td></tr>"
      + "<tr valign='top' style='font-size:.85em'><td>Response headers</td>"
      + "<td bgcolor='#eee'>"
      + "<div width='75%' style='overflow:auto;white-space:pre'>";

    for (var i = 0, e = headers.length; i < e; ++i)
      content += headers[i] + ": " + _sanitise(_lasterr.getResponseHeader[headers[i]]) + "\n";

    content += "</pre></td></tr>"
      + "<tr valign='top' style='font-size:.85em'><td>Response text</td>"
      + "<td bgcolor='#eee'>"
      + "<div width='75%' style='overflow:auto;white-space:pre'>"
      + _sanitise(_lasterr.responseText) + "</pre></td></tr>"
      + "</table></div>";

    _errbody.innerHTML = content;
    _messages.innerHTML = "";
    return false;
  };

  return this;
}();

//////////////////////////////////////////////////////////////////////
GUI.Plugin.AutoUpdate = new function() {
  var _gui = null;
  var _stamp = 0;
  this.attach = function(gui) { _gui = gui; };
  this.detach = function() { };
  this.update = function(state)
  {
    _gui.updateServerDuration(state.serverTime);

    if (state.interval > 0)
      _gui.updateInterval = state.interval * 1000;

    if (! _stamp)
      _stamp = state.stamp;
    else if (_stamp != state.stamp)
      window.location.reload();
  };
}();
