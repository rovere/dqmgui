GUI.Plugin.LHCCooldown = new function()
{
  var _canvas = $('canvas');

  this.attach = function(gui)
  {
    _canvas.style.top = 0;
    _canvas.style.bottom = 0;
    var content = ""
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<h3>LHC Cooldown Status</h3><img id='i-lhc-temp' style='margin:1em 0' src='"
      + ROOTPATH + "/plotfairy/lhc/lhc.png' />"
      + "</div>";

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;

    if (_canvas.style.display != '')
      _canvas.style.display = '';
  };

  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  };

  this.update = function(data)
  {
  };

  return this;
}();

GUI.Plugin.TPGQuery = new function()
{
  var _self = this;
  var _canvas = $('canvas');
  var _gui = null;
  this._resultDiv = null;
  this._optLine = null;
  this._optDirection = null;
  this._optStop = null;

  this._curLine = null;
  this._curDirection = null;
  this._curStop = null;

  this.attach = function(gui)
  {
    _gui = gui;
    _canvas.style.top = 0;
    _canvas.style.bottom = 0;

    var content = ""
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<div><h3>Selection</h3></div>"
      + "<div><table>"
      + "<tr id='opt-line-tr' valign='top'><td>Line</td><td>"
      + "<select style='font-size:100%' id='opt-line'>"
      + "<option value=''></option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-direction-tr' valign='top'><td>Direction</td><td>"
      + "<select style='font-size:100%' id='opt-direction'>"
      + "<option value=''></option>"
      + "</select>"
      + "</tr>"
      + "<tr id='opt-stop-tr' valign='top'><td>Stop</td><td>"
      + "<select style='font-size:100%' id='opt-stop'>"
      + "<option value=''></option>"
      + "</select>"
      + "</tr>"
      + "</table></div>"
      + "</div>"
      + "<div class='item' style='float:left;padding:.5em'>"
      + "<div><h3>Results</h3></div>"
      + "<div id='result-div'></div>"
      + "</div>";

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;

    if (_canvas.style.display != '')
      _canvas.style.display = '';

    this._resultDiv    = $('result-div');
    this._optLine      = $('opt-line');
    this._optDirection = $('opt-direction');
    this._optStop      = $('opt-stop');

    this._optLine.onchange = function() {
      _self.sendInput('line', this.value);
      return false;
    };

    this._optDirection.onchange = function() {
      _self.sendInput('direction', this.value);
      return false;
    };

    this._optStop.onchange = function() {
      _self.sendInput('stop', this.value);
      return false;
    };

    var c1 = YAHOO.util.Connect.asyncRequest
     ('GET',ROOTPATH + "/plotfairy/tpg/linelist",
      { success: GUI.Plugin.TPGQuery.receiveText,
        argument: 'linelist' });
  };

  this.receiveText = function(response)
  {
    if (response.argument == 'linelist')
    {
      _self._optLine.innerHTML = _self.makeOptionList(eval(response.responseText));
      _self.setSelection(_self._optLine, _self._curLine);
    }

    else if (response.argument == 'directionlist')
    {
      _self._optDirection.innerHTML = _self.makeOptionList(eval(response.responseText));
      _self.setSelection(_self._optDirection, _self._curDirection);
    }

    else if (response.argument == 'stoplist')
    {
      _self._optStop.innerHTML = _self.makeOptionList(eval(response.responseText));
      _self.setSelection(_self._optStop, _self._curStop);
    }

    else if (response.argument == 'times')
    {
      json = eval(response.responseText);

      content = "";
      for (var i = 0; i < json.length; i++)
        content += "<div>" + json[i] + "</div>";

      _self._resultDiv.innerHTML = content;
    }
  };

  this.setSelection = function(listobj,which)
  {
    for (var i = 0; i < listobj.length; i++)
      if (listobj.options[i].value == which)
        listobj.selectedIndex = i;
  };

  this.detach = function()
  {
    this._resultDiv = null;
    this._optLine = null;
    this._optDirection = null;
    this._optStop = null;
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  };

  this.sendInput = function(option,value)
  {
    if (value != '')
      _gui.makeCall(_url() + "/alter?field=" + encodeURIComponent(option)
                    + ";value=" + encodeURIComponent(value));
  };

  this.makeOptionList = function(data)
  {
    var result = "<option value=''></option>";

    if (data != null)
      for (var x in data)
        result += "<option value='" + x + "'>" + data[x] + "</option>";

    return result;
  };

  this.update = function(data)
  {
    var line = data['tpg.line'];
    var direction = data['tpg.direction'];
    var stop = data['tpg.stop'];

    if (line != null && line != 'null')
    {
      if (line != this._curLine)
      {
        this._curLine = line;
        this._curDirection = null;
        this._curStop = null;
        this._optDirection.innerHTML = "<option value=''></option>";
        this._optStop.innerHTML = "<option value=''></option>";
        this._resultDiv.innerHTML = "";
        var c2 = YAHOO.util.Connect.asyncRequest
          ('GET', ROOTPATH + "/plotfairy/tpg/directionlist?line="
           + encodeURIComponent(this._curLine),
           { success: GUI.Plugin.TPGQuery.receiveText,
             argument: 'directionlist' });
      }
    }

    if (direction != null && direction != 'null')
    {
      if (direction != this._curDirection)
      {
        this._curDirection = direction;
        this._curStop = null;
        this._optStop.innerHTML = "<option value=''></option>";
        this._resultDiv.innerHTML = "";
        var c3 = YAHOO.util.Connect.asyncRequest
          ('GET', ROOTPATH + "/plotfairy/tpg/stoplist?line="
           + encodeURIComponent(this._curLine) + ";direction="
           + encodeURIComponent(this._curDirection),
           { success: GUI.Plugin.TPGQuery.receiveText,
             argument: 'stoplist' });
      }
    }

    if (stop != null && stop != 'null')
    {
      if (stop != this._curStop)
      {
        this._curStop = stop;
        this._resultDiv.innerHTML = "";
        var c4 = YAHOO.util.Connect.asyncRequest
          ('GET', ROOTPATH + "/plotfairy/tpg/times?line="
           + encodeURIComponent(this._curLine) + ";direction="
           + encodeURIComponent(this._curDirection) + ";stop="
           + encodeURIComponent(this._curStop),
          { success: GUI.Plugin.TPGQuery.receiveText,
            argument: 'times' });
      }
    }
  };

  return this;
}();
