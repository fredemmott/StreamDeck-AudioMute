/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "BaseMuteAction.h"

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include "AudioDevices.h"
#include "DefaultAudioDevices.h"

using json = nlohmann::json;

void from_json(const json& json, MuteActionSettings& settings) {
  settings.deviceID = EPLJSONUtils::GetStringByName(
    json, "deviceID", DefaultAudioDevices::COMMUNICATIONS_INPUT_ID);
  settings.feedbackSounds
    = EPLJSONUtils::GetBoolByName(json, "feedbackSounds", true);
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
  j = json({{"id", device.id},
            {"interfaceName", device.interfaceName},
            {"endpointName", device.endpointName},
            {"displayName", device.displayName},
            {"state", device.state}});
}
}// namespace FredEmmott::Audio

BaseMuteAction::BaseMuteAction(
  ESDConnectionManager* esd_connection,
  const std::string& context)
  : ESDActionWithExternalState(esd_connection, context) {
}

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
  GetESD()->SendToPropertyInspector(
    GetActionID(), GetContext(),
    json{{"event", event},
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
  mDefaultChangeCallbackHandle = std::move(AddDefaultAudioDeviceChangeCallback(
    [this](
      AudioDeviceDirection direction, AudioDeviceRole role,
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
        mRealDeviceID, device);
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

void BaseMuteAction::RealDeviceDidChange() {
  const auto device(GetRealDeviceID());
  mMuteUnmuteCallbackHandle = std::move(AddAudioDeviceMuteUnmuteCallback(
    device, [this](bool isMuted) { this->MuteStateDidChange(isMuted); }));
  try {
    MuteStateDidChange(IsAudioDeviceMuted(device));
  } catch (device_error) {
    GetESD()->ShowAlertForContext(GetContext());
  }
}

void BaseMuteAction::KeyUp() {
  try {
    DoAction();
  } catch (FredEmmott::Audio::device_error e) {
    GetESD()->ShowAlertForContext(GetContext());
  }
}
