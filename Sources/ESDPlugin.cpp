/* Copyright (c) 2018, Corsair Memory, Inc.
 * Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "ESDPlugin.h"

#include "ESDAction.h"

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include <atomic>
#include <mutex>

using json = nlohmann::json;

ESDPlugin::ESDPlugin(): ESDBasePlugin() {
}

ESDPlugin::~ESDPlugin() {
}

void ESDPlugin::KeyUpForAction(
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

void ESDPlugin::WillAppearForAction(
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

void ESDPlugin::SendToPlugin(
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

void ESDPlugin::DidReceiveSettings(
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
