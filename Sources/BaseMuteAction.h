/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include "ESDActionWithExternalState.h"

struct MuteActionSettings {
  std::string deviceID;
  bool feedbackSounds = true;

  bool operator==(const MuteActionSettings& other) const {
    return deviceID == other.deviceID && feedbackSounds == other.feedbackSounds;
  }
};

void from_json(const nlohmann::json& json, MuteActionSettings& settings);

namespace FredEmmott::Audio {
class DefaultChangeCallbackHandle;
class MuteCallbackHandle;
}// namespace FredEmmott::Audio

using namespace FredEmmott::Audio;

class BaseMuteAction : public ESDActionWithExternalState<MuteActionSettings> {
 public:
  BaseMuteAction(
    ESDConnectionManager* esd_connection,
    const std::string& context);
  virtual ~BaseMuteAction();

  void MuteStateDidChange(const nlohmann::json& settings, bool isMuted);
  void SendToPlugin(const nlohmann::json& payload) override;

 protected:
  virtual std::string GetActionID() const = 0;
  virtual void DoAction() = 0;

  virtual void SettingsDidChange(
    const MuteActionSettings& old_settings,
    const MuteActionSettings& new_settings) final;
  virtual void MuteStateDidChange(bool isMuted) = 0;
  virtual void KeyUp() final override;
  std::string GetRealDeviceID() const;

 private:
  std::string mRealDeviceID;
  std::unique_ptr<MuteCallbackHandle> mMuteUnmuteCallbackHandle;
  std::unique_ptr<DefaultChangeCallbackHandle> mDefaultChangeCallbackHandle;

  void RealDeviceDidChange();
};
