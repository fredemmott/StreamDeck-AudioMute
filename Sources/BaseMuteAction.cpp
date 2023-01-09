/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "BaseMuteAction.h"

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDLogger.h>

#include <functional>

#include "DefaultAudioDevices.h"
#include "audio_json.h"

using json = nlohmann::json;

NLOHMANN_JSON_SERIALIZE_ENUM(
  DeviceMatchStrategy,
  {
    {DeviceMatchStrategy::ID, "ID"},
    {DeviceMatchStrategy::Fuzzy, "Fuzzy"},
    {DeviceMatchStrategy::Special, "Special"},
  });

void from_json(const json& json, MuteActionSettings& settings) {
  if (json.contains("device")) {
    settings.device = json.at("device");
    settings.matchStrategy = json.at("matchStrategy");
  } else if (json.contains("deviceID")) {
    const auto id = json.at("deviceID").get<std::string>();
    settings.device = {.id = id};
    if (DefaultAudioDevices::GetRealDeviceID(id) == id) {
      settings.matchStrategy = DeviceMatchStrategy::ID;
    } else {
      settings.matchStrategy = DeviceMatchStrategy::Special;
    }
  } else {
    settings.device = {
      .id = DefaultAudioDevices::GetRealDeviceID(
              DefaultAudioDevices::COMMUNICATIONS_INPUT_ID)
              .empty()
        ? DefaultAudioDevices::DEFAULT_INPUT_ID
        : DefaultAudioDevices::COMMUNICATIONS_INPUT_ID,
    };
    settings.matchStrategy = DeviceMatchStrategy::Special;
  };

  settings.feedbackSounds = json.value<bool>("feedbackSounds", true);
  settings.ptt = json.value<bool>("ptt", false);
}

std::string MuteActionSettings::VolatileDeviceID() const {
  if (matchStrategy == DeviceMatchStrategy::Special) {
    return DefaultAudioDevices::GetRealDeviceID(device.id);
  }

  if (matchStrategy == DeviceMatchStrategy::ID) {
    return device.id;
  }

  if (GetAudioDeviceState(device.id) == AudioDeviceState::CONNECTED) {
    return device.id;
  }

  for (const auto& [otherID, other]: GetAudioDeviceList(device.direction)) {
    if (
      device.interfaceName == other.interfaceName
      && device.endpointName == other.endpointName) {
      ESDDebug(
        "Fuzzy device match for {}/{}",
        device.interfaceName,
        device.endpointName);
      return otherID;
    }
  }
  ESDLog(
    "Failed fuzzy match for {}/{}", device.interfaceName, device.endpointName);
  return device.id;
}

BaseMuteAction::~BaseMuteAction() {
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
  if (old_settings.device == new_settings.device) {
    return;
  }
  RealDeviceDidChange();
  if (new_settings.matchStrategy == DeviceMatchStrategy::ID) {
    mDefaultChangeCallbackHandle = {};
    ESDDebug("Registering plugevent callback");
    mPlugEventCallbackHandle = AddAudioDevicePlugEventCallback(
      std::bind_front(&BaseMuteAction::OnPlugEvent, this));
    return;
  }

  // Differing real device ID means we have a place holder device, e.g.
  // 'Default input device'

  mPlugEventCallbackHandle = {};
  ESDDebug("Registering default device change callback for {}", GetContext());
  mDefaultChangeCallbackHandle = AddDefaultAudioDeviceChangeCallback(
    std::bind_front(&BaseMuteAction::OnDefaultDeviceChange, this));
}

void BaseMuteAction::OnPlugEvent(
  AudioDevicePlugEvent event,
  const std::string& deviceID) {
  ESDLog(
    "Received plug event {} for device {}", static_cast<int>(event), deviceID);
  if (deviceID != GetSettings().device.id) {
    ESDLog("Device is not a match");
    return;
  }

  switch (event) {
    case AudioDevicePlugEvent::ADDED:
      ESDLog("Matching device added");
      // Windows will now preserve/enforce the state if changed while the
      // device is unplugged, but MacOS won't, so we need to update the
      // displayed state to match reality
      this->MuteStateDidChange(IsAudioDeviceMuted(deviceID));
      ShowOK();
      return;
    case AudioDevicePlugEvent::REMOVED:
      ESDLog("Matching device removed");
      ShowAlert();
      return;
  }
}

void BaseMuteAction::OnDefaultDeviceChange(
  AudioDeviceDirection direction,
  AudioDeviceRole role,
  const std::string& newDevice) {
  ESDDebug("In default device change callback for {}", GetContext());
  if (
    GetSettings().device.id
    != DefaultAudioDevices::GetSpecialDeviceID(direction, role)) {
    ESDDebug("Not this device");
    return;
  }

  const auto oldDevice = GetRealDeviceID();
  ESDLog(
    "Special device change: old device: '{}' - new device: '{}'",
    oldDevice,
    newDevice);
  if (oldDevice == newDevice) {
    ESDLog("Default default change for context {} didn't actually change");
    return;
  }
  ESDLog(
    "Invoking RealDeviceDidChange from callback for context {}", GetContext());
  RealDeviceDidChange();
}

bool BaseMuteAction::FeedbackSoundsEnabled() const {
  return mFeedbackSounds;
}

void BaseMuteAction::RealDeviceDidChange() {
  const auto device = GetRealDeviceID();
  mMuteUnmuteCallbackHandle
    = AddAudioDeviceMuteUnmuteCallback(
        device,
        std::bind_front(&BaseMuteAction::OnMuteStateChanged, this, device))
        .value_or(nullptr);
}

void BaseMuteAction::OnMuteStateChanged(
  const std::string& device,
  bool isMuted) {
  auto state = IsAudioDeviceMuted(device);
  if (!state) {
    ESDDebug(
      "Error on real device change: {}", static_cast<int>(state.error()));
    ShowAlert();
    return;
  }
  MuteStateDidChange(*state);
}

bool BaseMuteAction::IsDevicePresent() const {
  return GetAudioDeviceState(GetRealDeviceID()) == AudioDeviceState::CONNECTED;
}

void BaseMuteAction::KeyUp() {
  DoAction();
  // This is actually fine on Windows, but show an alert anyway to make it
  // clear that something weird happened - don't want the user to have a hot
  // mic without realizing it
  if (!IsDevicePresent()) {
    ESDDebug("Acted on a device that isn't present");
    ShowAlert();
  }
}

std::string BaseMuteAction::GetRealDeviceID() const {
  return GetSettings().VolatileDeviceID();
}
