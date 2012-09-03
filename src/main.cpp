
#include "SBController.h"
#include "OSS/Application.h"

int main(int argc, char** argv)
{
  OSS::OSS_init();

  //
  // Initialize logger
  //
  std::string libLogger = "/home/joegen/Desktop/sipXsbc.log";
  std::string libLoggerPattern = "%h-%M-%S.%i: %t";
  OSS::LogPriority logPriority = OSS::PRIO_DEBUG;
  std::string libLoggerCompress =  "true";
  std::string libLoggerPurgeCount = "7";
  std::string libLoggerPriorityCache = "";

  OSS::logger_init(libLogger, logPriority, libLoggerPattern, libLoggerCompress, libLoggerPurgeCount);

  OSS::SIP::B2BUA::SBController sbc;

  //
  // Initialize the datastore
  //
  OSS::SIP::B2BUA::SBController::DataStoreConfig datastore;
  datastore.redisHost = "127.0.0.1";
  datastore.redisPort = 6379;
  sbc.initDataStore(datastore);

  //
  // Initialize java scripts
  //
  OSS::SIP::B2BUA::SBController::HandlerConfig handlers;
  handlers.globalScriptsDirectory = "/home/joegen/Desktop/js/globals";
  handlers.inboundRequestScript = "/home/joegen/Desktop/js/handlers/inboundRequest.js";
  handlers.outboundRequestScript = "/home/joegen/Desktop/js/handlers/outboundRequest.js";
  handlers.outboundResponseScript = "/home/joegen/Desktop/js/handlers/outboundResponse.js";
  handlers.routeRequestScript = "/home/joegen/Desktop/js/handlers/routeRequest.js";
  sbc.initHandler(handlers);

  //
  // Initialize the transports
  //
  OSS::SIP::B2BUA::SBController::ListenerConfig transport;
  OSS::SIP::B2BUA::SBController::ListenerInfo tcp, udp;
  udp.address = "192.168.1.10";
  udp.port = 5060;
  udp.proto = "udp";
  tcp.address = "192.168.1.10";
  tcp.port = 5060;
  tcp.proto = "tcp";
  transport.listeners.push_back(udp);
  transport.listeners.push_back(tcp);
  transport.tcpPortBase = 30000;
  transport.tcpPortMax = 40000;
  sbc.initListeners(transport);

  sbc.run();

  OSS::app_wait_for_termination_request();

  OSS::OSS_deinit();
}
