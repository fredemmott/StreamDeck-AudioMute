#include "AudioFunctions.h"

#include <fmt/format.h>
#include <unistd.h>
int main(int argc, char** argv) {
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
