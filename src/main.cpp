
#include <boost/program_options.hpp>

#include "SBController.h"
#include "OSS/Application.h"

#define SERVICE_NO_LOGGER 1
#include "sqa/ServiceOptions.h"

#ifndef SIPX_CONFDIR
#define SIPX_CONFDIR "/etc/sipxpbx/"
#endif

static void catch_global()
{
#define catch_global_print(msg)  \
  std::ostringstream bt; \
  bt << msg << std::endl; \
  void* trace_elems[20]; \
  int trace_elem_count(backtrace( trace_elems, 20 )); \
  char** stack_syms(backtrace_symbols(trace_elems, trace_elem_count)); \
  for (int i = 0 ; i < trace_elem_count ; ++i ) \
    bt << stack_syms[i] << std::endl; \
  OSS_LOG_CRITICAL(bt.str().c_str()); \
  std::cerr << bt.str().c_str(); \
  free(stack_syms);

  try
  {
      throw;
  }
  catch (std::string& e)
  {
    catch_global_print(e.c_str());
  }
#ifdef MONGO_assert
  catch (mongo::DBException& e)
  {
    catch_global_print(e.toString().c_str());
  }
#endif
  catch (boost::exception& e)
  {
    catch_global_print(diagnostic_information(e).c_str());
  }
  catch (std::exception& e)
  {
    catch_global_print(e.what());
  }
  catch (...)
  {
    catch_global_print("Error occurred. Unknown exception type.");
  }

  std::abort();
}

static void initLogger(ServiceOptions& service)
{
  std::string logFile;
  int priorityLevel = OSS::PRIO_NOTICE;
  bool compress = true;
  int purgeCount = 7;
  std::string pattern = "%h-%M-%S.%i: %t";
  if (service.hasOption("log-file", true))
  {
    if (service.getOption("log-file", logFile) && !logFile.empty())
    {
      if (service.hasOption("log-level", true))
        service.getOption("log-level", priorityLevel, priorityLevel);

      if (service.hasOption("log-no-compress", true))
        compress = false;

      if (service.hasOption("log-purge-count"))
        service.getOption("log-purge-count", purgeCount, purgeCount);

      OSS::logger_init(logFile, (OSS::LogPriority)priorityLevel, pattern, compress ? "true" : "false", boost::lexical_cast<std::string>(purgeCount));
      std::set_terminate(catch_global);
    }
  }
  else
  {
    service.displayUsage(std::cerr);
    std::cerr << std::endl << "ERROR: Log file not specified!" << std::endl;
    std::cerr.flush();
    _exit(-1);
  }
}

static void initDataStore(OSS::SIP::B2BUA::SBController& sbc)
{
  OSS::SIP::B2BUA::SBController::DataStoreConfig datastore;
  try
  {
    std::ostringstream sqaconfig;
    sqaconfig << SIPX_CONFDIR << "/" << "redis-client.ini";
    ServiceOptions configOptions(sqaconfig.str());
    if (configOptions.parseOptions())
    {
      configOptions.getOption("tcp-address", datastore.redisHost, datastore.redisHost);
      configOptions.getOption("tcp-port", datastore.redisPort, datastore.redisPort);
    }
  }
  catch(...)
  {
  }
  sbc.initDataStore(datastore);
}

static void initHandler(OSS::SIP::B2BUA::SBController& sbc, ServiceOptions& service)
{
  OSS::SIP::B2BUA::SBController::HandlerConfig handlers;
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

    ServiceOptions configOptions(config.str());
    if (configOptions.parseOptions())
    {
      configOptions.getOption("globalScriptsDirectory", handlers.globalScriptsDirectory, handlers.globalScriptsDirectory);
      configOptions.getOption("inboundRequestScript", handlers.inboundRequestScript, handlers.inboundRequestScript);
      configOptions.getOption("outboundRequestScript", handlers.outboundRequestScript, handlers.outboundRequestScript);
      configOptions.getOption("outboundResponseScript", handlers.outboundResponseScript, handlers.outboundResponseScript);
      configOptions.getOption("routeRequestScript", handlers.routeRequestScript, handlers.routeRequestScript);

      std::string targetDomain;
      configOptions.getOption("target-domain", targetDomain);
      sbc.setDefaultTargetDomain(targetDomain);
    }
  }
  catch(...)
  {
  }

  sbc.initHandler(handlers);
}

static void initListeners(OSS::SIP::B2BUA::SBController& sbc, ServiceOptions& service)
{
  OSS::SIP::B2BUA::SBController::ListenerConfig transport;
  OSS::SIP::B2BUA::SBController::ListenerInfo tcp, udp;

  std::string listenerAddress;
  std::string externalAddress;
  int listenerPort = 5060;
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

    ServiceOptions configOptions(config.str());
    if (configOptions.parseOptions())
    {
      int portBase = transport.tcpPortBase;
      int portMax = transport.tcpPortMax;
      configOptions.getOption("external-address", externalAddress);
      configOptions.getOption("listener-address", listenerAddress);
      configOptions.getOption("listener-port", listenerPort, listenerPort);
      configOptions.getOption("tcp-port-base", portBase, portBase);
      configOptions.getOption("tcp-port-max", portMax, portMax);
      transport.tcpPortBase = portBase;
      transport.tcpPortMax = portMax;
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
  transport.listeners.push_back(udp);
  transport.listeners.push_back(tcp);

  sbc.initListeners(transport);
}

int main(int argc, char** argv)
{
  OSS::OSS_init();

  ServiceOptions::daemonize(argc, argv);
  ServiceOptions service(argc, argv, "sipXsbc", "1.0.0", "(c) eZuce, Inc. All rights reserved.");
  service.addDaemonOptions();

  //
  // Initialize the logger
  //
  service.addOptionString('L', "log-file", ": Specify the application log file.", ServiceOptions::CommandLineOption);
  service.addOptionInt('l', "log-level",
      ": Specify the application log priority level."
      "Valid level is between 0-7.  "
      "0 (EMERG) 1 (ALERT) 2 (CRIT) 3 (ERR) 4 (WARNING) 5 (NOTICE) 6 (INFO) 7 (DEBUG)"
            , ServiceOptions::CommandLineOption);
  service.addOptionFlag("log-no-compress", ": Specify if logs will be compressed after rotation.");
  service.addOptionInt("log-purge-count", ": Specify the number of archive to maintain.");
  service.parseOptions();
  initLogger(service);


  OSS::SIP::B2BUA::SBController sbc;
  //
  // Initialize the datastore
  //
  initDataStore(sbc);
  //
  // Initialize java scripts
  //
  initHandler(sbc, service);
  //
  // Initialize the transports
  //
  initListeners(sbc, service);

  sbc.run();

  OSS::app_wait_for_termination_request();

  OSS::OSS_deinit();
}
