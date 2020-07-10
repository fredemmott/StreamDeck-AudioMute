#pragma once

#include <nlohmann/json.hpp>
#include <string>

class ESDConnectionManager;

class Action {
 public:
  Action(ESDConnectionManager* esd_connection, const std::string& context);
  virtual ~Action();

  virtual void DidReceiveSettings(const nlohmann::json& settings);
  virtual void WillAppear(const nlohmann::json& settings);
  virtual void KeyUp(const nlohmann::json& settings);

 protected:
  std::string GetContext() const;
  ESDConnectionManager* GetESD() const;

 private:
  std::string mContext;
  ESDConnectionManager* mESDConnection = nullptr;
};
