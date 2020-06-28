#pragma once

#include "MuteAction.h"

#include <StreamDeckSDK/ESDConnectionManager.h>

#include "AudioFunctions.h"

const std::string MuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.mute");

MuteAction::MuteAction(
  ESDConnectionManager* esd,
  const std::string& context)
  : Action(esd, context) {
}

void MuteAction::MuteStateDidChange(bool isMuted) {
  GetESD()->SetState(isMuted ? 0 : 1, GetContext());
}

void MuteAction::WillAppear() {
  MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
}

void MuteAction::KeyUp() {
  if (IsAudioDeviceMuted(GetRealDeviceID())) {
    return;
  }
  MuteAudioDevice(GetRealDeviceID());
  PlayFeedbackSound(FeedbackSoundEvent::MUTE);
}
