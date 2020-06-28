#include <CoreAudio/CoreAudio.h>
#include <fmt/format.h>

#include "AudioFunctions.h"

std::string GetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole _role) {
  AudioDeviceID theAnswer = 0;
  UInt32 theSize = sizeof(AudioDeviceID);
  AudioObjectPropertyAddress theAddress
    = {direction == AudioDeviceDirection::INPUT
         ? kAudioHardwarePropertyDefaultInputDevice
         : kAudioHardwarePropertyDefaultOutputDevice,
       kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};

  OSStatus theError = AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &theAddress, 0, NULL, &theSize, &theAnswer);
  return fmt::format("{}", theAnswer);
}

bool IsAudioDeviceMuted(const std::string& id) {
  UInt32 muted;
  UInt32 mutedSize = sizeof(muted);
  AudioObjectPropertyAddress prop{kAudioDevicePropertyMute,
                                  /* FIXME */ kAudioDevicePropertyScopeOutput,
                                  0};
  AudioObjectGetPropertyData(std::stoi(id), &prop, 0, NULL, &mutedSize, &muted);
  return muted;
};

void MuteAudioDevice(const std::string& id) {
  AudioObjectPropertyAddress prop{kAudioDevicePropertyMute,
                                  /* FIXME */ kAudioDevicePropertyScopeOutput,
                                  0};
  const UInt32 muted = 1;
  AudioObjectSetPropertyData(
    std::stoi(id), &prop, 0, NULL, sizeof(muted), &muted);
}

void UnmuteAudioDevice(const std::string& id) {
  AudioObjectPropertyAddress prop{kAudioDevicePropertyMute,
                                  /* FIXME */ kAudioDevicePropertyScopeOutput,
                                  0};
  const UInt32 muted = 0;
  AudioObjectSetPropertyData(
    std::stoi(id), &prop, 0, NULL, sizeof(muted), &muted);
}
