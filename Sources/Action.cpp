#include "Action.h"

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDLogger.h>

Action::Action(ESDConnectionManager* esd_connection, const std::string& context)
  : mESDConnection(esd_connection), mContext(context) {
}

Action::~Action() {
}

std::string Action::GetContext() const {
  return mContext;
}

void Action::DidReceiveSettings(const nlohmann::json& json) {
}

ESDConnectionManager* Action::GetESD() const {
  return mESDConnection;
}

void Action::WillAppear(const nlohmann::json& settings) {
}

void Action::KeyUp(const nlohmann::json& settings) {
}
