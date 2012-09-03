function Route404()
{
}
Route404.prototype = new RouteProfile();

Route404.prototype.routeRequest = function()
{
  log_info("VERIFIED Route404.isRoutable");
  this.setRejectReason("No route available");
  this.routeReject();
}

