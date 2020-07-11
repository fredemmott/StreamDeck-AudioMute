/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#include "PlayWavFile.h"

#include "StringEncoding.h"

#include "Windows.h"

#include <string>

namespace FredEmmott::Audio {

void PlayWavFile(const std::string& path) {
  const auto utf16 = FredEmmott::Encoding::Utf8ToUtf16(path);
  PlaySound(utf16.c_str(), NULL, SND_ASYNC| SND_NODEFAULT);
}

}
