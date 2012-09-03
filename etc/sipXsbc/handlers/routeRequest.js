var route = new Route();
var route404 = new Route404();

function handle_request(request)
{
  route.setRequest(request);
  route404.setRequest(request);

  if (route.isRoutable())
  {
    route.sipMessage.setProperty("enable-refer-retarget", "yes");
    route.routeRequest();
  }
  else
  {
    route404.routeRequest();
  }
}
