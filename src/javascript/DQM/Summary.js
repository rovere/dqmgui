/** An overview summary table of DQM sources.

    DQMSummary displays an overview summary of the DQM processing
    applications.  It shows an overall colour-coded summary and some
    essential metrics for every DQM source registering the standard
    EventInfo entities.  There is one "currently selected" source, for
    which the page shows a table of sub-part summaries and a 2D report
    summary picture.  Clicking on the DQM source names causes the
    details to be shown for that source.  */
GUI.Plugin.DQMSummary = new function()
{
  /** Reference to the GUI main object, for making server calls. */
  var _gui           = null;

  /** The DOM element for the canvas area where we display contents. */
  var _canvas        = $('canvas');

  /** Style for all cells in a table row: 3px horizontal padding. */
  var _tdstyle_all   = " style='padding:0 3px";

  /** Style for all but the last cell in a table row.  Adds faint dashed
      vertical border on the right side of the cell. */
  var _tdstyle_mid   = _tdstyle_all + ";border-right:1px dotted #d9d9d9";

  /** Style for all table header cells.  Standard padding plus uses
      normal font weight; table header is marked by background colour. */
  var _thstyle       = _tdstyle_all + ";font-weight:normal'";

  // ----------------------------------------------------------------
  /** Attach this plug-in to the GUI display.  Remembers the GUI. */
  this.attach = function(gui)
  {
    _gui = gui;
  };

  /** Detach this plug-in from the GUI display.  Clears the canvas. */
  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  };

  // ----------------------------------------------------------------
  /** Utility method to return a HTML-sanitised value if @a val
      is defined, or @a undefval otherwise. */
  this.defined = function(val, undefval)
  {
    return (val == undefined ? undefval : _sanitise(val));
  };

  /** Update this plug-in display.  Fills the canvas area with an
      up-to-date table for DQM summaries.  The currently selected - or
      first if no selection exists - row is expanded to show the
      sub-system details.  Updates the page incrementally to avoid
      flashing.

      The entire page is generated in one go, with two rows per
      sub-system.  The first row has the main summary row, and a
      second hidden row has all the details.  The second row is made
      visible for the currently selected sub-system, and the name of
      the sub-system is made bold face font to indicate selection.

      Note that the second row currently uses an embedded table which
      tries to carefully fudget the alignment of the main and embedded
      table columns to align.  I could not work out how to get the
      variable number of detail rows to behave reasonably in presence
      of a large image on the right, so ended up with two nested tables
      and some ugly fudging to get the columns to align.

      The #qmap() method moves table focus on sub-system name click. */
  this.update = function(data)
  {
    var status = data.items;
    var now = new Date();
    var content = "";

    // Loop over all items, checking for ones with enough info to
    // bother adding them to the table.  Build the table contents of
    // those, one row for the entry itself, and another details row.
    for (var i = 0, e = status.length, nrow = 1; i < e; ++i)
    {
      // Sanity check event info data.  Require processName and
      // reportSummary, but not reportSummaryContents.
      var obj = status[i];
      if (! obj.processName)
	continue;

      if (obj.reportSummary == undefined || obj.reportSummary == null)
	continue;

      // OK, has enough parts, show what we have.
      var color = (obj.reportSummary < 0 ? "#000"
		   : (obj.reportSummary < 0.85 ? "#a22"
		      : (obj.reportSummary < 0.95 ? "#c30" : "#272")));
      var bcolor = (obj.reportSummary < 0 ? "#f0f0f0"
		    : (obj.reportSummary < 0.85 ? "#fff0f0"
		       : (obj.reportSummary < 0.95 ? "#fff0c0" : "#efe")));

      var pts = new Date();
      var ets = new Date();

      // Event time stamp correct from CMSSW_1_7_1, DQMPATCH:04 onwards
      pts.setTime(obj.processTimeStamp*1000);
      ets.setTime((obj.eventTimeStamp & 0xffffffff) * 1000);

      // Extract detailed quality bits.  Remove process name prefix
      // in sub-reports (X/EventInfo/reportSummaryContents/X_Part).
      var q = {};
      var pfx1 = "reportSummaryContents/" + obj.processName + "_";
      var pfx2 = "reportSummaryContents/";
      for (key in obj)
	if (key.substr(0, pfx1.length) == pfx1)
	  q[key.substring(pfx1.length)] = obj[key];
	else if (key.substr(0, pfx2.length) == pfx2)
	  q[key.substring(pfx2.length)] = obj[key];

      var qkeys = [];
      for (var k in q)
	qkeys.push(k);
      qkeys = qkeys.sort();

      // Remember the process time stamp for image URL so we get
      // the image to refresh.  Build URL base for the image.
      var stamp = this.defined(obj.processTimeStamp, 0);
      var qrelurl = data.qrender + "/" + obj.processName + "/EventInfo/reportSummaryMap";

      // If no particular plot was chosen to be shown, show the last one.
      if (data.qplot == "" && i == e-1)
	data.qplot = obj.processName;

      // Build a row for this DQM source.  Use a complete dictionary
      // with _() here, eval() would be too slow.
      content +=
	_("<tr class='summary-mainrow' style='background-color:${bcolor};color:${color}"
	  + "'><td${_tdstyle_mid};font-weight:${rowWeight}'><a href='#' onclick='return"
	  + " GUI.Plugin.DQMSummary.qmap(\"${obj.processName}\", ${nrow})'>${processName}</a></td>"
	  + "<td align='right'>${reportSummary}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${runNumber}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${lumiNumber}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${eventNumber}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${processTimeStamp}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${eventTimeStamp}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${processedEvents}</td>"
	  + "<td align='right'${_tdstyle_mid}'>${processEventRate}</td>"
	  + "<td align='right'${_tdstyle_all}'>${obj.MEs}</td></tr>"

	  + "<tr valign='top' class='summary-qrow' style='display:${qdisplay}'>"
	  + "<td${_tdstyle_mid};border:none;padding:0' colspan='2'>${qtab}</td>"
	  + "<td align='center' colspan='8' style='padding:0;border:none'><img"
	  + " src='${qimg}' title='${processName} quality' width='532' height='400'"
	  + " /></td></tr>",
	  { '_tdstyle_mid': _tdstyle_mid,
	    '_tdstyle_all': _tdstyle_all,
	    'bcolor': bcolor,
	    'color': color,
	    'nrow': nrow,
	    'rowWeight': (data.qplot == obj.processName ? "bold" : "normal"),
	    'obj.processName': obj.processName,
	    'processName': _sanitise(obj.processName),
	    'reportSummary': (obj.reportSummary >= 0 ? sprintf("%.1f%%", obj.reportSummary*100.)
			      : (obj.reportSummary == -1 ? "No DAQ" : "N/A")),
	    'runNumber': this.defined(obj.iRun, "&#8211;"),
	    'lumiNumber': this.defined(obj.iLumiSection, "&#8211;"),
	    'eventNumber': this.defined(obj.iEvent, "&#8211;"),
	    'processTimeStamp': (obj.processTimeStamp > 0 ? _formatTime(pts, now) : "&#8211;"),
	    'eventTimeStamp': (obj.eventTimeStamp > 10000 ? _formatTime(ets, now) : "&#8211;"),
	    'processedEvents': this.defined(obj.processedEvents, "&#8211;"),
	    'processEventRate': (obj.processEventRate != undefined
				 ? Math.round(obj.processEventRate*100.)/100.
				 : "&#8211;"),
	    'obj.MEs': obj.MEs,
	    'qdisplay': (data.qplot == obj.processName ? "''" : "none"),
	    'qimg': (FULLROOTPATH + "/plotfairy/" + encodePathComponent(qrelurl)
		     + "?session=" + SESSION_ID + ";w=532;h=400;stamp=" + stamp),
	    'qtab': this.qtable(obj.processName, q, qkeys) });

      nrow += 2;
    }

    // If we had something to output, build the final page contents.
    // Otherwise show an informative neutral message that there either
    // is no data to be seen, or the sources are not registering the
    // standard entities.
    if (nrow > 1)
      content = _("<table id='summary-table' align='center'"
		  + " style='white-space:nowrap;border-collapse:collapse;"
		  + " padding:0.25em;margin-top:1em'>"
		  + "<col${_tdstyle_mid};min-width:8em' />"
		  + "<col${_tdstyle_mid};width:5em' />"
		  + "<col${_tdstyle_mid}' /><col${_tdstyle_mid}' />"
		  + "<col${_tdstyle_mid}' /><col${_tdstyle_mid}' />"
		  + "<col${_tdstyle_mid}' /><col${_tdstyle_mid}' />"
		  + "<col${_tdstyle_mid}' /><col${_tdstyle_all}' />"
		  + "<thead><tr bgcolor='#f0f0ff'>"
		  + "<th valign='bottom' align='left'${_thstyle}>Subsystem</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Summary</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Run</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Lumi section</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Event</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Last update</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Last event</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Processed events</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Event rate</th>"
		  + "<th valign='bottom' align='right'${_thstyle}>Monitor elements</th>"
		  + "</tr></thead><tbody>${content}</tbody></table>",
		  function(_) { return eval(_); });
    else
      content =
        "<div align='center' style='width:75%;font-size:2em;line-height:1.6;padding:2em;color:#700'>"
	+ "<p>No data are presently available or the DQM producers have not"
	+ " generated the standard EventInfo summary information.</p></div>";

    // Update the canvas if we have changed something.
    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  };

  /** Helper method to build the quality part table. */
  this.qtable = function(label, qmap, qkeys)
  {
    if (qkeys.length == 0)
      return "(No detail available)";

    var content = "";
    for (var i = 0, e = qkeys.length; i < e; ++i)
    {
      var name = qkeys[i];
      var value = qmap[name] * 1.0;
      var color = (value < 0 ? "#000"
		   : (value < 0.85 ? "#a22"
		      : (value < 0.95 ? "#c30" : "#272")));
      var pcolor = (value < 0 ? '#f0f0f0'
		    : (value < 0.85 ? "#fff0f0"
		       : (value < 0.95 ? "#fff0c0" : "#efe")));
      var qval = (value >= 0 ? sprintf("%.1f%%", value * 100.)
		  : (value == -1 ? "No DAQ" : "N/A"));

      content += _("<tr style='background-color:${pcolor};color:${color}'>"
		   + "<td${_tdstyle_mid};padding:0 3px 0 1em'>${name}</td>"
		   + "<td${_tdstyle_all};padding:0 1px 0 3px' align='right'>${qval}</td></tr>",
	          { 'pcolor': pcolor, 'color': color, 'name': name, 'qval': qval,
		    '_tdstyle_mid': _tdstyle_mid, '_tdstyle_all': _tdstyle_all });
    }

    return _("<table style='width:100%;border:none;"
	     + "white-space:nowrap;border-collapse:collapse'>"
	     + "<col${_tdstyle_mid}' />"
	     + "<col${_tdstyle_all};width:4.96em' />"
	     + "<tbody>${content}</tbody></table>",
	     function(_) { return eval(_); });
  };

  /** Response callback to change to a different sub-system details.
      Shows the detail rows for the selected sub-system, and hides the
      details for all others.  Notifies the server asynchronously of
      the change; does not wait or expect a reply. */
  this.qmap = function(label, nrow)
  {
    var qtable = $('summary-table');
    var i, e = qtable.rows.length;

    // Hide all details and revert to 'normal' title for all rows
    // above the selected one.
    for (i = 0; i < nrow && i < e; ++i)
    {
      if (qtable.rows[i].className == 'summary-mainrow'
	  && qtable.rows[i].cells[0].style.fontWeight != "normal")
	qtable.rows[i].cells[0].style.fontWeight = "normal";

      if (qtable.rows[i].className == 'summary-qrow'
	  && qtable.rows[i].style.display != "none")
	qtable.rows[i].style.display = "none";
    }

    // Show the details and mark with 'bold' title the selected row.
    if (nrow < e && qtable.rows[i].cells[0].style.fontWeight != "bold")
      qtable.rows[nrow].cells[0].style.fontWeight = "bold";

    if (++nrow < e)
      qtable.rows[nrow].style.display = "";

    // Hide all details and revert to 'normal' title for all rows
    // below the selected one.
    for (i = ++nrow; i < e; ++i)
    {
      if (qtable.rows[i].className == 'summary-mainrow'
	  && qtable.rows[i].cells[0].style.fontWeight != "normal")
	qtable.rows[i].cells[0].style.fontWeight = "normal";

      if (qtable.rows[i].className == 'summary-qrow'
	  && qtable.rows[i].style.display != "none")
	qtable.rows[i].style.display = "none";
    }

    // Notify the server we've changed selection.
    YAHOO.util.Connect.asyncRequest("GET", _url() + "/setQualityPlot?n="
				    + encodeURIComponent(label), {});
    return false;
  };

  return this;
}();
