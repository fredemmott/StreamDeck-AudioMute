#pragma once

#include "Action.h"

class UnmuteAction final : public Action {
 public:
  UnmuteAction(
    ESDConnectionManager* esd,
    const std::string& context);
  static const std::string ACTION_ID;

  static void PlayFeedbackSound();
 protected:
  virtual void MuteStateDidChange(bool isMuted) override;
  virtual void WillAppear() override;
  virtual void KeyUp() override;
};
