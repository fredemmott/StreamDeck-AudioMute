#include <fmt/format.h>
#include <unistd.h>

#include "AudioFunctions.h"
int main(int argc, char** argv) {
  fmt::print("Input devices:\n");
  for (const auto& [id, info] :
       GetAudioDeviceList(AudioDeviceDirection::INPUT)) {
    fmt::print(
      "- Key: {}\n  Id: {}\n  Name: {}\n  Direction: {}\n", id, info.id,
      info.displayName,
      info.direction == AudioDeviceDirection::INPUT ? "input" : "output");
  }
  fmt::print("Output devices:\n");
  for (const auto& [id, info] :
       GetAudioDeviceList(AudioDeviceDirection::OUTPUT)) {
    fmt::print(
      "- Key: {}\n  Id: {}\n  Name: {}\n  Direction: {}\n", id, info.id,
      info.displayName,
      info.direction == AudioDeviceDirection::INPUT ? "input" : "output");
  }

  const auto device = GetDefaultAudioDeviceID(
    AudioDeviceDirection::OUTPUT, AudioDeviceRole::DEFAULT);
  fmt::print("Default ID: {}\n", device);
  fmt::print("Muted? {}. Muting...\n", IsAudioDeviceMuted(device));
  MuteAudioDevice(device);
  fmt::print("Muted? {}... sleep(3)\n", IsAudioDeviceMuted(device));
  sleep(3);
  UnmuteAudioDevice(device);
  fmt::print("Muted? {}\n", IsAudioDeviceMuted(device));
  return 0;
}
