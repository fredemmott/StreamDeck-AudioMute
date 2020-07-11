#pragma once

#include "BaseMuteAction.h"

class UnmuteAction final : public BaseMuteAction {
 public:
  UnmuteAction(
    ESDConnectionManager* esd,
    const std::string& context);
  static const std::string ACTION_ID;
  virtual std::string GetActionID() const {
    return ACTION_ID;
  }

  static void PlayFeedbackSound();
 protected:
  virtual void MuteStateDidChange(bool isMuted) override;
  virtual void WillAppear() override;
  virtual void KeyUp() override;
};
