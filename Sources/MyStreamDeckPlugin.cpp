//==============================================================================
/**
@file       MyStreamDeckPlugin.cpp

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include "MyStreamDeckPlugin.h"
#include <atomic>

#include "AudioFunctions.h"
#include "Common/EPLJSONUtils.h"
#include "Common/ESDConnectionManager.h"

namespace {
const char* DEFAULT_INPUT_ID = "com.fredemmott.sdmute.deviceIds.defaultInput";
const char* DEFAULT_OUTPUT_ID = "com.fredemmott.sdmute.deviceIds.defaultOutput";
const char* COMMUNICATIONS_INPUT_ID
  = "com.fredemmott.sdmute.deviceIds.communicationsInput";
const char* COMMUNICATIONS_OUTPUT_ID
  = "com.fredemmott.sdmute.deviceIds.communicationsOutput";
}// namespace

void to_json(json& j, const AudioDeviceInfo& device) {
  j = json(
    {{"id", device.id},
     {"interfaceName", device.interfaceName},
     {"endpointName", device.endpointName},
     {"displayName", device.displayName},
     {"state", device.state}});
}

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

MyStreamDeckPlugin::MyStreamDeckPlugin() {
  CoInitialize(NULL);// initialize COM for the main thread
}

MyStreamDeckPlugin::~MyStreamDeckPlugin() {
}

void MyStreamDeckPlugin::UpdateContextCallback(const std::string& context) {
  mVisibleContextsMutex.lock();
  if (mContextCallbacks.find(context) != mContextCallbacks.end()) {
    RemoveAudioDeviceMuteUnmuteCallback(mContextCallbacks[context]);
  }
  DebugPrint(
    "Installing callback for %s",
    ConvertPluginAudioDeviceID(mContextDeviceIDs[context]).c_str());
  mContextCallbacks[context] = AddAudioDeviceMuteUnmuteCallback(
    ConvertPluginAudioDeviceID(mContextDeviceIDs[context]),
    [this, context](bool isMuted) {
      DebugPrint("In callback - %s", isMuted ? "muted" : "unmuted");
      mVisibleContextsMutex.lock();
      mConnectionManager->SetState(isMuted ? 0 : 1, context);
      mVisibleContextsMutex.unlock();
    });
  DebugPrint("Got callback? %s", mContextCallbacks[context] ? "yes" : "no");
  mVisibleContextsMutex.unlock();
}

std::string MyStreamDeckPlugin::ConvertPluginAudioDeviceID(
  const std::string& dev) {
  if (dev == DEFAULT_INPUT_ID) {
    return GetDefaultAudioDeviceID(AudioDeviceDirection::INPUT, AudioDeviceRole::DEFAULT);
  }
  if (dev == DEFAULT_OUTPUT_ID) {
    return GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::DEFAULT);
  }
  if (dev == COMMUNICATIONS_INPUT_ID) {
    return GetDefaultAudioDeviceID(AudioDeviceDirection::INPUT, AudioDeviceRole::COMMUNICATION);
  }
  if (dev == COMMUNICATIONS_OUTPUT_ID) {
    return GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::COMMUNICATION);
  }
  return dev;
}

void MyStreamDeckPlugin::KeyDownForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // Nothing to do
}

void MyStreamDeckPlugin::KeyUpForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  mVisibleContextsMutex.lock();
  const auto id = ConvertPluginAudioDeviceID(mContextDeviceIDs[inContext]);
  SetIsAudioDeviceMuted(id, MuteAction::TOGGLE);
  if (mContextFeedbackSounds[inContext]) {
    if (IsAudioDeviceMuted(id)) {
      PlayFeedbackSound(MuteAction::MUTE);
    } else {
      PlayFeedbackSound(MuteAction::UNMUTE);
    }
  }
  mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::WillAppearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // Remember the context
  mVisibleContextsMutex.lock();
  mVisibleContexts.insert(inContext);

  json settings;
  EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);
  const std::string audioDevice = EPLJSONUtils::GetStringByName(
    settings, "deviceID", COMMUNICATIONS_INPUT_ID);
  mContextDeviceIDs[inContext] = audioDevice;
  mContextFeedbackSounds[inContext]
    = EPLJSONUtils::GetBoolByName(settings, "feedbackSounds", true);
  mConnectionManager->SetState(
    IsAudioDeviceMuted(ConvertPluginAudioDeviceID(audioDevice)) ? 0 : 1,
    inContext);
  mVisibleContextsMutex.unlock();
  UpdateContextCallback(inContext);
}

void MyStreamDeckPlugin::WillDisappearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // Remove the context
  mVisibleContextsMutex.lock();
  mVisibleContexts.erase(inContext);
  mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::DeviceDidConnect(
  const std::string& inDeviceID,
  const json& inDeviceInfo) {
  // Nothing to do
}

void MyStreamDeckPlugin::DeviceDidDisconnect(const std::string& inDeviceID) {
  // Nothing to do
}

void MyStreamDeckPlugin::DidReceiveGlobalSettings(const json&) {
}

void MyStreamDeckPlugin::SendToPlugin(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDevice) {
  const auto event = EPLJSONUtils::GetStringByName(inPayload, "event");

  if (event != "getDeviceList") {
    return;
  }
  mConnectionManager->SendToPropertyInspector(
    inAction, inContext,
    json{{"event", event},
         {"outputDevices", GetAudioDeviceList(AudioDeviceDirection::OUTPUT)},
         {"inputDevices", GetAudioDeviceList(AudioDeviceDirection::INPUT)},
         {"defaultDevices",
          {
            {DEFAULT_INPUT_ID,
             GetDefaultAudioDeviceID(AudioDeviceDirection::INPUT, AudioDeviceRole::DEFAULT)},
            {DEFAULT_OUTPUT_ID,
             GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::DEFAULT)},
            {COMMUNICATIONS_INPUT_ID,
             GetDefaultAudioDeviceID(AudioDeviceDirection::INPUT, AudioDeviceRole::COMMUNICATION)},
            {COMMUNICATIONS_OUTPUT_ID,
             GetDefaultAudioDeviceID(AudioDeviceDirection::OUTPUT, AudioDeviceRole::COMMUNICATION)},
          }}});
  return;
}

void MyStreamDeckPlugin::DidReceiveSettings(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDevice) {
  json settings;
  EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);
  mContextDeviceIDs[inContext] = EPLJSONUtils::GetStringByName(
    settings, "deviceID", COMMUNICATIONS_INPUT_ID);
  mContextFeedbackSounds[inContext]
    = EPLJSONUtils::GetBoolByName(settings, "feedbackSounds", true);
  UpdateContextCallback(inContext);
}
