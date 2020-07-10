#include "Action.h"

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDLogger.h>

#include "AudioFunctions.h"
#include "DefaultAudioDevices.h"

using json = nlohmann::json;

void from_json(const json& json, Action::Settings& settings) {
  settings.deviceID = EPLJSONUtils::GetStringByName(
    json, "deviceID", DefaultAudioDevices::COMMUNICATIONS_INPUT_ID);
  settings.feedbackSounds
    = EPLJSONUtils::GetBoolByName(json, "feedbackSounds", true);
}

Action::Action(ESDConnectionManager* esd_connection, const std::string& context)
  : mESDConnection(esd_connection), mContext(context) {
}

Action::~Action() {
}

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
    if (mRealDeviceID != mSettings.deviceID) {
      ESDDebug(
        "Registering default device change callback for {}", GetContext());
      mDefaultChangeCallbackHandle
        = std::move(AddDefaultAudioDeviceChangeCallback(
          [this](
            AudioDeviceDirection direction, AudioDeviceRole role,
            const std::string& device) {
            ESDDebug("In default device change callback for {}", GetContext());
            if (
              DefaultAudioDevices::GetSpecialDeviceID(direction, role)
              != mSettings.deviceID) {
              ESDDebug("Not this device");
              return;
            }
            ESDLog(
              "Special device change: old device: '{}' - new device: '{}'",
              mRealDeviceID, device);
            if (device == mRealDeviceID) {
              ESDLog(
                "Default default change for context {} didn't actually change");
              return;
            }
            mRealDeviceID = device;
            ESDLog(
              "Invoking RealDeviceDidChange from callback for context {}",
              GetContext());
            RealDeviceDidChange();
          }));
    }
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
  const auto device(GetRealDeviceID());
  mMuteUnmuteCallbackHandle = std::move(AddAudioDeviceMuteUnmuteCallback(
    device, [this](bool isMuted) { this->MuteStateDidChange(isMuted); }));
  MuteStateDidChange(IsAudioDeviceMuted(device));
}
