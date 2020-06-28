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

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include <atomic>
#include <mutex>

#include "Action.h"
#include "AudioFunctions.h"
#include "DefaultAudioDevices.h"
#include "ToggleMuteAction.h"

void to_json(json& j, const AudioDeviceInfo& device) {
  j = json({{"id", device.id},
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

void MyStreamDeckPlugin::KeyUpForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  GetOrCreateAction(inAction, inContext, inPayload["settings"])
    ->KeyUp(inPayload["settings"]);
}

void MyStreamDeckPlugin::WillAppearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  GetOrCreateAction(inAction, inContext, inPayload["settings"])->WillAppear(inPayload["settings"]);
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
          {{DefaultAudioDevices::DEFAULT_INPUT_ID,
            DefaultAudioDevices::GetRealDeviceID(DefaultAudioDevices::DEFAULT_INPUT_ID)},
           {DefaultAudioDevices::DEFAULT_OUTPUT_ID,
            DefaultAudioDevices::GetRealDeviceID(DefaultAudioDevices::DEFAULT_OUTPUT_ID)},
           {DefaultAudioDevices::COMMUNICATIONS_INPUT_ID,
            DefaultAudioDevices::GetRealDeviceID(DefaultAudioDevices::COMMUNICATIONS_INPUT_ID)},
           {DefaultAudioDevices::COMMUNICATIONS_OUTPUT_ID,
            DefaultAudioDevices::GetRealDeviceID(DefaultAudioDevices::COMMUNICATIONS_OUTPUT_ID)}}}});
  return;
}

void MyStreamDeckPlugin::DidReceiveSettings(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDevice) {
  json settings;
  EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);

  GetOrCreateAction(inAction, inContext, settings)
    ->DidReceiveSettings(settings);
}

std::shared_ptr<Action> MyStreamDeckPlugin::GetOrCreateAction(
  const std::string& action,
  const std::string& context,
  const nlohmann::json& settings) {
  std::scoped_lock lock(mActionsMutex);
  auto it = mActions.find(context);
  if (it != mActions.end()) {
    ESDDebug("Found existing action: {} {}", action, context);
    return it->second;
  }

  if (action == ToggleMuteAction::ACTION_ID) {
    ESDLog("Creating action {} with context {}", action, context);
    auto ptr = std::make_shared<ToggleMuteAction>(mConnectionManager, context);
    mActions.emplace(context, ptr);
    return ptr;
  }

  ESDLog("Attempted to create invalid action '{}'", action);
  return nullptr;
}
