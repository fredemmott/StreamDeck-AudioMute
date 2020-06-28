#pragma once

#include "ToggleMuteAction.h"

#include <StreamDeckSDK/ESDConnectionManager.h>

#include "AudioFunctions.h"

const std::string ToggleMuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.toggle");

ToggleMuteAction::ToggleMuteAction(
  ESDConnectionManager* esd,
  const std::string& context)
  : Action(esd, context) {
}

void ToggleMuteAction::MuteStateDidChange(bool isMuted) {
  GetESD()->SetState(isMuted ? 0 : 1, GetContext());
}

void ToggleMuteAction::WillAppear() {
  MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
}

void ToggleMuteAction::KeyUp() {
  const auto muted = IsAudioDeviceMuted(GetRealDeviceID());
  const auto action = muted ? MuteAction::UNMUTE : MuteAction::MUTE;
  SetIsAudioDeviceMuted(GetRealDeviceID(), action);
  PlayFeedbackSound(action);
}
