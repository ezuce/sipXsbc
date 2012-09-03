
#include "SBCDataStore.h"

namespace OSS {
namespace SIP {
namespace B2BUA {

const int DIALOG_WORKSPACE = 11;
const int REGISTRY_WORKSPACE = 12;

SBCDataStore::SBCDataStore(const std::string& redisHost, int tcpPort) :
  _dialogs(redisHost, tcpPort),
  _registry(redisHost, tcpPort)
{
  _dialogs.connect("", DIALOG_WORKSPACE);
  _registry.connect("", REGISTRY_WORKSPACE);
}

void SBCDataStore::registerDataStoreFunctions(SIPB2BDialogStateManager& dialogState)
{
  dialogState.datastore().persist = boost::bind(&SBCDataStore::persist, this, _1);
  dialogState.datastore().getAll = boost::bind(&SBCDataStore::getAll, this, _1);
  dialogState.datastore().removeSession = boost::bind(&SBCDataStore::removeSession, this, _1);
  dialogState.datastore().removeAllDialogs = boost::bind(&SBCDataStore::removeAllDialogs, this, _1);
  dialogState.datastore().persistReg = boost::bind(&SBCDataStore::persistReg, this, _1);
  dialogState.datastore().getOneReg = boost::bind(&SBCDataStore::getOneReg, this, _1, _2);
  dialogState.datastore().getReg = boost::bind(&SBCDataStore::getReg, this, _1, _2);
  dialogState.datastore().removeReg = boost::bind(&SBCDataStore::removeReg, this, _1);
  dialogState.datastore().removeAllReg = boost::bind(&SBCDataStore::removeAllReg, this, _1);
  dialogState.datastore().getAllReg = boost::bind(&SBCDataStore::getAllReg, this, _1);
}

bool SBCDataStore::persist(const DialogData& dialogData)
{
  DialogList dialogs;
  json::Object data;
  dialogData.toJsonObject(data);
  return _dialogs.set(dialogData.sessionId, data, dialogData.expires);
}

void SBCDataStore::getAll(DialogList& dialogs)
{
  std::vector<json::Object> values;
  _dialogs.getAll(values);
  for (std::vector<json::Object>::const_iterator iter = values.begin(); iter != values.end(); iter++)
  {
    DialogData dialog;
    dialog.fromJsonObject(*iter);
    dialogs.push_back(dialog);
  }
}

void SBCDataStore::removeSession(const std::string& sessionId)
{
  _dialogs.del(sessionId);
}

void SBCDataStore::removeAllDialogs(const std::string& callId)
{
  std::vector<std::string> deleteThese;
  DialogList dialogs;
  getAll(dialogs);
  for (DialogList::const_iterator iter = dialogs.begin(); iter != dialogs.end(); iter++)
    if (iter->leg1.callId == callId)
      deleteThese.push_back(iter->sessionId);
  for (std::vector<std::string>::const_iterator iter = deleteThese.begin(); iter != deleteThese.end(); iter++)
    _dialogs.del(*iter);
}

bool SBCDataStore::persistReg(const RegData& regData)
{
  json::Object data;
  regData.toJsonObject(data);
  _registry.set(regData.key, data, regData.expires);
  return false;
}

bool SBCDataStore::getOneReg(const std::string& regId, RegData& regData)
{
  json::Object data;
  if (!_registry.get(regId, data))
    return false;
  regData.fromJsonObject(data);
  return true;
}

bool SBCDataStore::getReg(const std::string& regIdPrefix, RegList& regData)
{
  std::vector<json::Object > values;
  _registry.getAll(values, regIdPrefix);
  for (std::vector<json::Object>::const_iterator iter = values.begin(); iter != values.end(); iter++)
  {
    RegData data;
    data.fromJsonObject(*iter);
    regData.push_back(data);
  }
  return !regData.empty();
}

void SBCDataStore::removeReg(const std::string& regId)
{
  _registry.del(regId);
}

void SBCDataStore::removeAllReg(const std::string& regIdPrefix)
{
  std::vector<std::string> deleteThese;
  RegList regs;
  getReg(regIdPrefix, regs);

  for (RegList::const_iterator iter = regs.begin(); iter != regs.end(); iter++)
      deleteThese.push_back(iter->key);
  for (std::vector<std::string>::const_iterator iter = deleteThese.begin(); iter != deleteThese.end(); iter++)
    _registry.del(*iter);
}

void SBCDataStore::getAllReg(RegList& regs)
{
  std::vector<json::Object> values;
  _registry.getAll(values);
  for (std::vector<json::Object>::const_iterator iter = values.begin(); iter != values.end(); iter++)
  {
    RegData data;
    data.fromJsonObject(*iter);
    regs.push_back(data);
  }
}

} } } // OSS::SIP::B2BUA
