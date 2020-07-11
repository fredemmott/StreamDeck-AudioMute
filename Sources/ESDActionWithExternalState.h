/* Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include "ESDAction.h"

/** An ESDAction where state can be changed by something other than the
 * StreamDeck software.
 *
 * For example:
 * - hardware events
 * - time
 *
 * In these cases, you likely want to consider settings to be persistent, and
 * 'settings changed' to be an event in itself. This class removes the settings
 * parameter (which may be changed or unchanged) from most events, and adds a
 * new event for when they've changed.
 */
template <class TSettings>
class ESDActionWithExternalState : public ESDAction {
 public:
  ESDActionWithExternalState(
    ESDConnectionManager* esd_connection,
    const std::string& context)
    : ESDAction(esd_connection, context) {
  }

  virtual ~ESDActionWithExternalState() {
  }

  virtual void DidReceiveSettings(const nlohmann::json& json_settings) final {
    TSettings new_settings(json_settings);
    if (new_settings == mSettings) {
      return;
    }
    const auto old_settings = std::move(mSettings);
    mSettings = std::move(new_settings);
    SettingsDidChange(old_settings, mSettings);
  }

  virtual void WillAppear(const nlohmann::json& settings) final {
    DidReceiveSettings(settings);
    WillAppear();
  }

  virtual void KeyUp(const nlohmann::json& settings) final {
    DidReceiveSettings(settings);
    KeyUp();
  }

 protected:
  virtual void SettingsDidChange(
    const TSettings& old_settings,
    const TSettings& new_settings)
    = 0;
  virtual void WillAppear() {
  }
  virtual void KeyUp() {
  }

  const TSettings& GetSettings() const {
    return mSettings;
  }

 private:
  TSettings mSettings;
};
