#include <CoreAudio/CoreAudio.h>
#include <fmt/format.h>

#include "AudioFunctions.h"

namespace {
std::string MakeDeviceID(UInt32 id, AudioDeviceDirection dir) {
  CFStringRef uid;
  UInt32 uid_size = sizeof(uid);
  AudioObjectPropertyAddress prop{kAudioDevicePropertyDeviceUID,
                                  kAudioObjectPropertyScopeGlobal,
                                  kAudioObjectPropertyElementMaster};
  AudioObjectGetPropertyData(id, &prop, 0, nullptr, &uid_size, &uid);

  auto utf8 = CFStringGetCStringPtr(uid, kCFStringEncodingUTF8);
  CFRelease(uid);
  return fmt::format(
    "{}/{}", dir == AudioDeviceDirection::INPUT ? "input" : "output", utf8);
}

std::tuple<UInt32, AudioDeviceDirection> ParseDeviceID(const std::string& id) {
  auto idx = id.find_first_of('/');
  auto direction = id.substr(0, idx) == "input" ? AudioDeviceDirection::INPUT
                                                : AudioDeviceDirection::OUTPUT;
  CFStringRef uid = CFStringCreateWithCString(
    kCFAllocatorDefault, id.substr(idx + 1).c_str(), kCFStringEncodingUTF8);
  UInt32 device_id;
  AudioValueTranslation value{&uid, sizeof(CFStringRef), &device_id,
                              sizeof(device_id)};
  AudioObjectPropertyAddress prop{kAudioHardwarePropertyDeviceForUID,
                                  kAudioObjectPropertyScopeGlobal,
                                  kAudioObjectPropertyElementMaster};
  UInt32 size = sizeof(value);

  AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &prop, 0, nullptr, &size, &value);
  CFRelease(uid);

  return std::make_tuple(device_id, direction);
}

void SetAudioDeviceIsMuted(const std::string& id, bool muted) {
  const UInt32 value = muted;
  const auto [native_id, direction] = ParseDeviceID(id);
  AudioObjectPropertyAddress prop{kAudioDevicePropertyMute,
                                  direction == AudioDeviceDirection::INPUT
                                    ? kAudioDevicePropertyScopeInput
                                    : kAudioDevicePropertyScopeOutput,
                                  0};
  AudioObjectSetPropertyData(
    native_id, &prop, 0, NULL, sizeof(value), &value);
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
  const auto [native_id, direction] = ParseDeviceID(id);
  AudioObjectPropertyAddress prop{kAudioDevicePropertyMute,
                                  direction == AudioDeviceDirection::INPUT
                                    ? kAudioDevicePropertyScopeInput
                                    : kAudioDevicePropertyScopeOutput,
                                  0};
  AudioObjectGetPropertyData(native_id, &prop, 0, nullptr, &mutedSize, &muted);
  return muted;
};

void MuteAudioDevice(const std::string& id) {
  SetAudioDeviceIsMuted(id, true);
}

void UnmuteAudioDevice(const std::string& id) {
  SetAudioDeviceIsMuted(id, false);
}

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(
  AudioDeviceDirection direction) {
  UInt32 count = 0;
  AudioObjectPropertyAddress prop
    = {kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal,
       kAudioObjectPropertyElementMaster};

  AudioObjectGetPropertyDataSize(
    kAudioObjectSystemObject, &prop, 0, nullptr, &count);

  UInt32 ids[count];
  UInt32 size = sizeof(ids);
  AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &prop, 0, nullptr, &size, &ids);

  std::map<std::string, AudioDeviceInfo> out;

  // The array of devices will always contain both input and output, even if we
  // set the scope above; instead filter inside the loop
  const auto scope = direction == AudioDeviceDirection::INPUT
                       ? kAudioObjectPropertyScopeInput
                       : kAudioObjectPropertyScopeOutput;
  for (UInt32 i = 0; i < count; ++i) {
    const auto id = ids[i];
    // ... and we do that filtering by finding out how many channels there are.
    // No channels for a given direction? Not a valid device for that direction.
    UInt32 num_streams;
    prop
      = {kAudioDevicePropertyStreams, scope, kAudioObjectPropertyScopeGlobal};
    AudioObjectGetPropertyDataSize(id, &prop, 0, nullptr, &num_streams);
    if (num_streams == 0) {
      continue;
    }
    AudioDeviceInfo info{.id = MakeDeviceID(id, direction),
                         .direction = direction};
    CFStringRef value = NULL;
    UInt32 size = sizeof(value);
    // kAudioObjectPropertyName: "Built-in Microphone" or "Built-in Output"
    prop = {kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMaster};
    AudioObjectGetPropertyData(id, &prop, 0, nullptr, &size, &value);
    if (!size) {
      continue;
    }
    info.displayName = CFStringGetCStringPtr(value, kCFStringEncodingUTF8);
    CFRelease(value);

    out.emplace(info.id, info);
  }
  return out;
}
