
function Route()
{
}

Route.prototype = new RouteProfile();

Route.prototype.isRoutable = function()
{
	log_info("Setting target domain to sip.mysipdomain.ph");
  this.setTargetDomain("sip.mysipdomain.ph");
  return true;
}


