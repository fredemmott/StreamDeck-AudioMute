#pragma once

#include <string>

struct DefaultAudioDevices {
  static const std::string DEFAULT_INPUT_ID;
  static const std::string DEFAULT_OUTPUT_ID;
  static const std::string COMMUNICATIONS_INPUT_ID;
  static const std::string COMMUNICATIONS_OUTPUT_ID;

  static std::string GetRealDeviceID(const std::string& dev);
};
