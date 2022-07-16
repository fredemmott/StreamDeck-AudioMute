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
  try {
    MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
    if (!IsDevicePresent()) {
      ShowAlert();
    }
  } catch (const FredEmmott::Audio::device_error&) {
    ShowAlert();
  }
}

void MuteAction::DoAction() {
  if (IsAudioDeviceMuted(GetRealDeviceID())) {
    return;
  }
  MuteAudioDevice(GetRealDeviceID());
  if (FeedbackSoundsEnabled()) {
    PlayFeedbackSound();
  }
}

void MuteAction::PlayFeedbackSound() {
  FredEmmott::Audio::PlayWavFile(
    ESDUtilities::GetPluginDirectoryPath() / "mute.wav");
}
