#pragma once

#include "ToggleMuteAction.h"

#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include "AudioFunctions.h"
#include "MuteAction.h"
#include "UnmuteAction.h"

const std::string ToggleMuteAction::ACTION_ID(
  "com.fredemmott.micmutetoggle.toggle");

ToggleMuteAction::ToggleMuteAction(
  ESDConnectionManager* esd,
  const std::string& context)
  : Action(esd, context) {
}

void ToggleMuteAction::MuteStateDidChange(bool isMuted) {
  ESDDebug("MuteStateDidChange to {} for context {}", isMuted, GetContext());
  GetESD()->SetState(isMuted ? 0 : 1, GetContext());
}

void ToggleMuteAction::WillAppear() {
  MuteStateDidChange(IsAudioDeviceMuted(GetRealDeviceID()));
}

void ToggleMuteAction::KeyUp() {
  const auto device(GetRealDeviceID());
  if (IsAudioDeviceMuted(device)) {
    UnmuteAudioDevice(device);
    UnmuteAction::PlayFeedbackSound();
  } else {
    MuteAudioDevice(device);
    MuteAction::PlayFeedbackSound();
  }
}
