#include <CoreAudio/CoreAudio.h>
#include <fmt/format.h>

#include "AudioFunctions.h"

namespace {
std::string MakeDeviceID(UInt32 native, AudioDeviceDirection dir) {
  return fmt::format(
    "{}/{}", native, dir == AudioDeviceDirection::INPUT ? "input" : "output");
}

std::tuple<UInt32, AudioDeviceDirection> ParseDeviceID(const std::string& id) {
  auto idx = id.find_first_of('/');
  return std::make_tuple(
    std::stoi(id.substr(0, idx)), id.substr(idx + 1) == "input"
                                    ? AudioDeviceDirection::INPUT
                                    : AudioDeviceDirection::OUTPUT);
}

void SetAudioDeviceIsMuted(const std::string& id, bool muted) {
  const UInt32 native = muted;
  const auto [native_id, direction] = ParseDeviceID(id);
  AudioObjectPropertyAddress prop{kAudioDevicePropertyMute,
                                  direction == AudioDeviceDirection::INPUT
                                    ? kAudioDevicePropertyScopeInput
                                    : kAudioDevicePropertyScopeOutput,
                                  0};
  AudioObjectSetPropertyData(
    native_id, &prop, 0, NULL, sizeof(native), &native);
}

}// namespace

std::string GetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole _role) {
  AudioDeviceID native_id = 0;
  UInt32 native_id_size = sizeof(native_id);
  AudioObjectPropertyAddress prop
    = {direction == AudioDeviceDirection::INPUT
         ? kAudioHardwarePropertyDefaultInputDevice
         : kAudioHardwarePropertyDefaultOutputDevice,
       kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};

  AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &prop, 0, NULL, &native_id_size, &native_id);
  return MakeDeviceID(native_id, direction);
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
  SetAudioDeviceIsMuted(id, true);
}

void UnmuteAudioDevice(const std::string& id) {
  SetAudioDeviceIsMuted(id, false);
}
