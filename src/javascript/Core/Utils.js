//////////////////////////////////////////////////////////////////////
// Compatibility stuff.
var None = null;

function _maparray(array, fun /*, thisp */)
{
  return array.map(fun, arguments[1]);
}

function _maplist(list, fun /*, thisp */)
{
  var len = list.length;
  if (typeof fun != "function")
    throw new TypeError();

  var res = new Array(len);
  var thisp = arguments[1];
  for (var i = 0; i < len; i++)
    res[i] = fun.call(thisp, list[i], i, this);

  return res;
}

var _map;
if (Array.prototype.map)
  _map = function(array, fun /*, thisp */) {
    if (array.map)
      return array.map(fun, arguments[1]);
    else
      return _maplist(array, fun, arguments[1]);
  };
else
  _map = _maplist;

function _windowSize()
{
  var d = document;
  if (typeof window.innerWidth != 'undefined')
    return { width: window.innerWidth, height: window.innerHeight };
  else if (d.documentElement
	   && typeof d.documentElement.clientWidth != 'undefined'
	   && d.documentElement.clientWidth != 0)
    return { width: d.documentElement.clientWidth,
	     height: d.documentElement.clientHeight };
  else if (d.body && typeof d.body.clientWidth != 'undefined')
    return { width: d.body.clientWidth, height: d.body.clientHeight };
  else
    return { width: 0, height: 0 };
}

function _grep(list, func)
{
  result = [];
  for (var i = 0; i < list.length; ++i)
  {
    var x = list[i];
    if (func(x))
      result.push(x);
  }
  return result;
}

function _clone(obj) // from mochikit, see http://oranlooney.com/functional-javascript
{
  var me = arguments.callee;
  if (arguments.length == 1)
  {
    me.prototype = obj;
    return new me();
  }
}

function _dump(o)
{
    if (typeof(o) == 'object')
  {
    var result = "{ ";
    var joiner = "";
    for (i in o)
    {
      result += joiner + i + ": " + _dump(o[i]);
      joiner = ", ";
    }
    return result + " }";
  }
  else if (typeof(o) == typeof(""))
    return "\"" + o + "\"";
  else
    return "" + o;
}

// Adapted from http://ejohn.org/files/jsdiff.js
// FIXME: http://search.cpan.org/src/NEDKONZ/Algorithm-Diff-1.15/lib/Algorithm/Diff.pm?
function _diff(o, n, f)
{
  var ns = new Object();
  var os = new Object();

  for (var i = 0; i < n.length; i++)
  {
    var x = f(n[i]);
    if (ns[x] == null) ns[x] = { rows: [], o: null };
    ns[x].rows.push(i);
  }

  for (var i = 0; i < o.length; i++)
  {
    var x = f(o[i]);
    if (os[x] == null) os[x] = { rows: [], n: null };
    os[x].rows.push(i);
  }

  for (var i in ns)
    if (ns[i].rows.length == 1
	&& typeof(os[i]) != "undefined"
	&& os[i].rows.length == 1)
    {
      n[ns[i].rows[0]] = { orig: n[ns[i].rows[0]], row: os[i].rows[0] };
      o[os[i].rows[0]] = { orig: o[os[i].rows[0]], row: ns[i].rows[0] };
    }

  for (var i = 0; i < n.length - 1; i++)
    if (n[i].orig != null
	&& n[i+1].orig == null
	&& n[i].row + 1 < o.length
	&& o[n[i].row + 1].orig == null
	&& n[i+1] == o[ n[i].row + 1 ] )
    {
      n[i+1] = { orig: n[i+1], row: n[i].row + 1 };
      o[n[i].row+1] = { orig: o[n[i].row+1], row: i + 1 };
    }

  for (var i = n.length - 1; i > 0; i--)
    if (n[i].orig != null
	&& n[i-1].orig == null
	&& n[i].row > 0
	&& o[n[i].row - 1].orig == null
	&& n[i-1] == o[ n[i].row - 1])
    {
      n[i-1] = { orig: n[i-1], row: n[i].row - 1 };
      o[n[i].row-1] = { orig: o[n[i].row-1], row: i - 1 };
    }

  return { o: o, n: n };
}

function _setval (obj, className, newval)
{
  if (obj.innerHTML != newval)
    obj.innerHTML = newval;
  if (obj.className != className)
    obj.className = className;
}

function _setvaltitle (obj, className, newval, newtitle)
{
  if (obj.innerHTML != newval)
  {
    obj.innerHTML = newval;
    obj.title = newtitle;
  }
  if (obj.className != className)
    obj.className = className;
}

function _settitle (obj, className, newval)
{
  if (obj.title != newval)
    obj.title = newval;
  if (obj.className != className)
    obj.className = className;
}

function _setclass (obj, className)
{
  if (obj.className != className)
    obj.className = className;
}

function _url ()
{
  return location.href.replace(/#.*/, "");
}

function _showOrHide(link,obj,hideLink)
{
  if (typeof(obj) != 'object')
    obj = document.getElementById(obj);

  if (obj.style.display == '')
  {
    obj.style.display = 'none';
    if (hideLink) link.style.display = '';
  }
  else
  {
    obj.style.display = '';
    if (hideLink) link.style.display = 'none';
  }
  return false;
}

function _dblclick(e, data, singleCall, doubleCall)
{
  var _DBL_CLICK_DELAY	 = 100; // No clicks after double click
  var _DBL_CLICK_TIME	 = 250; // Single-to-double click time window

  e.cancelBubble = true;
  if (e.stopPropagation)
    e.stopPropagation();

  var t = new Date().getTime();
  if (e.type == 'click')
  {
    if (t - data.timeDoubleClick < _DBL_CLICK_DELAY)
      return false;
    data.event = { srcElement: (e.srcElement || e.target),
		   type: e.type, x: e.x || e.clientX };
    data.timeClick = t;
    data.timeout = setTimeout(function() {
      	var event = data.event;
      	data.event = null;
      	data.timeClick = 0;
      	data.timeDoubleClick = 0;
      	return (event && singleCall(event));
      }, _DBL_CLICK_TIME);
  }
  else if (e.type == 'dblclick')
  {
    data.timeDoubleClick = new Date().getTime();
    if (data.timeout)
    {
      clearTimeout(data.timeout);
      data.timeout = null;
      data.event = null;
    }
    doubleCall(e);
  }
}

var _WEEKDAY = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];
var _MONTH = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];

function _formatTime(ref, now)
{
  var str = "";
  var future = 0;
  var diff = (now.getTime() - ref.getTime()) / 1000;
  var nowday = now.getDay();
  var refday = ref.getDay();

  if (diff < 0)
  {
    diff = -diff;
    future = 1;
  }

  if (diff < 60 && refday == nowday)
    return sprintf("%d'' %s",
		   diff,
		   future ? "in future" : "ago");

  if (diff < 3600 && refday == nowday)
    return sprintf("%d' %d'' %s",
		   (diff % 3600) / 60,
		   (diff % 60),
		   future ? "in future" : "ago");

  if (diff < 4*3600 && refday == nowday)
    return sprintf("%dh %d' %d'' %s",
		   diff / 3600,
		   (diff % 3600) / 60,
		   (diff % 60),
		   future ? "in future" : "ago");

  if (diff < 86400 && ! future)
    return sprintf("%sat %02d:%02d.%02d",
		   (refday == nowday ? "" : "Yesterday "),
		   ref.getHours(),
		   ref.getMinutes(),
		   ref.getSeconds());

  if (diff < 7*86400 && ! future)
    return sprintf("%s %02d:%02d.%02d",
		   _WEEKDAY[ref.getDay()],
		   ref.getHours(),
		   ref.getMinutes(),
		   ref.getSeconds());

  if (diff < 365*86400 && ! future)
    return sprintf("%s %d, %02d:%02d.%02d",
		   _MONTH[ref.getMonth()],
		   ref.getDate(),
		   ref.getHours(),
		   ref.getMinutes(),
		   ref.getSeconds());

  return sprintf("%s %d, %d, %02d:%02d.%02d",
		 _MONTH[ref.getMonth()],
		 ref.getDate(),
		 ref.getFullYear(),
		 ref.getHours(),
		 ref.getMinutes(),
		 ref.getSeconds());
}

/** Utility method to make an arbitrary string safe for display in
    HTML context. */
function _sanitise(str)
{
  if (str)
  {
    str = str.replace(/&/g, "&amp;");
    str = str.replace(/</g, "&lt;");
    str = str.replace(/>/g, "&gt;");
  }
  return str;
}

/** A page error handler which sets the message area message
    to a request to report a bug with the error details. */
function _pageErrorHandler(msg, url, line)
{
  _$("cmslogo").className = "internal-error";
  _$("messages").innerHTML =
    "<span class='alert'>Please <a href='https://savannah.cern.ch/bugs/"
    + "?group=iguana'>report</a> the following programming error in the"
    + " GUI in " + url.replace(/.*\//, ".../") + ":" + line + ": "
    + _sanitise(msg.toString())
    + "</span>";
  return true;
}

/** Utility function to look up a document element or elements by
    name.  The arguments are either a single string, or an array of
    strings.  For the former returns a document element by that name,
    for the latter returns an array of elements, one for each name. */
function _$()
{
  var elements = new Array();
  for (var i = 0; i < arguments.length; i++)
  {
    var element = arguments[i];
    if (typeof element == 'string')
      element = document.getElementById(element);
    if (arguments.length == 1)
      return element;
    elements.push(element);
  }
  return elements;
}

/** Utility function to look up a document element by type.  Returns
    elements of type @a type that are children of @a el. */
function $$(el, type)
{
  return (el || document).getElementsByTagName(type);
}

/** Utility to make string replacements.

    For each ${var} in @a str looks for replacements in @a opts, which
    may be a dictionary or a function.  If it is a dictionary and the
    value of key 'var' is a string or a number, the replacement is
    made.  If @a opts is a function, it's invoked with the name 'var'
    for each replacement.

    Returns the string after all possible replacements have been made. */
function _(str, opts)
{
  return str.replace(/\$\{([^{}]*)\}/g,
		     function (a, b) {
		       if (typeof opts === 'function')
			 return opts(b);
		       var r = opts[b];
		       return typeof r === 'string' || typeof r === 'number' ? r : a;
		     });
}

function encodePathComponent(str)
{
  return encodeURIComponent(str).replace(/%2F/g, "/");
}

/** Build a mapping table from the <option> values of a
    <select> to corresponding index for use as selectedIndex. */
function _optionmap(sel)
{
  var map = {};
  for (var i = 0; i < sel.childNodes.length; ++i)
    map[sel.childNodes[i].value] = i;
  return map;
}

/** function that returns the scrolling offsets in both the horizontal
    and vertical direction. See
    http://www.howtocreate.co.uk/tutorials/javascript/browserwindow
    for original source code.

    Returns and array containing scrolled offsets X and Y
    respectively. */
function _getScrollXY() {
  var scrOfX = 0, scrOfY = 0;
  if( typeof( window.pageYOffset ) == 'number' ) {
    //Netscape compliant
    scrOfY = window.pageYOffset;
    scrOfX = window.pageXOffset;
  } else if( document.body && ( document.body.scrollLeft || document.body.scrollTop ) ) {
    //DOM compliant
    scrOfY = document.body.scrollTop;
    scrOfX = document.body.scrollLeft;
  } else if( document.documentElement && ( document.documentElement.scrollLeft || document.documentElement.scrollTop ) ) {
    //IE6 standards compliant mode
    scrOfY = document.documentElement.scrollTop;
    scrOfX = document.documentElement.scrollLeft;
  }
  return [ scrOfX, scrOfY ];
}

/** Function that returns the position of the passed object
 *  inside the renderd html page.
 *
 *  Returns an array with the calculated coordinates to
 *  whichever script asked for it.
 *
 *  See http://www.quirksmode.org/js/findpos.html */

function findPos(obj) {
  var curleft = curtop = 0;
  if (obj.offsetParent) {
    do {
      curleft += obj.offsetLeft;
      curtop += obj.offsetTop;
      console.log('findPos, el, top, left', obj, curleft, curtop) ;
    } while ((obj = obj.offsetParent));
    return [curleft,curtop];
  }
  else
    return [0,0] ;
}

function localToGlobal( _el ) {
  var  target = _el,
  target_width = target.offsetWidth,
  target_height = target.offsetHeight,
  target_left = target.offsetLeft,
  target_top = target.offsetTop,
  gleft = 0,
  gtop = 0,
  rect = {};

  var moonwalk = function( _parent ) {
    if (!!_parent) {
      gleft += _parent.offsetLeft;
      gtop += _parent.offsetTop;
      console.log('localToGlobal, el, top, left', _parent, gleft, gtop) ;
      moonwalk( _parent.offsetParent );
    }
    return rect = {
      top: target.offsetTop + gtop,
      left: target.offsetLeft + gleft,
      bottom: (target.offsetTop + gtop) + target_height,
      right: (target.offsetLeft + gleft) + target_width
    };
  };
  moonwalk( target.offsetParent );
  return rect;
}

/** Function to test if a specific @a url is available or not. In
    order to avoid excessive load on both client and server, the AJAX
    request is sent using the "HEAD" method, since we are interested
    only on the status of the request, not on the content. The status
    is passed to the @a callback function for further
    customization. */

function _urlExists(url, callback)
{
    var http = new XMLHttpRequest();
    http.open('HEAD', url);
    http.onreadystatechange = function() {
        if (this.readyState == this.DONE) {
            callback(this.status);
        }
    };
    http.send();
}

/*
Copyright (c) 2008 Fred Palmer fred.palmer_at_gmail.com

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
function StringBuffer()
{
    this.buffer = [];
}

StringBuffer.prototype.append = function append(string)
{
    this.buffer.push(string);
    return this;
};

StringBuffer.prototype.toString = function toString()
{
    return this.buffer.join("");
};

var Base64 =
{
    codex : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",

    encode : function (input)
    {
        var output = new StringBuffer();

        var enumerator = new Utf8EncodeEnumerator(input);
        while (enumerator.moveNext())
        {
            var chr1 = enumerator.current;

            enumerator.moveNext();
            var chr2 = enumerator.current;

            enumerator.moveNext();
            var chr3 = enumerator.current;

            var enc1 = chr1 >> 2;
            var enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
            var enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
            var enc4 = chr3 & 63;

            if (isNaN(chr2))
            {
                enc3 = enc4 = 64;
            }
            else if (isNaN(chr3))
            {
                enc4 = 64;
            }

            output.append(this.codex.charAt(enc1) + this.codex.charAt(enc2) + this.codex.charAt(enc3) + this.codex.charAt(enc4));
        }

        return output.toString();
    },

    decode : function (input)
    {
        var output = new StringBuffer();

        var enumerator = new Base64DecodeEnumerator(input);
        while (enumerator.moveNext())
        {
            var charCode = enumerator.current;

            if (charCode < 128)
                output.append(String.fromCharCode(charCode));
            else if ((charCode > 191) && (charCode < 224))
            {
                enumerator.moveNext();
                var charCode2 = enumerator.current;

                output.append(String.fromCharCode(((charCode & 31) << 6) | (charCode2 & 63)));
            }
            else
            {
                enumerator.moveNext();
                var charCode2 = enumerator.current;

                enumerator.moveNext();
                var charCode3 = enumerator.current;

                output.append(String.fromCharCode(((charCode & 15) << 12) | ((charCode2 & 63) << 6) | (charCode3 & 63)));
            }
        }

        return output.toString();
    }
}


function Utf8EncodeEnumerator(input)
{
    this._input = input;
    this._index = -1;
    this._buffer = [];
}

Utf8EncodeEnumerator.prototype =
{
    current: Number.NaN,

    moveNext: function()
    {
        if (this._buffer.length > 0)
        {
            this.current = this._buffer.shift();
            return true;
        }
        else if (this._index >= (this._input.length - 1))
        {
            this.current = Number.NaN;
            return false;
        }
        else
        {
            var charCode = this._input.charCodeAt(++this._index);

            // "\r\n" -> "\n"
            //
            if ((charCode == 13) && (this._input.charCodeAt(this._index + 1) == 10))
            {
                charCode = 10;
                this._index += 2;
            }

            if (charCode < 128)
            {
                this.current = charCode;
            }
            else if ((charCode > 127) && (charCode < 2048))
            {
                this.current = (charCode >> 6) | 192;
                this._buffer.push((charCode & 63) | 128);
            }
            else
            {
                this.current = (charCode >> 12) | 224;
                this._buffer.push(((charCode >> 6) & 63) | 128);
                this._buffer.push((charCode & 63) | 128);
            }

            return true;
        }
    }
}

function Base64DecodeEnumerator(input)
{
    this._input = input;
    this._index = -1;
    this._buffer = [];
}

Base64DecodeEnumerator.prototype =
{
    current: 64,

    moveNext: function()
    {
        if (this._buffer.length > 0)
        {
            this.current = this._buffer.shift();
            return true;
        }
        else if (this._index >= (this._input.length - 1))
        {
            this.current = 64;
            return false;
        }
        else
        {
            var enc1 = Base64.codex.indexOf(this._input.charAt(++this._index));
            var enc2 = Base64.codex.indexOf(this._input.charAt(++this._index));
            var enc3 = Base64.codex.indexOf(this._input.charAt(++this._index));
            var enc4 = Base64.codex.indexOf(this._input.charAt(++this._index));

            var chr1 = (enc1 << 2) | (enc2 >> 4);
            var chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
            var chr3 = ((enc3 & 3) << 6) | enc4;

            this.current = chr1;

            if (enc3 != 64)
                this._buffer.push(chr2);

            if (enc4 != 64)
                this._buffer.push(chr3);

            return true;
        }
    }
};
