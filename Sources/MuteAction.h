#pragma once

#include "Action.h"

class MuteAction final : public Action {
 public:
  MuteAction(
    ESDConnectionManager* esd,
    const std::string& context);
  static const std::string ACTION_ID;

 protected:
  virtual void MuteStateDidChange(bool isMuted) override;
  virtual void WillAppear() override;
  virtual void KeyUp() override;
};
