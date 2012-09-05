function handle_request(request)
{
  var sipMessage = new SIPMessage(request);
  sipMessage.hdrRemove("Accept-Language");
  sipMessage.hdrRemove("Remote-Party-Id");
}