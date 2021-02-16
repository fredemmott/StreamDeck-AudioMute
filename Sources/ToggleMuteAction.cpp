/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "ToggleMuteAction.h"

#include <StreamDeckSDK/ESDLogger.h>

#include <AudioDevices/AudioDevices.h>
#include "MuteAction.h"
#include "UnmuteAction.h"

const std::string ToggleMuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.toggle");

void ToggleMuteAction::MuteStateDidChange(bool isMuted) {
  ESDDebug("MuteStateDidChange to {} for context {}", isMuted, GetContext());
  SetState(isMuted ? 0 : 1);
}

void ToggleMuteAction::WillAppear() {
  MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
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
