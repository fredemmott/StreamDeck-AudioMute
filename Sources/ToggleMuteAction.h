/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#pragma once

#include <asio.hpp>
#include <chrono>

#include "BaseMuteAction.h"

class ToggleMuteAction final : public BaseMuteAction {
 public:
  using BaseMuteAction::BaseMuteAction;
  static const std::string ACTION_ID;

  virtual void KeyDown() override;
  virtual void KeyUp() override;

 protected:
  virtual void SettingsDidChange(
    const MuteActionSettings& oldSettings,
    const MuteActionSettings& newSettings) override;

  virtual void MuteStateDidChange(bool isMuted) override;
  virtual void WillAppear() override;
  virtual void DoAction() override;
  
  void ToggleMute();

 private:
  ToggleKind mKind { ToggleKind::ToggleOnly };
  std::chrono::steady_clock::time_point mKeyDownTime;
  std::unique_ptr<asio::steady_timer> mPttReleaseTimer;
};
