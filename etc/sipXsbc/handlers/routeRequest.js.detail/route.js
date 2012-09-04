
function Route()
{
}

Route.prototype = new RouteProfile();

Route.prototype.isRoutable = function()
{
  var target = msgGetDefaultTargetDomain();
  if (typeof target == "undefined" || target.length == 0)
    return false;
  this.setTargetDomain(target);
  
  //
  // Remove Cisco offending headers
  //
  this.sipMessage.hdrRemove("Remote-Party-Id");
  this.sipMessage.hdrRemove("Accept-Laguage");
  return true;
}


