/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "UnmuteAction.h"

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/ESDUtilities.h>

#include "PlayWavFile.h"

const std::string UnmuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.unmute");

void UnmuteAction::MuteStateDidChange(bool isMuted) {
  SetState(isMuted ? 1 : 0);
}

void UnmuteAction::WillAppear() {
  const auto muteState = IsAudioDeviceMuted(GetRealDeviceID());
  if (!muteState.has_value()) {
    ShowAlert();
    return;
  }

  MuteStateDidChange(*muteState);
}

void UnmuteAction::DoAction() {
  const auto device(GetRealDeviceID());
  const auto muteState = IsAudioDeviceMuted(device);
  if (!muteState.has_value()) {
    ShowAlert();
    return;
  }

  const auto muted = *muteState;

  if (!muted) {
    return;
  }

  if (!UnmuteAudioDevice(device)) {
    ShowAlert();
    return;
  }

  if (FeedbackSoundsEnabled()) {
    PlayFeedbackSound();
  }
}

void UnmuteAction::PlayFeedbackSound() {
  FredEmmott::Audio::PlayWavFile(
    ESDUtilities::GetPluginDirectoryPath() / "unmute.wav");
}
