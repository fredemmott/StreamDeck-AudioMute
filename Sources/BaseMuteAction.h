/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/ESDActionWithExternalState.h>

using namespace FredEmmott::Audio;

enum class DeviceMatchStrategy {
  ID,
  Fuzzy,
  Special,
};

struct MuteActionSettings {
  AudioDeviceInfo device;
  DeviceMatchStrategy matchStrategy = DeviceMatchStrategy::ID;
  bool feedbackSounds = true;
  bool ptt = false;

  auto operator<=>(const MuteActionSettings&) const = default;
  std::string VolatileDeviceID() const;
};
void from_json(const nlohmann::json& json, MuteActionSettings& settings);

class BaseMuteAction : public ESDActionWithExternalState<MuteActionSettings> {
 public:
  using ESDActionWithExternalState<
    MuteActionSettings>::ESDActionWithExternalState;
  virtual ~BaseMuteAction();

  void SendToPlugin(const nlohmann::json& payload) override;

 protected:
  virtual void DoAction() = 0;

  virtual void SettingsDidChange(
    const MuteActionSettings& old_settings,
    const MuteActionSettings& new_settings) override;
  virtual void MuteStateDidChange(bool isMuted) = 0;
  virtual void KeyUp() override;
  std::string GetRealDeviceID() const;
  virtual bool FeedbackSoundsEnabled() const;
  bool IsDevicePresent() const;

 private:
  MuteCallbackHandle mMuteUnmuteCallbackHandle;
  DefaultChangeCallbackHandle mDefaultChangeCallbackHandle;
  AudioDevicePlugEventCallbackHandle mPlugEventCallbackHandle;
  bool mFeedbackSounds;

  void RealDeviceDidChange();
  void OnPlugEvent(AudioDevicePlugEvent, const std::string& device);
  void OnDefaultDeviceChange(
    AudioDeviceDirection direction,
    AudioDeviceRole role,
    const std::string& device);
  void OnMuteStateChanged(const std::string& device, bool isMuted);
};
