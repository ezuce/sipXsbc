/* 
 * File:   SBController.h
 * Author: joegen
 *
 * Created on August 28, 2012, 9:44 AM
 */

#ifndef SBCONTROLLER_H
#define	SBCONTROLLER_H


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
    std::string domain;
    ListenerInfoVector listeners;
    ListenerConfig() : tcpPortBase(9000), tcpPortMax(30000) {}
  };

  struct DataStoreConfig
  {
    std::string redisHost;
    int redisPort;
    std::string redisPassword;
    DataStoreConfig() : redisHost(), redisPort(6379) {}
  };

  SBController();
  ~SBController();
  bool initListeners(ListenerConfig& config);
  bool initDataStore(DataStoreConfig& config);
  bool run();
  bool onProcessRequest(MessageType type, const SIPMessage::Ptr& pRequest);
protected:
  void setTargetDomain(const std::string& domain, const SIPMessage::Ptr& pRequest);
  SBCDataStore* _pDataStore;
  std::string _targetDomain;
};

} } } // OSS::SIP::B2BUA

#endif	/* SBCONTROLLER_H */

