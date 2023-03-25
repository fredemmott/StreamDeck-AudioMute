/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#include <string>

#include "PlayWavFile.h"
#include <Windows.h>
#include <Mmsystem.h>

namespace FredEmmott::Audio {

void PlayWavFile(const ESD::filesystem::path& path) {
  PlaySoundW(path.c_str(), NULL, SND_ASYNC | SND_NODEFAULT);
}

}// namespace FredEmmott::Audio
