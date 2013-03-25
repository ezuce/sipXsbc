#include "SBController.h"
#include "SBCDataStore.h"
#include <v8.h>

#include "OSS/JS/JSBase.h"
#include "OSS/Core.h"
#include "OSS/Logger.h"
#include "OSS/IPAddress.h"
#include "OSS/SIP/SIPRequestLine.h"
#include "OSS/SIP/SIPStatusLine.h"
#include "OSS/SIP/SIPFrom.h"
#include "OSS/SIP/SIPURI.h"
#include "OSS/SIP/SIPContact.h"

#define B2BUA_MAX_THREAD 1024
#define B2BUA_MIN_THREAD 2

namespace OSS {
namespace SIP {
namespace B2BUA {


SBController::SBController() :
  SIPB2BTransactionManager(B2BUA_MIN_THREAD, B2BUA_MAX_THREAD),
  SIPB2BDialogStateManager(dynamic_cast<SIPB2BTransactionManager*>(this)),
  SIPB2BScriptableHandler(dynamic_cast<SIPB2BTransactionManager*>(this), dynamic_cast<SIPB2BDialogStateManager*>(this)),
  _pDataStore(0)
{
}

SBController::~SBController()
{
  if (_pDataStore)
  {
    delete _pDataStore;
    _pDataStore = 0;
  }
}

bool SBController::initDataStore(DataStoreConfig& config)
{
  OSS_ASSERT(!_pDataStore);
  _pDataStore = new SBCDataStore(config.redisHost, config.redisPort);
  _pDataStore->registerDataStoreFunctions(*this);
  //
  // Connect RTP Proxy to redis
  //
  std::vector<RedisClient::ConnectionInfo> connectionInfo;
  RedisClient::ConnectionInfo conn;
  conn.host = config.redisHost;
  conn.port = config.redisPort;
  connectionInfo.push_back(conn);
  dynamic_cast<SIPB2BScriptableHandler*>(this)->rtpProxy().redisConnect(connectionInfo);
  return true;
}

bool SBController::initListeners(ListenerConfig& config)
{
  try
  {
    bool hasDefault = false;
    for (ListenerInfoVector::iterator iter = config.listeners.begin(); iter != config.listeners.end(); iter++)
    {
      std::string proto = iter->proto;
      boost::to_lower(proto);

      if (stack().enableUDP() && proto == "udp")
      {
        OSS::IPAddress listener;
        listener = iter->address;
        listener.externalAddress() = iter->externalAddress;
        listener.setPort(iter->port);
        stack().udpListeners().push_back(listener);

        if (!hasDefault)
        {
          stack().transport().defaultListenerAddress() = listener;
          hasDefault = true;
        }
      }
      else if (stack().enableTCP() && proto == "tcp")
      {
        OSS::IPAddress listener;
        listener = iter->address;
        listener.externalAddress() = iter->externalAddress;
        listener.setPort(iter->port);
        stack().tcpListeners().push_back(listener);

        if (!hasDefault)
        {
          stack().transport().defaultListenerAddress() = listener;
          hasDefault = true;
        }
      }
    }

    _targetDomain = config.domain;
    stack().transport().setTCPPortRange(config.tcpPortBase, config.tcpPortMax);
    stack().transportInit();
  }
  catch(std::exception e)
  {
    OSS_LOG_ERROR("Unable to intialize transport.  Error: " << e.what());
    return false;
  }

  return true;
}

bool SBController::run()
{
  stack().run();
  dynamic_cast<SIPB2BDialogStateManager*>(this)->run();
  return true;
}

void SBController::setTargetDomain(const std::string& domain, const SIPMessage::Ptr& pRequest)
{
  //
  // Rewrite the target uri
  //
  SIPRequestLine requestLine = pRequest->getStartLine();
  SIPURI requestURI;
  requestLine.getURI(requestURI);
  requestURI.setHostPort(domain.c_str());
  requestLine.setURI(requestURI.data().c_str());
  pRequest->setStartLine(requestLine.data());
  //
  // Rewrite the to and from domains
  //
  SIPTo to = pRequest->hdrGet("to");
  SIPFrom from = pRequest->hdrGet("from");
  to.setHostPort(domain.c_str());
  from.setHostPort(domain.c_str());
  pRequest->hdrSet("From", from.data().c_str());
  pRequest->hdrSet("To", to.data().c_str());
}

bool SBController::onProcessRequest(MessageType type, const SIPMessage::Ptr& pRequest)
{
  setTargetDomain(_targetDomain, pRequest);
  pRequest->setProperty("route-action", "accept");
  pRequest->setProperty("auth-action", "accept");
  return true;
}

} } } // OSS::SIP::B2BUA