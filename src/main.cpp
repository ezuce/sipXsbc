#include "SBController.h"
#include "OSS/Application.h"
#include "OSS/ServiceOptions.h"
#include "OSS/DNS.h"
#include "OSS/Net.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef SIPX_CONFDIR
#define SIPX_CONFDIR "/etc/sipxpbx/"
#endif

#define EXTERNAL_IP_HOST_URL "myip.ossapp.com"


static void initDataStore(OSS::SIP::B2BUA::SBController& sbc, OSS::ServiceOptions& service)
{
  OSS::SIP::B2BUA::SBController::DataStoreConfig datastore;

  try
  {
    //
    // Check if the service config specified the redis configuration parameters
    //
    service.getOption("redis-address", datastore.redisHost, datastore.redisHost);
    service.getOption("redis-port", datastore.redisPort, datastore.redisPort);
    service.getOption("redis-password", datastore.redisPassword);

    if (datastore.redisHost.empty())
    {
      //
      // Check if redis-client.ini has the configuration
      //
      std::ostringstream sqaconfig;
      sqaconfig << SIPX_CONFDIR << "/" << "redis-client.ini";
      OSS::ServiceOptions configOptions(sqaconfig.str());
      if (configOptions.parseOptions())
      {
        configOptions.getOption("tcp-address", datastore.redisHost, datastore.redisHost);
        configOptions.getOption("tcp-port", datastore.redisPort, datastore.redisPort);
      }
    }
  }
  catch(...)
  {
  }

  if (!datastore.redisHost.empty())
  {
    sbc.initDataStore(datastore);
  }
}

static std::string getExternalIp(const std::string& host, const std::string& path)
{
  std::string ip;
  OSS::dns_host_record_list targets = OSS::dns_lookup_host(host);
  if (targets.empty())
  {
    OSS_LOG_ERROR("Unable to resolve HTTP server " << host);
    return ip;
  }


  OSS::socket_handle sock = OSS::socket_tcp_client_create();
  if (!sock)
    return ip;

  try
  {
    std::ostringstream strm;
    strm << "GET " << path << " HTTP/1.0" << "\r\n"
      << "Host: " << host << "\r\n"
      << "User-Agent: oss_b2bua" << "\r\n\r\n";
    OSS::socket_tcp_client_connect(sock, *targets.begin(), 80, 5000);
    if (!OSS::socket_tcp_client_send_bytes(sock, strm.str().c_str(), strm.str().length()))
    {
      OSS_LOG_ERROR("Unable to send to HTTP server at " << host);
    }

    std::string reply;

    while (true)
    {
      char buff[256];
      int len = OSS::socket_tcp_client_receive_bytes(sock, buff, 256);
      if (!len)
        break;
      reply += std::string(buff, len);
    }

    if (!reply.empty())
    {
      OSS::SIP::SIPMessage msg(reply);
      ip = msg.getBody();
    }

  }
  catch(std::exception& e)
  {
    OSS_LOG_ERROR("Unable to connect to HTTP server at " << host << " Error: " << e.what());
  }

  if (sock)
  {
    OSS::socket_tcp_client_shutdown(sock);
    OSS::socket_free(sock);
  }

  return ip;
}

static void initListeners(OSS::SIP::B2BUA::SBController& sbc, OSS::ServiceOptions& service)
{
  OSS::SIP::B2BUA::SBController::ListenerConfig transport;
  OSS::SIP::B2BUA::SBController::ListenerInfo tcp, udp, ws;

  std::string listenerAddress;
  std::string externalAddress;
  std::string wsListenerAddress;
  int listenerPort = 5060;
  int wsListenerPort = 5062;
  transport.tcpPortBase = 30000;
  transport.tcpPortMax = 40000;
  try
  {
    std::ostringstream config;
    if (!service.hasOption("config-file", false))
    {
      config << SIPX_CONFDIR << "/" << "sipxsbc.ini";
    }
    else
    {
      std::string configFile;
      service.getOption("config-file", configFile);
      config << configFile;
    }

    OSS::ServiceOptions configOptions(config.str());
    if (configOptions.parseOptions())
    {
      int portBase = transport.tcpPortBase;
      int portMax = transport.tcpPortMax;
      configOptions.getOption("target-domain", transport.domain);
      configOptions.getOption("external-address", externalAddress);
      configOptions.getOption("listener-address", listenerAddress);
      configOptions.getOption("ws-listener-address", wsListenerAddress);
      configOptions.getOption("listener-port", listenerPort, listenerPort);
      configOptions.getOption("ws-listener-port", wsListenerPort, wsListenerPort);
      configOptions.getOption("tcp-port-base", portBase, portBase);
      configOptions.getOption("tcp-port-max", portMax, portMax);
      transport.tcpPortBase = portBase;
      transport.tcpPortMax = portMax;
    }

    if (externalAddress.empty()&& configOptions.hasOption("guess-external-address"))
    {
      externalAddress = getExternalIp(EXTERNAL_IP_HOST_URL, "/");
    }
  }
  catch(...)
  {
    OSS_LOG_ERROR("Unable to configure SIP transports.");
    _exit(-1);
  }

  if (listenerAddress.empty())
  {
    OSS_LOG_ERROR("Unable to configure SIP transports.");
    _exit(-1);
  }

  udp.address = listenerAddress;
  udp.externalAddress = externalAddress;
  udp.port = listenerPort;
  udp.proto = "udp";
  tcp.address = listenerAddress;
  tcp.externalAddress = externalAddress;
  tcp.port = listenerPort;
  tcp.proto = "tcp";
  ws.address = wsListenerAddress;
  ws.externalAddress = externalAddress;
  ws.port = wsListenerPort;
  ws.proto = "ws";
  transport.listeners.push_back(udp);
  transport.listeners.push_back(tcp);
  transport.listeners.push_back(ws);

  sbc.initListeners(transport);
}

int main(int argc, char** argv)
{
  //
  // Daemonize the service if specified by the arguments
  //
  OSS::ServiceOptions::daemonize(argc, argv);
  //
  // Initialize global statics
  //
  OSS::OSS_init();
  //
  // Set the global exception handler
  //
  std::set_terminate(OSS::ServiceOptions::catch_global);
  //
  // Add configuration options and parse it
  //
  OSS::ServiceOptions service(argc, argv, "sipXsbc", VERSION, "Copyright (c) eZuce, Inc.");
  service.addDaemonOptions();
  service.addOptionString('i', "interface-address", "The IP Address where the B2BUA will listen for connections.");
  service.addOptionString('x', "external-address", "The Public IP Address if the B2BUA is behind a firewall.");
  service.addOptionFlag('X', "guess-external-address", "If this flag is set, the external IP will be automatically assigned.");
  service.addOptionInt('p', "listener-port", "The port where the B2BUA will listen for UDP/TCP connections.");
  service.addOptionInt('w', "ws-listener-port", "The port where the B2BUA will listen for web socket connections.");
  service.addOptionString('t', "target-address", "IP Address, Host or DNS/SRV address of your SIP Server.");
  service.addOptionFlag('r', "allow-relay", "Allow relaying of transactions towards SIP Servers other than the one specified in the target-domain.");
  service.addOptionString('S', "redis-address", "The IP address of redis where states will be stored.");
  service.addOptionString('s', "redis-port", "The port where redis listens for connections.");
  service.addOptionString('t', "redis-password", "Password to be used when connecting with redis.");
  service.addOptionFlag('n', "no-rtp-proxy", "Disable built in media relay.");
  service.parseOptions();
  //
  // Create the SBC instance
  //
  OSS::SIP::B2BUA::SBController sbc;
  //
  // Initialize the datastore
  //
  initDataStore(sbc, service);
  //
  // Initialize the transports
  //
  initListeners(sbc, service);
  //
  // Run the service
  //
  sbc.run();
  //
  // Block until a termination signal is received
  //
  OSS::app_wait_for_termination_request();
  //
  // Deinitialize global statics
  //
  OSS::OSS_deinit();
}
