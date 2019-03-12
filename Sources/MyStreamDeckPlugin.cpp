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
    "Installign callback for %s",
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
    return GetDefaultAudioDeviceID(Direction::INPUT, Role::DEFAULT);
  }
  if (dev == DEFAULT_OUTPUT_ID) {
    return GetDefaultAudioDeviceID(Direction::OUTPUT, Role::DEFAULT);
  }
  if (dev == COMMUNICATIONS_INPUT_ID) {
    return GetDefaultAudioDeviceID(Direction::INPUT, Role::COMMUNICATION);
  }
  if (dev == COMMUNICATIONS_OUTPUT_ID) {
    return GetDefaultAudioDeviceID(Direction::OUTPUT, Role::COMMUNICATION);
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
  SetIsAudioDeviceMuted(
    ConvertPluginAudioDeviceID(mContextDeviceIDs[inContext]),
    MuteAction::TOGGLE);
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
         {"outputDevices", GetAudioDeviceList(Direction::OUTPUT)},
         {"inputDevices", GetAudioDeviceList(Direction::INPUT)},
         {"defaultDevices",
          {
            {DEFAULT_INPUT_ID,
             GetDefaultAudioDeviceID(Direction::INPUT, Role::DEFAULT)},
            {DEFAULT_OUTPUT_ID,
             GetDefaultAudioDeviceID(Direction::OUTPUT, Role::DEFAULT)},
            {COMMUNICATIONS_INPUT_ID,
             GetDefaultAudioDeviceID(Direction::INPUT, Role::COMMUNICATION)},
            {COMMUNICATIONS_OUTPUT_ID,
             GetDefaultAudioDeviceID(Direction::OUTPUT, Role::COMMUNICATION)},
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
  UpdateContextCallback(inContext);
}
