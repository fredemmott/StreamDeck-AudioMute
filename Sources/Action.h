#pragma once

#include <nlohmann/json.hpp>
#include <string>

class ESDConnectionManager;

class Action {
 public:
  struct Settings {
    std::string deviceID;
    bool feedbackSounds = true;

    bool operator==(const Settings& other) const {
      return deviceID == other.deviceID
             && feedbackSounds == other.feedbackSounds;
    }
  };

  Action(ESDConnectionManager* esd_connection, const std::string& context);

  virtual ~Action();

  void DidReceiveSettings(const nlohmann::json& settings);

  void MuteStateDidChange(const nlohmann::json& settings, bool isMuted);
  void WillAppear(const nlohmann::json& settings);
  void KeyUp(const nlohmann::json& settings);

 protected:
  virtual void MuteStateDidChange(bool isMuted) = 0;
  virtual void WillAppear() = 0;
  virtual void KeyUp() = 0;

  std::string GetContext() const;
  std::string GetRealDeviceID() const;

  ESDConnectionManager* GetESD() const;

  const Settings& GetSettings() const;

 private:
  std::string mContext;
  Settings mSettings;
  std::string mRealDeviceID;
  void* mMuteUnmuteCallbackHandle = nullptr;
  ESDConnectionManager* mESDConnection = nullptr;

  void RealDeviceDidChange();
};
