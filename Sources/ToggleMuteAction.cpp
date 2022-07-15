/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "ToggleMuteAction.h"

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include "MuteAction.h"
#include "UnmuteAction.h"

const std::string ToggleMuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.toggle");

void ToggleMuteAction::MuteStateDidChange(bool isMuted) {
  ESDDebug("MuteStateDidChange to {} for context {}", isMuted, GetContext());
  SetState(isMuted ? 0 : 1);
}

void ToggleMuteAction::WillAppear() {
  try {
    MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
    if (!IsDevicePresent()) {
      ShowAlert();
    }
  } catch (FredEmmott::Audio::device_error&) {
    ShowAlert();
  }
}

void ToggleMuteAction::DoAction() {
  const auto device(GetRealDeviceID());
  if (IsAudioDeviceMuted(device)) {
    UnmuteAudioDevice(device);
    if (FeedbackSoundsEnabled()) {
      UnmuteAction::PlayFeedbackSound();
    }
  } else {
    MuteAudioDevice(device);
    if (FeedbackSoundsEnabled()) {
      MuteAction::PlayFeedbackSound();
    }
  }
}

void ToggleMuteAction::KeyUp() {
  if (mPushAndHoldToTalk) {
    const bool muted = IsAudioDeviceMuted(GetRealDeviceID());
    if (
      std::chrono::steady_clock::now() - mKeyDownTime
      < std::chrono::milliseconds(500)) {
      // Short press, and action already taken on keydown - but the streamdeck
      // software automatically switches state on key up. Undo this.
      MuteStateDidChange(muted);
      return;
    }

    if (muted) {
      // PTM-mode, unmute immediately...
      DoAction();
      return;
    }

    // PTT: people tend to let go of the button just a little too soon, so delay
    auto ctx = GetESD()->GetAsioContext();
    mPttReleaseTimer = std::make_unique<asio::steady_timer>(
      *ctx, std::chrono::milliseconds(250));
    mPttReleaseTimer->async_wait([this](const asio::error_code& ec) {
      if (ec) {
        return;
      }
      DoAction();
    });
    return;
  }
  BaseMuteAction::KeyUp();
}

void ToggleMuteAction::KeyDown() {
  if (mPushAndHoldToTalk) {
    DoAction();
    mKeyDownTime = std::chrono::steady_clock::now();
    return;
  }
  BaseMuteAction::KeyDown();
}

void ToggleMuteAction::SettingsDidChange(
  const MuteActionSettings& oldSettings,
  const MuteActionSettings& newSettings) {
  mPushAndHoldToTalk = newSettings.ptt;
  BaseMuteAction::SettingsDidChange(oldSettings, newSettings);
}
