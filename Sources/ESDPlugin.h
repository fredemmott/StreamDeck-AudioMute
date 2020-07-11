/* Copyright (c) 2018, Corsair Memory, Inc.
 * Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include <StreamDeckSDK/ESDBasePlugin.h>

#include <memory>

class ESDAction;

class ESDPlugin : public ESDBasePlugin {
 public:
  ESDPlugin();
  virtual ~ESDPlugin();

  void KeyUpForAction(
    const std::string& inAction,
    const std::string& inContext,
    const nlohmann::json& inPayload,
    const std::string& inDeviceID) override;

  void WillAppearForAction(
    const std::string& inAction,
    const std::string& inContext,
    const nlohmann::json& inPayload,
    const std::string& inDeviceID) override;

  void SendToPlugin(
    const std::string& inAction,
    const std::string& inContext,
    const nlohmann::json& inPayload,
    const std::string& inDevice) override;

  void DidReceiveSettings(
    const std::string& inAction,
    const std::string& inContext,
    const nlohmann::json& inPayload,
    const std::string& inDevice) override;

 protected:
  /** Create or retrieve an ESDAction instance for the given action/context.
   *
   * Return a null/empty shared_ptr if the action is unrecognized.
   */
  virtual std::shared_ptr<ESDAction>
  GetOrCreateAction(const std::string& action, const std::string& context) = 0;
};
