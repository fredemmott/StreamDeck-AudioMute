#include "Action.h"

#include "AudioFunctions.h"
#include "DefaultAudioDevices.h"

#include <StreamDeckSDK/EPLJSONUtils.h>

using json = nlohmann::json;

void from_json(const json& json, Action::Settings& settings) {
  settings.deviceID = EPLJSONUtils::GetStringByName(
    json, "deviceID", DefaultAudioDevices::COMMUNICATIONS_INPUT_ID);
  settings.feedbackSounds
    = EPLJSONUtils::GetBoolByName(json, "feedbackSounds", true);
}

Action::Action(
  ESDConnectionManager* esd_connection,
  const std::string& context)
  : mESDConnection(esd_connection), mContext(context) {
}

Action::~Action() {
  if (mMuteUnmuteCallbackHandle) {
    RemoveAudioDeviceMuteUnmuteCallback(mMuteUnmuteCallbackHandle);
  }
};

const Action::Settings& Action::GetSettings() const {
  return mSettings;
}

std::string Action::GetContext() const {
  return mContext;
}
std::string Action::GetRealDeviceID() const {
  return mRealDeviceID;
}

void Action::DidReceiveSettings(const nlohmann::json& json) {
  const Settings settings(json);
  if (mSettings == settings) {
    return;
  }
  const bool deviceDidChange = settings.deviceID != mSettings.deviceID;
  this->mSettings = std::move(settings);
  if (deviceDidChange) {
    mRealDeviceID = DefaultAudioDevices::GetRealDeviceID(mSettings.deviceID);
    RealDeviceDidChange();
  }
}

ESDConnectionManager* Action::GetESD() const {
  return mESDConnection;
}

void Action::WillAppear(const nlohmann::json& settings) {
  DidReceiveSettings(settings);
  WillAppear();
}

void Action::KeyUp(const nlohmann::json& settings) {
  DidReceiveSettings(settings);
  KeyUp();
}

void Action::RealDeviceDidChange() {
  if (mMuteUnmuteCallbackHandle) {
    RemoveAudioDeviceMuteUnmuteCallback(mMuteUnmuteCallbackHandle);
  }
  const auto device(GetRealDeviceID());
  mMuteUnmuteCallbackHandle = AddAudioDeviceMuteUnmuteCallback(
    device, [this](bool isMuted) { this->MuteStateDidChange(isMuted); });
  MuteStateDidChange(IsAudioDeviceMuted(device));
}
