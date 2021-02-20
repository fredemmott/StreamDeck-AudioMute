/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include <StreamDeckSDK/ESDActionWithExternalState.h>

struct MuteActionSettings {
  std::string deviceID;
  bool feedbackSounds = true;
  bool ptt = false;

  bool operator==(const MuteActionSettings& other) const {
    return deviceID == other.deviceID && feedbackSounds == other.feedbackSounds && ptt == other.ptt;
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
  using ESDActionWithExternalState<MuteActionSettings>::ESDActionWithExternalState;
  virtual ~BaseMuteAction();

  void MuteStateDidChange(const nlohmann::json& settings, bool isMuted);
  void SendToPlugin(const nlohmann::json& payload) override;

 protected:
  virtual void DoAction() = 0;

  virtual void SettingsDidChange(
    const MuteActionSettings& old_settings,
    const MuteActionSettings& new_settings);
  virtual void MuteStateDidChange(bool isMuted) = 0;
  virtual void KeyUp() override;
  std::string GetRealDeviceID() const;
  virtual bool FeedbackSoundsEnabled() const;

 private:
  std::string mRealDeviceID;
  std::unique_ptr<MuteCallbackHandle> mMuteUnmuteCallbackHandle;
  std::unique_ptr<DefaultChangeCallbackHandle> mDefaultChangeCallbackHandle;
  bool mFeedbackSounds;

  void RealDeviceDidChange();
};
