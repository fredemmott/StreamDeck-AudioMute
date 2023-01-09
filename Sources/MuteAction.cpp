/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#include "MuteAction.h"

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/ESDUtilities.h>

#include "PlayWavFile.h"

const std::string MuteAction::ACTION_ID("com.fredemmott.micmutetoggle.mute");

void MuteAction::MuteStateDidChange(bool isMuted) {
  SetState(isMuted ? 0 : 1);
}

void MuteAction::WillAppear() {
  auto muteState = IsAudioDeviceMuted(GetRealDeviceID());
  if (!muteState) {
    ShowAlert();
    return;
  }
  MuteStateDidChange(*muteState);
}

void MuteAction::DoAction() {
  auto deviceID = GetRealDeviceID();
  auto muteState = IsAudioDeviceMuted(deviceID);
  if (!muteState) {
    ShowAlert();
    return;
  }
  const auto muted = *muteState;
  if (muted) {
    return;
  }

  if (!MuteAudioDevice(deviceID)) {
    ShowAlert();
    return;
  }

  if (FeedbackSoundsEnabled()) {
    PlayFeedbackSound();
  }
}

void MuteAction::PlayFeedbackSound() {
  FredEmmott::Audio::PlayWavFile(
    ESDUtilities::GetPluginDirectoryPath() / "mute.wav");
}
