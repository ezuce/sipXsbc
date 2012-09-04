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


namespace OSS {
namespace SIP {
namespace B2BUA {

std::string SBController::_jsDefaultTargetDomain;

SBController::SBController() :
  SIPB2BTransactionManager(2, 1024),
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
}

bool SBController::initHandler(HandlerConfig& config)
{
  if (!config.inboundRequestScript.empty() && !loadInboundScript(
          config.inboundRequestScript,
          &SBController::jsRegisterExports,
          config.globalScriptsDirectory,
          config.inboundRequestScriptHelper))
  {
    OSS_LOG_ERROR("Unable to load inbound script " << config.inboundRequestScript);
    return false;
  }

  if (!config.requestAuthenticatorScript.empty() && !loadAuthScript(
          config.requestAuthenticatorScript,
          &SBController::jsRegisterExports,
          config.globalScriptsDirectory,
          config.requestAuthenticatorScriptHelper))
  {
    OSS_LOG_ERROR("Unable to load authenticator script " << config.requestAuthenticatorScript);
    return false;
  }

  if (!config.routeRequestScript.empty() && !loadRouteScript(
          config.routeRequestScript,
          &SBController::jsRegisterExports,
          config.globalScriptsDirectory,
          config.routeRequestScriptHelper))
  {
    OSS_LOG_ERROR("Unable to load route script " << config.routeRequestScript);
    return false;
  }

  if (!config.failoverRequestScript.empty() && !loadRouteFailoverScript(
          config.failoverRequestScript,
          &SBController::jsRegisterExports,
          config.globalScriptsDirectory,
          config.failoverRequestScriptHelper))
  {
    OSS_LOG_ERROR("Unable to load failover script " << config.failoverRequestScript);
    return false;
  }

  if (!config.outboundRequestScript.empty() && !loadOutboundScript(
          config.outboundRequestScript,
          &SBController::jsRegisterExports,
          config.globalScriptsDirectory,
          config.outboundRequestScriptHelper))
  {
    OSS_LOG_ERROR("Unable to load outbound script " << config.outboundRequestScript);
    return false;
  }

  if (!config.outboundResponseScript.empty() && !loadOutboundResponseScript(
          config.outboundResponseScript,
          &SBController::jsRegisterExports,
          config.globalScriptsDirectory,
          config.outboundResponseScriptHelper))
  {
    OSS_LOG_ERROR("Unable to load response script " << config.outboundResponseScript);
    return false;
  }

  registerDefaultHandler(dynamic_cast<SIPB2BHandler*>(this));
}



OSS::SIP::SIPMessage* SBController::jsUnwrapRequest(const v8::Arguments& args)
{
  if (args.Length() < 1)
    return 0;
  v8::Handle<v8::Value> obj = args[0];
  if (!obj->IsObject())
    return 0;
  v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(obj->ToObject()->GetInternalField(0));
  void* ptr = field->Value();
  return static_cast<OSS::SIP::SIPMessage*>(ptr);
}

std::string SBController::jsValToString(const v8::Handle<v8::Value>& str)
{
  if (!str->IsString())
    return "";
  v8::String::Utf8Value value(str);
  return *value;
}

bool SBController::jsValToBoolean(const v8::Handle<v8::Value>& str)
{
  if (!str->IsBoolean())
    return false;
  return str->IsTrue();;
}

int SBController::jsValToInt(const v8::Handle<v8::Value>& str)
{
  if (!str->IsNumber())
    return 0;
  return str->Int32Value();
}

v8::Handle<v8::Value> SBController::jsMsgSetTransactionProperty(const v8::Arguments& args)
{
  if (args.Length() < 3)
    return v8::Boolean::New(false);

  v8::HandleScope scope;
  OSS::SIP::SIPMessage* pMsg = jsUnwrapRequest(args);
  if (!pMsg)
    return v8::Boolean::New(false);

  OSS::SIP::B2BUA::SIPB2BTransaction* pTrn = static_cast<OSS::SIP::B2BUA::SIPB2BTransaction*>(pMsg->userData());
  if (!pTrn)
    return v8::Boolean::New(false);

  std::string name = jsValToString(args[1]);
  std::string value = jsValToString(args[2]);

  if (name.empty() || value.empty())
    return v8::Boolean::New(false);

  pTrn->setProperty(name, value);

  return v8::Boolean::New(true);
}

v8::Handle<v8::Value> SBController::jsMsgGetTransactionProperty(const v8::Arguments& args)
{
  if (args.Length() < 2)
    return v8::Undefined();

  v8::HandleScope scope;
  OSS::SIP::SIPMessage* pMsg = jsUnwrapRequest(args);
  if (!pMsg)
    return v8::Undefined();

  OSS::SIP::B2BUA::SIPB2BTransaction* pTrn = static_cast<OSS::SIP::B2BUA::SIPB2BTransaction*>(pMsg->userData());
  if (!pTrn)
    return v8::Undefined();

  std::string name = jsValToString(args[1]);

  if (name.empty())
    return v8::Undefined();
  std::string value;
  if (name == "log-id")
    value = pTrn->getLogId();
  else
    pTrn->getProperty(name, value);

  return v8::String::New(value.c_str());
}

v8::Handle<v8::Value> SBController::jsMsgGetSourceAddress(const v8::Arguments& args)
{
  if (args.Length() < 1)
    return v8::Undefined();

  v8::HandleScope scope;
  OSS::SIP::SIPMessage* pMsg = jsUnwrapRequest(args);
  if (!pMsg)
    return v8::Undefined();

  OSS::SIP::B2BUA::SIPB2BTransaction* pTrn = static_cast<OSS::SIP::B2BUA::SIPB2BTransaction*>(pMsg->userData());
  if (!pTrn || !pTrn->serverTransport())
    return v8::Undefined();

  OSS::IPAddress addr = pTrn->serverTransport()->getRemoteAddress();

  return v8::String::New(addr.toString().c_str());
}

v8::Handle<v8::Value> SBController::jsMsgGetSourcePort(const v8::Arguments& args)
{
  if (args.Length() < 1)
    return v8::Undefined();

  v8::HandleScope scope;
  OSS::SIP::SIPMessage* pMsg = jsUnwrapRequest(args);
  if (!pMsg)
    return v8::Undefined();

  OSS::SIP::B2BUA::SIPB2BTransaction* pTrn = static_cast<OSS::SIP::B2BUA::SIPB2BTransaction*>(pMsg->userData());
  if (!pTrn || !pTrn->serverTransport())
    return v8::Undefined();

  OSS::IPAddress addr = pTrn->serverTransport()->getRemoteAddress();

  return v8::Integer::New(addr.getPort());
}

v8::Handle<v8::Value> SBController::jsMsgGetInterfaceAddress(const v8::Arguments& args)
{
  if (args.Length() < 1)
    return v8::Undefined();

  v8::HandleScope scope;
  OSS::SIP::SIPMessage* pMsg = jsUnwrapRequest(args);
  if (!pMsg)
    return v8::Undefined();

  OSS::SIP::B2BUA::SIPB2BTransaction* pTrn = static_cast<OSS::SIP::B2BUA::SIPB2BTransaction*>(pMsg->userData());
  if (!pTrn || !pTrn->serverTransport())
    return v8::Undefined();

  OSS::IPAddress addr = pTrn->serverTransport()->getLocalAddress();
  return v8::String::New(addr.toString().c_str());
}

v8::Handle<v8::Value> SBController::jsMsgGetInterfacePort(const v8::Arguments& args)
{
  if (args.Length() < 1)
    return v8::Undefined();

  v8::HandleScope scope;
  OSS::SIP::SIPMessage* pMsg = jsUnwrapRequest(args);
  if (!pMsg)
    return v8::Undefined();

  OSS::SIP::B2BUA::SIPB2BTransaction* pTrn = static_cast<OSS::SIP::B2BUA::SIPB2BTransaction*>(pMsg->userData());
  if (!pTrn || !pTrn->serverTransport())
    return v8::Undefined();

  OSS::IPAddress addr = pTrn->serverTransport()->getLocalAddress();

  return v8::Integer::New(addr.getPort());
}

void SBController::setDefaultTargetDomain(const std::string& targetDomain)
{
  SBController::_jsDefaultTargetDomain = targetDomain;
}

v8::Handle<v8::Value> SBController::jsMsgGetDefaultTargetDomain(const v8::Arguments& args)
{
  if(SBController::_jsDefaultTargetDomain.empty())
    return v8::Undefined();
  return v8::String::New(SBController::_jsDefaultTargetDomain.c_str());
}

void SBController::jsRegisterExports(OSS_HANDLE objectTemplate)
{
  v8::Handle<v8::ObjectTemplate>& global = *(static_cast<v8::Handle<v8::ObjectTemplate>*>(objectTemplate));
  global->Set(v8::String::New("msgSetTransactionProperty"), v8::FunctionTemplate::New(&SBController::jsMsgSetTransactionProperty));
  global->Set(v8::String::New("msgGetTransactionProperty"), v8::FunctionTemplate::New(&SBController::jsMsgGetTransactionProperty));
  global->Set(v8::String::New("msgGetSourceAddress"), v8::FunctionTemplate::New(&SBController::jsMsgGetSourceAddress));
  global->Set(v8::String::New("msgGetSourcePort"), v8::FunctionTemplate::New(&SBController::jsMsgGetSourcePort));
  global->Set(v8::String::New("msgGetInterfaceAddress"), v8::FunctionTemplate::New(&SBController::jsMsgGetInterfaceAddress));
  global->Set(v8::String::New("msgGetInterfacePort"), v8::FunctionTemplate::New(&SBController::jsMsgGetInterfacePort));
  global->Set(v8::String::New("msgGetDefaultTargetDomain"), v8::FunctionTemplate::New(&SBController::jsMsgGetDefaultTargetDomain));
}

} } } // OSS::SIP::B2BUA