#include "UnmuteAction.h"

#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDUtilities.h>

#include "AudioFunctions.h"
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

void UnmuteAction::KeyUp() {
  const auto device(GetRealDeviceID());
  if (!IsAudioDeviceMuted(device)) {
    return;
  }
  UnmuteAudioDevice(device);
  PlayFeedbackSound();
}


void UnmuteAction::PlayFeedbackSound() {
  PlayWavFile(ESDUtilities::AddPathComponent(
    ESDUtilities::GetPluginDirectoryPath(), "unmute.wav"));
}
