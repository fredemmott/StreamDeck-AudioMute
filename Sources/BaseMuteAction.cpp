/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "BaseMuteAction.h"

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDLogger.h>

#include "DefaultAudioDevices.h"

using json = nlohmann::json;

void from_json(const json& json, MuteActionSettings& settings) {
  const auto default_id = DefaultAudioDevices::GetRealDeviceID(
                            DefaultAudioDevices::COMMUNICATIONS_INPUT_ID)
                            .empty()
    ? DefaultAudioDevices::DEFAULT_INPUT_ID
    : DefaultAudioDevices::COMMUNICATIONS_INPUT_ID;
  settings.deviceID = json.value<std::string>("deviceID", default_id);
  settings.feedbackSounds = json.value<bool>("feedbackSounds", true);
  settings.ptt = json.value<bool>("ptt", false);
}

namespace FredEmmott::Audio {

void to_json(json& j, const AudioDeviceState& state) {
  switch (state) {
    case AudioDeviceState::CONNECTED:
      j = "connected";
      return;
    case AudioDeviceState::DEVICE_NOT_PRESENT:
      j = "device_not_present";
      return;
    case AudioDeviceState::DEVICE_DISABLED:
      j = "device_disabled";
      return;
    case AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION:
      j = "device_present_no_connection";
      return;
  }
}

void to_json(json& j, const AudioDeviceInfo& device) {
  j = json(
    {{"id", device.id},
     {"interfaceName", device.interfaceName},
     {"endpointName", device.endpointName},
     {"displayName", device.displayName},
     {"state", device.state}});
}
}// namespace FredEmmott::Audio

BaseMuteAction::~BaseMuteAction() {
}

std::string BaseMuteAction::GetRealDeviceID() const {
  return mRealDeviceID;
}

void BaseMuteAction::SendToPlugin(const nlohmann::json& payload) {
  const auto event = EPLJSONUtils::GetStringByName(payload, "event");

  if (event != "getDeviceList") {
    return;
  }
  SendToPropertyInspector(json {
    {"event", event},
    {"outputDevices", GetAudioDeviceList(AudioDeviceDirection::OUTPUT)},
    {"inputDevices", GetAudioDeviceList(AudioDeviceDirection::INPUT)},
    {"defaultDevices",
     {{DefaultAudioDevices::DEFAULT_INPUT_ID,
       DefaultAudioDevices::GetRealDeviceID(
         DefaultAudioDevices::DEFAULT_INPUT_ID)},
      {DefaultAudioDevices::DEFAULT_OUTPUT_ID,
       DefaultAudioDevices::GetRealDeviceID(
         DefaultAudioDevices::DEFAULT_OUTPUT_ID)},
      {DefaultAudioDevices::COMMUNICATIONS_INPUT_ID,
       DefaultAudioDevices::GetRealDeviceID(
         DefaultAudioDevices::COMMUNICATIONS_INPUT_ID)},
      {DefaultAudioDevices::COMMUNICATIONS_OUTPUT_ID,
       DefaultAudioDevices::GetRealDeviceID(
         DefaultAudioDevices::COMMUNICATIONS_OUTPUT_ID)}}}});
}

void BaseMuteAction::SettingsDidChange(
  const MuteActionSettings& old_settings,
  const MuteActionSettings& new_settings) {
  mFeedbackSounds = new_settings.feedbackSounds;
  if (old_settings.deviceID == new_settings.deviceID) {
    return;
  }
  mRealDeviceID = DefaultAudioDevices::GetRealDeviceID(new_settings.deviceID);
  RealDeviceDidChange();
  if (mRealDeviceID == new_settings.deviceID) {
    return;
  }

  // Differing real device ID means we have a place holder device, e.g.
  // 'Default input device'

  ESDDebug("Registering default device change callback for {}", GetContext());
  mDefaultChangeCallbackHandle = std::move(
    AddDefaultAudioDeviceChangeCallback([this](
                                          AudioDeviceDirection direction,
                                          AudioDeviceRole role,
                                          const std::string& device) {
      ESDDebug("In default device change callback for {}", GetContext());
      if (
        DefaultAudioDevices::GetSpecialDeviceID(direction, role)
        != GetSettings().deviceID) {
        ESDDebug("Not this device");
        return;
      }
      ESDLog(
        "Special device change: old device: '{}' - new device: '{}'",
        mRealDeviceID,
        device);
      if (device == mRealDeviceID) {
        ESDLog("Default default change for context {} didn't actually change");
        return;
      }
      mRealDeviceID = device;
      ESDLog(
        "Invoking RealDeviceDidChange from callback for context {}",
        GetContext());
      RealDeviceDidChange();
    }));
}

bool BaseMuteAction::FeedbackSoundsEnabled() const {
  return mFeedbackSounds;
}

void BaseMuteAction::RealDeviceDidChange() {
  const auto device(GetRealDeviceID());
  mMuteUnmuteCallbackHandle = std::move(AddAudioDeviceMuteUnmuteCallback(
    device, [this](bool isMuted) { this->MuteStateDidChange(isMuted); }));
  try {
    MuteStateDidChange(IsAudioDeviceMuted(device));
  } catch (const device_error& e) {
    ESDDebug("Error on real device change: {}", e.what());
    ShowAlert();
  }
}

void BaseMuteAction::KeyUp() {
  try {
    DoAction();
    if (GetAudioDeviceState(GetRealDeviceID()) != AudioDeviceState::CONNECTED) {
      // This is actually fine on Windows, but show an alert anyway to make it clear
      // that something weird happened - don't want the user to have a hot mic without
      // realizing it
      ESDDebug("Acted on a device that isn't present");
      ShowAlert();
    }
  } catch (const device_error& e) {
    ESDDebug("Error on keyup: {}", e.what());
    ShowAlert();
  }
}
