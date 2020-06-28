#pragma once

#include <functional>
#include <map>
#include <string>

enum class AudioDeviceRole {
  DEFAULT,
  COMMUNICATION,
};

enum class AudioDeviceDirection {
  OUTPUT,
  INPUT,
};

enum class AudioDeviceState {
  CONNECTED,
  DEVICE_NOT_PRESENT,// USB device unplugged
  DEVICE_DISABLED,
  DEVICE_PRESENT_NO_CONNECTION,// device present, but nothing's plugged into it,
                               // e.g. headphone jack with nothing plugged in
};

struct AudioDeviceInfo {
  std::string id;
  std::string interfaceName; // e.g. "Generic USB Audio Device"
  std::string endpointName; // e.g. "Speakers"
  std::string displayName; // e.g. "Generic USB Audio Device (Speakers)"
  AudioDeviceDirection direction;
  AudioDeviceState state;
};

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(AudioDeviceDirection);
AudioDeviceState GetAudioDeviceState(const std::string& id);

std::string GetDefaultAudioDeviceID(AudioDeviceDirection, AudioDeviceRole);
void SetDefaultAudioDeviceID(
  AudioDeviceDirection,
  AudioDeviceRole,
  const std::string& deviceID);

bool IsAudioDeviceMuted(const std::string& deviceID);
void MuteAudioDevice(const std::string& deviceID);
void UnmuteAudioDevice(const std::string& deviceID);

class MuteCallbackHandle {
 public:
  struct Impl;
  MuteCallbackHandle(Impl*);
  ~MuteCallbackHandle();

  MuteCallbackHandle(const MuteCallbackHandle& other) = delete;
  MuteCallbackHandle& operator=(const MuteCallbackHandle& other) = delete;
 private:
  std::unique_ptr<Impl> mImpl;
};

std::unique_ptr<MuteCallbackHandle> AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)>);

typedef void* DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE;
DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE
AddDefaultAudioDeviceChangeCallback(
  std::function<
    void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>);
void RemoveDefaultAudioDeviceChangeCallback(
  DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE);
