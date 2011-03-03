GUI.Plugin.CompGrid = new function()
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
      + "<a href='http://dashb-ssb.cern.ch/dashboard/request.py/siteviewhome' target='_blank'>Site Status for the CMS sites</a>"
      + "<br />"
      + "<iframe "
      + "src='http://www.google.com/calendar/embed?height=600&amp;wkst=1&amp;bgcolor=%23FFFFFF&amp;"
      + "src=6cb9c0gl57akvgk4f42q8c4sitaplk4g%40import.calendar.google.com&amp;color=%238D6F47&amp;"
      + "src=t5gki1ogro2v5n8hn93q5lkfnqdra4ir%40import.calendar.google.com&amp;color=%2328754E&amp;"
      + "ctz=Etc%2FGMT' style=' border-width:0; margin-top: 30px; ' width='800' height='800' frameborder='0' scrolling='no'></iframe>";

    if (_canvas.innerHTML != content)
      _canvas.innerHTML = content;
  }

  return this;
}();
