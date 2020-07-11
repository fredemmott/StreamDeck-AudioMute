/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include "AudioFunctions.h"
#include <string>

struct DefaultAudioDevices {
  static const std::string DEFAULT_INPUT_ID;
  static const std::string DEFAULT_OUTPUT_ID;
  static const std::string COMMUNICATIONS_INPUT_ID;
  static const std::string COMMUNICATIONS_OUTPUT_ID;

  static std::string GetRealDeviceID(const std::string& dev);
  static std::string GetSpecialDeviceID(AudioDeviceDirection, AudioDeviceRole);
};
