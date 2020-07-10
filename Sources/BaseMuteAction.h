#pragma once

#include "ESDActionWithExternalState.h"

class DefaultChangeCallbackHandle;
class MuteCallbackHandle;

struct MuteActionSettings {
  std::string deviceID;
  bool feedbackSounds = true;

  bool operator==(const MuteActionSettings& other) const {
    return deviceID == other.deviceID && feedbackSounds == other.feedbackSounds;
  }
};

void from_json(const nlohmann::json& json, MuteActionSettings& settings);

class BaseMuteAction : public ESDActionWithExternalState<MuteActionSettings> {
 public:
  BaseMuteAction(
    ESDConnectionManager* esd_connection,
    const std::string& context);
  virtual ~BaseMuteAction();

  void MuteStateDidChange(const nlohmann::json& settings, bool isMuted);

 protected:
  virtual void SettingsDidChange(
    const MuteActionSettings& old_settings,
    const MuteActionSettings& new_settings) final;
  virtual void MuteStateDidChange(bool isMuted) = 0;
  std::string GetRealDeviceID() const;

 private:
  std::string mRealDeviceID;
  std::unique_ptr<MuteCallbackHandle> mMuteUnmuteCallbackHandle;
  std::unique_ptr<DefaultChangeCallbackHandle> mDefaultChangeCallbackHandle;

  void RealDeviceDidChange();
};
