/* Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "ESDAction.h"

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDLogger.h>

ESDAction::ESDAction(ESDConnectionManager* esd_connection, const std::string& context)
  : mESDConnection(esd_connection), mContext(context) {
}

ESDAction::~ESDAction() {
}

std::string ESDAction::GetContext() const {
  return mContext;
}

void ESDAction::DidReceiveSettings(const nlohmann::json& json) {
}

ESDConnectionManager* ESDAction::GetESD() const {
  return mESDConnection;
}

void ESDAction::KeyUp(const nlohmann::json& settings) {
}

void ESDAction::SendToPlugin(const nlohmann::json& payload) {
}

void ESDAction::WillAppear(const nlohmann::json& settings) {
}
