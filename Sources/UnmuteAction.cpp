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
  MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
}

void UnmuteAction::DoAction() {
  const auto device(GetRealDeviceID());
  if (!IsAudioDeviceMuted(device)) {
    return;
  }
  UnmuteAudioDevice(device);
  if (FeedbackSoundsEnabled()) {
    PlayFeedbackSound();
  }
}

void UnmuteAction::PlayFeedbackSound() {
  FredEmmott::Audio::PlayWavFile(ESDUtilities::AddPathComponent(
    ESDUtilities::GetPluginDirectoryPath(), "unmute.wav"));
}
