/* Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>

class ESDConnectionManager;

class ESDAction {
 public:
  ESDAction(ESDConnectionManager* esd_connection, const std::string& context);
  virtual ~ESDAction();

  virtual void DidReceiveSettings(const nlohmann::json& settings);
  virtual void KeyUp(const nlohmann::json& settings);
  virtual void SendToPlugin(const nlohmann::json& payload);
  virtual void WillAppear(const nlohmann::json& settings);

 protected:
  std::string GetContext() const;
  ESDConnectionManager* GetESD() const;

 private:
  std::string mContext;
  ESDConnectionManager* mESDConnection = nullptr;
};
