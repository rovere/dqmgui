/** An overview of DQM Automatic ByLumi Certification Values. */
GUI.Plugin.DQMCertification = new function()
{
  /** Reference to the GUI main object, for making server calls. */
  var _gui           = null;

  /** The DOM element for the canvas area where we display contents. */
  var _canvas        = $('canvas');

  var _self          = this;
  var _localXmin     = null;
  var _localXmax     = null;
  var _zoomWin       = null;
  var _zoomlocal     = false;
  var _state         = null;
  var _selectedNode  = null;

  // ----------------------------------------------------------------
  /** Attach this plug-in to the GUI display.  Remembers the GUI. */
  this.attach = function(gui)
  {
    _gui = gui;
    this.drawZoomInAttach();
  };

  /** Detach this plug-in from the GUI display.  Clears the canvas. */
  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
    if (_zoomWin)
    {
      _zoomWin.suspendEvents();
      _zoomWin.destroy();
      _zoomWin = null;
    }
    _state = null;
    _selectedNode = null;
  };

  /** Update this plug-in display. */
  this.update = function(data)
  {
    if (_state && !_selectedNode) //FIXME: find a much better way to handle the usual refresh problem.
      return;
    else if (_selectedNode)
    {
      // Do not redraw entire Tree, but simply update the plot and the
      // current state. This is particularly usefull when the user
      // loops over runs of the same DS: in this case the plot is
      // updated, but the tree is kept unchanges.
      _state = data;
      _self.updateZoomPlot(_selectedNode);
      return;
    }
    else
    {
      _state = data;
      _zoomWin.setVisible(data.certzoom.show);
      var systems = data.items;
      var now = new Date();
      var content = "";
      var Tree = Ext.tree;

      var tree = new Tree.TreePanel({ useArrows       : true,
                                      autoScroll      : true,
                                      animate         : true,
                                      containerScroll : true,
                                      loader          : new Tree.TreeLoader(),
                                      border          : false,
                                      renderTo        : _canvas,
                                      root            : new Tree.AsyncTreeNode(
					{
					  expanded      : true,
					  leaf          : false,
					  text          : 'Certification Folders',
					  children      : systems
					}),
                                      listeners:
                                      {
					click         : function(node, e) {_self.drawZoom(node);},
					scope         : this}
                                    });

      tree.getRootNode().expand();
    }
  };

  this.drawZoom = function(node)
  {
    _selectedNode = node;
    /* Update zoom window position and size from server, if server owns state. */
    if (! _zoomlocal)
    {
      var winWidth  = _state.w > 0 ? _state.w : _zoomWin.getInnerWidth();
      var winHeight = _state.h > 0 ? _state.h : _zoomWin.getInnerHeight();
      if (_state.x >= 0 && _state.y >= 0)
        _zoomWin.setPosition(_state.x, _state.y);
      _zoomWin.setSize(winWidth+14, winHeight+32);
    }
    _gui.asyncCall(_url() + "/setCertZoom?show=yes");

    /* Always hide or show according to server state, but without events that
       are normally triggered by direct user interaction. */
    _zoomWin.suspendEvents();
    _zoomWin.setVisible(true);
    _zoomWin.resumeEvents();
    _self.updateZoomPlot(node);
  };

  this.updateZoomPlot = function(node)
  {
    var title     = node.parentNode.parentNode.text + '/EventInfo/' + node.parentNode.text + '/' + node.text;
    var current   = FULLROOTPATH + '/plotfairy/certification/'+title+'?trend=value;w=500;h=500;current='+_state.current;
    if (_localXmin.getValue())
      current += ';xmin='+_localXmin.getValue();
    if (_localXmax.getValue())
      current += ';xmax='+_localXmax.getValue();
    var winWidth  = _zoomWin.getInnerWidth();
    var winHeight = _zoomWin.getInnerHeight();
    current = current.replace(/;w=\d+/, ';w=' + winWidth);
    current = current.replace(/;h=\d+/, ';h=' + winHeight);
    _self.updateZoomImage(_zoomWin.body.dom.firstChild, winWidth, winHeight, current);
    if (title != _zoomWin.title)
      _zoomWin.setTitle(title);
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
        title       : 'Data Certification',
        height      : h,
        width       : w,
        resizable   : true,
        shadow      : false,
        closeAction : 'hide',
        html        : "<img width='" + w + "' height='" + h + "' src='" + FULLROOTPATH + "/static/blank.gif' />",
        style       : { position: 'fixed' },
        tools       : [{ id      : 'maximize',
                         qtip    : 'maximize',
                         scope   : _self,
                         handler : function(event, toolEl, panel, tc) {
                             var sizes = _windowSize();
                             var offsets = _getScrollXY();
                             // the following method(setSize()) automatically fires
                             // the resize method so there is no need to implement
                             // any logic on the image here, since the 'resize'
                             // method is already taking care of it.
                             panel.setSize(sizes.width-10,sizes.height-30);
                             panel.setPagePosition(offsets[0],offsets[1]);
                         }}],
      bbar: [
      {
        text  : 'From Lumi:',
        xtype : 'tbtext'
      },
      {
        xtype : 'numberfield',
        id    : 'localXmin',
        listeners:
        {
          change: function(obj, newValue, oldValue)
          {
            if (newValue != oldValue)
            {
              var curr = _zoomWin.body.dom.firstChild.src;
              if (curr.search(/;xmin=\d+/)>0)
                curr = curr.replace(/;xmin=\d+/, ';xmin=' + newValue);
              else
                curr += ';xmin='+newValue;
              if (newValue == '')
                curr = curr.replace(/;xmin=/, '');
              _zoomWin.body.dom.firstChild.src = curr;
            }
          },
          scope: this
        }},
        {
          text  : 'To Lumi:',
          xtype : 'tbtext'
        },
        {
          xtype : 'numberfield',
          id    : 'localXmax',
          listeners:
          {
            change: function(obj, newValue, oldValue)
            {
              if (newValue != oldValue)
              {
                var curr = _zoomWin.body.dom.firstChild.src;
                if (curr.search(/;xmax=\d+/)>0)
                  curr = curr.replace(/;xmax=\d+/, ';xmax=' + newValue);
                else
                  curr += ';xmax='+newValue;
                if (newValue == '')
                  curr = curr.replace(/;xmax=/, '');
                _zoomWin.body.dom.firstChild.src = curr;
              }
            },
            scope: this
          }
        }]});

    _zoomWin.show(this);
    _zoomWin.setVisible(false);
    _zoomWin.on('bodyresize', _self.zoomResize, _self);
    _zoomWin.on('beforehide', function() { _gui.asyncCall(_url() + "/setCertZoom?show=no"); return true; }, _self);
    _zoomWin.on('move', _self.zoomMove, _self);
    _zoomWin.dd = new Ext.Panel.DD(_zoomWin, {
      insertProxy: false, onDrag: _self.zoomDrag,
      endDrag : function(e) { this.panel.setPosition(this.x, this.y); }});
    _localXmin = Ext.getCmp('localXmin');
    _localXmax = Ext.getCmp('localXmax');
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
      _gui.asyncCall(_url() + "/setCertZoom?w=" + w + ';h=' +h);
    }
    return true;
  };

  this.zoomMove = function(el, x, y)
  {
    _zoomlocal = true;
    _gui.asyncCall(_url() + "/setCertZoom?x=" + x + ';y=' + y);
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

  return this;
}();
