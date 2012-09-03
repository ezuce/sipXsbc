/* 
 * File:   SBCDataStore.h
 * Author: joegen
 *
 * Created on August 28, 2012, 10:19 AM
 */

#ifndef SBCDATASTORE_H
#define	SBCDATASTORE_H

#include "OSS/RedisClient.h"
#include "OSS/SIP/B2BUA/SIPB2BDialogStateManager.h"


namespace OSS {
namespace SIP {
namespace B2BUA {


class SBCDataStore
{
public:
  SBCDataStore(const std::string& redisHost, int tcpPort);

  void registerDataStoreFunctions(SIPB2BDialogStateManager& dialogState);

  bool persist(const DialogData& dialogData);
  void getAll(DialogList& dialogs);
  void removeSession(const std::string& sessionId);
  void removeAllDialogs(const std::string& callId);
  bool persistReg(const RegData& regData);
  bool getOneReg(const std::string& regId, RegData& regData);
  bool getReg(const std::string& regIdPrefix, RegList& regData);
  void removeReg(const std::string& regId);
  void removeAllReg(const std::string& regIdPrefix);
  void getAllReg(RegList& regs);

protected:
  RedisClient _dialogs;
  RedisClient _registry;
};

} } } // OSS::SIP::B2BUA


#endif	/* SBCDATASTORE_H */

