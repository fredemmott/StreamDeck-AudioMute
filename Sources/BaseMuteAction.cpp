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
#include <regex>

#include "DefaultAudioDevices.h"
#include "audio_json.h"

using json = nlohmann::json;

NLOHMANN_JSON_SERIALIZE_ENUM(
  ToggleKind,
  {
    {ToggleKind::ToggleOnly, "ToggleOnly"},
    {ToggleKind::TogglePressPttPtmHold, "TogglePressPttPtmHold"},
    {ToggleKind::PttPtmOnly, "PttPtmOnly"},
  });

static AudioDeviceInfo GetAudioDeviceInfo(const std::string& id) {
  {
    const auto in = GetAudioDeviceList(AudioDeviceDirection::INPUT);
    if (in.contains(id)) {
      return in.at(id);
    }
  }

  {
    const auto out = GetAudioDeviceList(AudioDeviceDirection::OUTPUT);
    if (out.contains(id)) {
      return out.at(id);
    }
  }

  return {.id = id};
}

void from_json(const json& json, MuteActionSettings& settings) {
  if (json.contains("device")) {
    settings.device = json.at("device");
  }

  if (json.contains("deviceID")) {
    const auto id = json.at("deviceID").get<std::string>();

    if (DefaultAudioDevices::GetRealDeviceID(id) != id) {
      settings.device = {.id = id};
    } else if (id != settings.device.id) {
      settings.device = GetAudioDeviceInfo(id);
    }
  } else {
    settings.device = {
      .id = DefaultAudioDevices::GetRealDeviceID(
              DefaultAudioDevices::COMMUNICATIONS_INPUT_ID)
              .empty()
        ? DefaultAudioDevices::DEFAULT_INPUT_ID
        : DefaultAudioDevices::COMMUNICATIONS_INPUT_ID,
    };
  };

  settings.fuzzyMatching = json.value<bool>("fuzzyMatching", false);
  settings.feedbackSounds = json.value<bool>("feedbackSounds", true);

  if (json.contains("toggleKind")) {
    settings.toggleKind = json.at("toggleKind");
  }

  // Legacy setting
  if (json.value<bool>("ptt", false)) {
    settings.toggleKind = ToggleKind::TogglePressPttPtmHold;
  }
}

static std::string FuzzifyInterface(const std::string& name) {
  // Windows likes to replace "Foo" with "2- Foo"
  const std::regex pattern {"^([0-9]+- )?(.+)$"};
  std::smatch captures;
  if (!std::regex_match(name, captures, pattern)) {
    return name;
  }
  return captures[2];
}

std::string MuteActionSettings::VolatileDeviceID() const {
  const auto specificID = DefaultAudioDevices::GetRealDeviceID(device.id);
  if (GetAudioDeviceState(specificID) == AudioDeviceState::CONNECTED) {
    ESDDebug("Exact match found");
    return specificID;
  }

  if (!fuzzyMatching) {
    ESDDebug("Fuzzy disabled, but no match");
    return specificID;
  }

  if (device.interfaceName.empty()) {
    ESDDebug(
      "Would try a fuzzy match, but don't have a saved interface name :'(");
    return specificID;
  }

  const auto fuzzyInterface = FuzzifyInterface(device.interfaceName);
  ESDDebug(
    "Trying a fuzzy match: '{}' -> '{}'", device.interfaceName, fuzzyInterface);

  for (const auto& [otherId, otherDevice]:
       GetAudioDeviceList(device.direction)) {
    if (otherDevice.state != AudioDeviceState::CONNECTED) {
      continue;
    }
    const auto fuzzyOtherInterface
      = FuzzifyInterface(otherDevice.interfaceName);
    ESDDebug(
      "Trying '{}' -> '{}'", otherDevice.interfaceName, fuzzyOtherInterface);
    if (
      fuzzyInterface != fuzzyOtherInterface
      || device.endpointName != otherDevice.endpointName) {
      continue;
    }
    ESDDebug(
      "Fuzzy device match for {}/{}",
      device.interfaceName,
      device.endpointName);
    return otherId;
  }

  ESDDebug(
    "Failed fuzzy match for {}/{}", device.interfaceName, device.endpointName);
  return device.id;
}

BaseMuteAction::BaseMuteAction(
  ESDConnectionManager* esd_connection,
  const std::string& action,
  const std::string& context)
  : ESDActionWithExternalState(esd_connection, action, context) {
  mPlugEventCallbackHandle = AddAudioDevicePlugEventCallback(
    std::bind_front(&BaseMuteAction::OnPlugEvent, this));
  mDefaultChangeCallbackHandle = AddDefaultAudioDeviceChangeCallback(
    std::bind_front(&BaseMuteAction::OnDefaultDeviceChange, this));
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
}

void BaseMuteAction::OnPlugEvent(
  AudioDevicePlugEvent event,
  const std::string& deviceID) {
  ESDLog(
    "Received plug event {} for device {}", static_cast<int>(event), deviceID);
  if (deviceID != GetRealDeviceID()) {
    ESDLog("Device is not a match");
    return;
  }

  switch (event) {
    case AudioDevicePlugEvent::ADDED: {
      ESDLog("Matching device added");
      this->RealDeviceDidChange();
      // Windows will now preserve/enforce the state if changed while the
      // device is unplugged, but MacOS won't, so we need to update the
      // displayed state to match reality
      const auto result = IsAudioDeviceMuted(deviceID);
      if (result.has_value()) {
        this->MuteStateDidChange(result.value());
      }
      ShowOK();
      return;
    }
    case AudioDevicePlugEvent::REMOVED:
      ESDLog("Matching device removed");
      this->RealDeviceDidChange();
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
  if (!state.has_value()) {
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
