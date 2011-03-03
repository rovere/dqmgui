GUI.Plugin.CompWelcome = new function()
{
  var _canvas = $('canvas');
  this.attach = function(gui) {}
  this.detach = function()
  {
    while (_canvas.firstChild)
      _canvas.removeChild(_canvas.firstChild);
  }

  this.update = function(data)
  {
    var content = ""
      + "<div class='warning'>"
      + "Warning : this is an experiment future CSP monitoring framework. All instructions regarding the monitoring and alarming procedures of the CSP are currently still in <a href='https://twiki.cern.ch/twiki/bin/view/CMS/ComputingShifts' target='_blank'>ComputingShifts</a> . Eventually,the instructions will be moved to a dedicated place."
      + "</div>"

      + "<div class='top'>"
      + "<h2 class='h2'>CMS Computing is - and has to be treated as - a CMS subdetector. Core computing people engineered and built it. We expect that now a wider pool of people do contribute to constantly monitor the system(s), identify problems, and trigger actions/calls. To achieve this, we think that the creation of general CMS Computing shift, covering all the aspects and component of the overall machinery, is crucial.</h2>"
      + "</div>"

      + "<div class='links'>"
      + " Useful links"
      + "<ul>"
      + "<li><a href='http://cms.cern.ch/iCMS/jsp/secr/stats/cmsStats.jsp' target='_blank'>CMS Person Search</a></li>"
      + "<li><a href='http://consult.cern.ch/xwho' target='_blank'>CERN xwho</a></li>"
      + "<li><a href='https://cmsweb.cern.ch/sitedb/sitelist/' target='_blank'>SiteDB</a></li>"
      + "<li><a href='https://savannah.cern.ch/support/?func=additem&group=cmscompinfrasup' target='_blank'>CMS CompInfrSupport Savannah</a> <br> (please login first !)</li>"
      + "<li><a href='https://gus.fzk.de/pages/ticket.php' target='_blank'>GGUS</a></li>"
      + "<li><a href='http://espace.cern.ch/be-dep-op-lhc-machine-operation/default.aspx' target='_blank'>LHC Operation web site</a></li>"
      + "<li><a href='http://ab-dep-op-elogbook.web.cern.ch/ab-dep-op-elogbook/elogbook/elogbook.php' target='_blank'>LHC E-log</a></li>"
      + "<li><a href='http://cmsonline.cern.ch/portal/page/portal/CMS%20online%20system/Elog?_piref815_429145_815_429142_429142.strutsAction=%2FviewSubcatMessages.do%3FcatId%3D2%26subId%3D7%26page%3D1%26fetch%3D1' target='_blank'>CMS Online Shift E-log</a></li>"
      + "<li><a href='https://prod-grid-logger.cern.ch/elog/' target='_blank'>CMS Computing Logbook</a></li>"
      + "<li><a href='http://cmsdoc.cern.ch/cmscc/index.jsp' target='_blank'>CMS Centre</a></li>"
      + "<li><a href='http://cmsdoc.cern.ch/cmscc/shift/today.jsp' target='_blank'>Computing Plan of the Day</a></li>"
      + "<li><a href='http://cmsonline.cern.ch/portal/page/portal/CMS%20online%20system/Shiftlist/ShiftSelection?month=9&shift_type=25&year=2009&_piref815_525408_815_492858_492869.submit=Show&_piref815_525406_815_492858_525203.submit=Show' target='_blank'>CSP Shift Sign-Up</a></li>"
      + "</ul>"
      + "</div>"

      + "<iframe src='http://www.google.com/calendar/embed?src=d4r3obus6vva4bg191jr55u9ps%40group.calendar.google.com&src=8mvgr99s9i96noc5f027jv733ifkqc4h%40import.calendar.google.com&color=%230D7813&color=%23AB8B00&ctz=Europe/Zurich' style=' border-width:0; display:inline; ' width='800' height='600' frameborder='0' scrolling='no'></iframe>"
      + "<br />"
      + "<ul>"
      + "<li>CMS Data Taking modes</li>"
      + "<li class='indented'><span class='bold'>BEAM</span></li>"
      + "<li class='indented'><span class='bold'>CRUZET</span> : Global runs no Magnet</li>"
      + "<li class='indented'><span class='bold'>CRAFT</span>: Global runs with Magnet</li>"
      + "<li class='indented'><span class='bold'>MWGR</span>: mid-week global runs</li>"
      + "<li class='indented'><span class='bold'>GR</span>: Global runs</li>"
      + "<li class='indented'><span class='bold'>LR</span>: Local runs</li>"
      + "</ul>"

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();
