#include "PlayWavFile.h"

#include "StringEncoding.h"

#include "Windows.h"

#include <string>

void PlayWavFile(const std::string& path) {
  const auto utf16 = FredEmmott::Encoding::Utf8ToUtf16(path);
  PlaySound(utf16.c_str(), NULL, SND_ASYNC| SND_NODEFAULT);
}
