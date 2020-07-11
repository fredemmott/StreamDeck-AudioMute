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

#include "AudioFunctions.h"
#include "DefaultAudioDevices.h"
#include "ESDAction.h"
#include "MuteAction.h"
#include "ToggleMuteAction.h"
#include "UnmuteAction.h"

MyStreamDeckPlugin::MyStreamDeckPlugin() {
#ifdef _MSC_VER
  CoInitializeEx(
    NULL, COINIT_MULTITHREADED);// initialize COM for the main thread
#endif
}

MyStreamDeckPlugin::~MyStreamDeckPlugin() {
}

void MyStreamDeckPlugin::KeyUpForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  auto action = GetOrCreateAction(inAction, inContext);
  if (!action) {
    ESDLog("No action for keyup - {} {}", inAction, inContext);
    return;
  }
  action->KeyUp(inPayload["settings"]);
}

void MyStreamDeckPlugin::WillAppearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  auto action = GetOrCreateAction(inAction, inContext);
  if (!action) {
    ESDLog("No action for WillAppear - {} {}", inAction, inContext);
    return;
  }
  action->WillAppear(inPayload["settings"]);
}

void MyStreamDeckPlugin::SendToPlugin(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDevice) {
    auto action = GetOrCreateAction(inAction, inContext);
  if (!action) {
    ESDLog(
      "Received plugin request for unknown action {} {}", inAction, inContext);
    return;
  }
  action->SendToPlugin(inPayload);
}

void MyStreamDeckPlugin::DidReceiveSettings(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDevice) {
  auto action = GetOrCreateAction(inAction, inContext);
  if (!action) {
    ESDLog("No action for DidReceiveSettings: {} {}", inAction, inContext);
    return;
  }
  action->DidReceiveSettings(inPayload["settings"]);
}

std::shared_ptr<ESDAction> MyStreamDeckPlugin::GetOrCreateAction(
  const std::string& action,
  const std::string& context) {
  std::scoped_lock lock(mActionsMutex);
  auto it = mActions.find(context);
  if (it != mActions.end()) {
    ESDDebug("Found existing action: {} {}", action, context);
    return it->second;
  }

  ESDLog("Creating action {} with context {}", action, context);

  if (action == ToggleMuteAction::ACTION_ID) {
    ESDDebug("Creating ToggleMuteAction");
    auto impl = std::make_shared<ToggleMuteAction>(mConnectionManager, context);
    mActions.emplace(context, impl);
    return impl;
  }

  if (action == MuteAction::ACTION_ID) {
    ESDDebug("Creating MuteAction");
    auto impl = std::make_shared<MuteAction>(mConnectionManager, context);
    mActions.emplace(context, impl);
    return impl;
  }

  if (action == UnmuteAction::ACTION_ID) {
    ESDDebug("Creating UnmuteAction");
    auto impl = std::make_shared<UnmuteAction>(mConnectionManager, context);
    mActions.emplace(context, impl);
    return impl;
  }

  ESDLog("Didn't recognize action '{}'", action);
  return nullptr;
}
