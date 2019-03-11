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
#include "CallbackTimer.h"
#include "Common/EPLJSONUtils.h"
#include "Common/ESDConnectionManager.h"

MyStreamDeckPlugin::MyStreamDeckPlugin() {
  CoInitialize(NULL);// initialize COM for the main thread
  mTimer = new CallBackTimer();
  mTimer->start(500, [this]() { this->UpdateTimer(); });
}

MyStreamDeckPlugin::~MyStreamDeckPlugin() {
  if (mTimer != nullptr) {
    mTimer->stop();

    delete mTimer;
    mTimer = nullptr;
  }
}

void MyStreamDeckPlugin::UpdateTimer() {
  //
  // Warning: UpdateTimer() is running in the timer thread
  //
  if (mConnectionManager != nullptr) {
    mVisibleContextsMutex.lock();
    const bool isMuted = IsMuted();
    for (const std::string& context : mVisibleContexts) {
      mConnectionManager->SetState(isMuted ? 0 : 1, context);
    }
    mVisibleContextsMutex.unlock();
  }
}

bool MyStreamDeckPlugin::IsMuted() {
  return IsAudioDeviceMuted(
    GetDefaultAudioDeviceID(Direction::INPUT, Role::COMMUNICATION));
}

void MyStreamDeckPlugin::KeyDownForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // don't race the timer, which leads to flickering
  mVisibleContextsMutex.lock();
  SetIsAudioDeviceMuted(
    GetDefaultAudioDeviceID(Direction::INPUT, Role::COMMUNICATION),
    MuteAction::TOGGLE);
}

void MyStreamDeckPlugin::KeyUpForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  mVisibleContextsMutex.unlock();
  // Nothing to do
}

void MyStreamDeckPlugin::WillAppearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // Remember the context
  mVisibleContextsMutex.lock();
  mVisibleContexts.insert(inContext);
  mConnectionManager->SetState(IsMuted() ? 0 : 1, inContext);
  mVisibleContextsMutex.unlock();
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
         {"inputDevices", GetAudioDeviceList(Direction::INPUT)}});
  return;
}

void MyStreamDeckPlugin::DidReceiveSettings(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDevice) {
}
