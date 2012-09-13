
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
  // Retarget Cisco out of dialog REFER
  //
  if (this.sipMessage.isRequest("REFER"))
  {
    log_info("Retarget Cisco out of dialog REFER");
    var user = this.sipMessage.getRequestUriUser();
    if (typeof user == "undefined" || user.length == 0)
    {
      user = this.sipMessage.getToUser();
      if (typeof user != "undefined" && user.length > 0)
      {
        log_info("Retarget Cisco REFER to user " + user);
        this.sipMessage.setRequestUriUser(user);
      }
    }
  }

  //
  // Remove Cisco offending headers
  //
  this.sipMessage.hdrRemove("Remote-Party-Id");
  this.sipMessage.hdrRemove("Accept-Laguage");
  return true;
}


