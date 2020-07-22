/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "UnmuteAction.h"

#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDUtilities.h>

#include "AudioDevices.h"
#include "PlayWavFile.h"

const std::string UnmuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.unmute");

UnmuteAction::UnmuteAction(
  ESDConnectionManager* esd,
  const std::string& context)
  : BaseMuteAction(esd, context) {
}

void UnmuteAction::MuteStateDidChange(bool isMuted) {
  GetESD()->SetState(isMuted ? 1 : 0, GetContext());
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
  PlayFeedbackSound();
}

void UnmuteAction::PlayFeedbackSound() {
  FredEmmott::Audio::PlayWavFile(ESDUtilities::AddPathComponent(
    ESDUtilities::GetPluginDirectoryPath(), "unmute.wav"));
}
