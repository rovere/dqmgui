GUI.Plugin.DQMPlay = new function()
{
  Ext.QuickTips.init();

  var _gui		= null;
  var _self		= this;
  var _canvas		= $('canvas');
  var _optsarea		= $('canvas-opts');
  var _playarea         = $('canvas-play');
  var _playInterval	= null;
  var _optPlayInterval  = null;

  var _data		= { items: [], interval: 5, pos: 0, max: 0 };
  var _click		= { event: null, timeout: null, timeClick: 0, timeDoubleClick: 0 };

  this.onresize = function()
  {
    var h = _optsarea.clientHeight;
    var size = _SIZEMAP["XL"];
    for (var i = 0, obj = _canvas.firstChild; obj; ++i, obj = obj.nextSibling)
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

  this.attach = function(gui)
  {
    window.onresize = function() {
      $('canvas-group').style.top = $('header').offsetHeight + 'px';
    };

    _gui = gui;
    _optsarea.style.display = '';
    _playarea.style.display = '';
    var tb = new Ext.Toolbar({ id : 'canvas-submenu' });
    tb.add({ text          : 'Stop',
	     enableToggle  : false,
	     handler       : function() { _self.stop(); return false; }
	   }, '-',
	   { text	   : 'Rate:',
	     xtype         : 'tbtext'
	   },
	   { xtype         : 'numberfield',
	     value         : _data.interval,
	     allowBlank    : false,
	     allowDecimals : false,
	     allowNegative : false,
	     minValue      : 2,
	     maxValue      : 99,
	     style         : { 'font-size': '100%', 'width': '5ex' },
	     id            : 'play-interval-value',
	     listeners     : { change: function(el, val, old) { _self.interval(val); } }
	   },
	   { text	   : ' sec',
	     xtype         : 'tbtext'
	   });

    tb.render('canvas-opts');
    tb.doLayout();
    _optPlayInterval = Ext.getCmp('play-interval-value');
  };

  this.detach = function()
  {
    if (_playInterval)
    {
      clearInterval(_playInterval);
      _playInterval = null;
    }

    _canvas.onclick = null;
    _canvas.innerHTML = "";
    _playarea.innerHTML = "";
    _optsarea.innerHTML = "";
    _playarea.style.display = 'none';
    _optsarea.style.display = 'none';
  };

  this.stop = function()
  {
    if (_playInterval)
    {
      clearInterval(_playInterval);
      _playInterval = null;
    }

    _gui.makeCall(_url() + "/stop");
  };

  this.interval = function(value)
  {
    var newval = parseInt(value);
    if (isNaN(newval))
      newval = 5;
    else if (newval < 2)
      newval = 2;
    else if (newval > 99)
      newval = 99;

    if (newval != _data.interval)
    {
      _data.interval = newval;
      if (_optPlayInterval.getValue() != newval)
        _optPlayInterval.setValue(newval);
      _gui.makeCall(_url() + "/interval?v=" + newval);
    }
  };

  this.step = function()
  {
    var curpos = _data.pos;
    if (curpos < _playarea.childNodes.length)
    {
      var curnode = _playarea.childNodes[curpos];
      _setclass(curnode, curnode.className.replace(/ playhead/, ""));
    }

    if (++_data.pos >= _data.max)
    {
      clearInterval(_playInterval);
      _playInterval = null;
      _data.pos = 0;
      if (_data.max)
      {
        _gui.makeCall(_url() + "/step?pos=" + _data.pos + ";move=yes");
        return;
      }
    }

    if (_data.items.length)
    {
      curpos = _data.pos;
      if (curpos >= _data.items.length)
      {
        clearInterval(_playInterval);
        _playInterval = null;
        _gui.makeCall(_url() + "/step?pos=" + _data.pos + ";move=yes");
      }
      else
      {
        var curnode = _playarea.childNodes[curpos];
        _setclass(curnode, curnode.className + " playhead");

        layout("div", _canvas, _canvas.firstChild, _data.items[curpos],
	       "XL", _data.reference, _data.strip, null, null, "", "div0");
        YAHOO.util.Connect.asyncRequest("GET", _url() + "/step?pos=" + _data.pos, {});
      }
    }
  };

  this.update = function(data)
  {
    _data = data;

    // Put the selected object in full-window size and clear the
    // the rest of the main canvas.
    var item = null;
    if (_data.pos < _data.items.length)
      item = layout("div", _canvas, _canvas.firstChild,
		    _data.items[_data.pos], "XL",
		    _data.reference, _data.strip,
		    null, null, "", "div0")
	     .nextSibling;
    else
      item = _canvas.firstChild;

    while (item)
    {
      var next = item.nextSibling;
      _canvas.removeChild(item);
      item = next;
    }

    // Fill the play list with small objects.
    for (var n = 0, item = _playarea.firstChild; n < _data.items.length;
	 ++n, item = item.nextSibling)
      item = layout("div", _playarea, item, _data.items[n], "XS",
		    _data.reference, _data.strip, null, null,
		    n == _data.pos ? " playhead" : "", "pdiv" + n);

    // Remove whatever is left of old contents.
    while (item)
    {
      var next = item.nextSibling;
      _playarea.removeChild(item);
      item = next;
    }

    // Handle resize.
    this.onresize();

    // Update play interval.
    if (_optPlayInterval.getValue() != _data.interval)
      _optPlayInterval.setValue(_data.interval);

    if (_playInterval)
    {
      clearInterval(_playInterval);
      _playInterval = null;
    }

    _playInterval = setInterval(this.step, _data.interval*1000);
  };

  return this;
}();
