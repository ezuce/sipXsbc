/* 
 * File:   SBController.h
 * Author: joegen
 *
 * Created on August 28, 2012, 9:44 AM
 */

#ifndef SBCONTROLLER_H
#define	SBCONTROLLER_H

#include <v8.h>

#include "OSS/SIP/B2BUA/SIPB2BTransactionManager.h"
#include "OSS/SIP/B2BUA/SIPB2BScriptableHandler.h"
#include "OSS/SIP/B2BUA/SIPB2BDialogStateManager.h"


namespace OSS {
namespace SIP {
namespace B2BUA {

class SBCDataStore;

class SBController :
  public SIPB2BTransactionManager,
  public SIPB2BDialogStateManager,
  public SIPB2BScriptableHandler
{
public:
  struct ListenerInfo
  {
    std::string address;
    std::string externalAddress;
    unsigned short port;
    std::string proto;
    ListenerInfo() : port(5060){}
  };
  typedef std::vector<ListenerInfo> ListenerInfoVector;

  struct ListenerConfig
  {
    unsigned short tcpPortBase;
    unsigned short tcpPortMax;
    ListenerInfoVector listeners;
    ListenerConfig() : tcpPortBase(9000), tcpPortMax(30000) {}
  };

  struct HandlerConfig
  {
    std::string globalScriptsDirectory;
    std::string inboundRequestScript;
    std::string requestAuthenticatorScript;
    std::string routeRequestScript;
    std::string failoverRequestScript;
    std::string outboundRequestScript;
    std::string outboundResponseScript;
    std::string inboundRequestScriptHelper;
    std::string requestAuthenticatorScriptHelper;
    std::string routeRequestScriptHelper;
    std::string failoverRequestScriptHelper;
    std::string outboundRequestScriptHelper;
    std::string outboundResponseScriptHelper;
  };

  struct DataStoreConfig
  {
    std::string redisHost;
    int redisPort;
    DataStoreConfig() : redisHost("127.0.0.1"), redisPort(6379) {}
  };

  SBController();
  ~SBController();
  bool initListeners(ListenerConfig& config);
  bool initDataStore(DataStoreConfig& config);
  bool initHandler(HandlerConfig& config);
  bool run();
  //
  // Javascript exports
  //
  void setDefaultTargetDomain(const std::string& targetDomain);
protected:
  static OSS::SIP::SIPMessage* jsUnwrapRequest(const v8::Arguments& args);
  static std::string jsValToString(const v8::Handle<v8::Value>& str);
  static bool jsValToBoolean(const v8::Handle<v8::Value>& str);
  static int jsValToInt(const v8::Handle<v8::Value>& str);
  static v8::Handle<v8::Value> jsMsgSetTransactionProperty(const v8::Arguments& args);
  static v8::Handle<v8::Value> jsMsgGetTransactionProperty(const v8::Arguments& args);
  static v8::Handle<v8::Value> jsMsgGetSourceAddress(const v8::Arguments& args);
  static v8::Handle<v8::Value> jsMsgGetSourcePort(const v8::Arguments& args);
  static v8::Handle<v8::Value> jsMsgGetInterfaceAddress(const v8::Arguments& args);
  static v8::Handle<v8::Value> jsMsgGetInterfacePort(const v8::Arguments& args);
  static void jsRegisterExports(OSS_HANDLE objectTemplate);
  static v8::Handle<v8::Value> jsMsgGetDefaultTargetDomain(const v8::Arguments& args);
  static std::string _jsDefaultTargetDomain;
protected:
  SBCDataStore* _pDataStore;
};

} } } // OSS::SIP::B2BUA

#endif	/* SBCONTROLLER_H */

