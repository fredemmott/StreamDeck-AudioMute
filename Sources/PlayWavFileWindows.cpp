/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#include <string>

#include "PlayWavFile.h"
#include "Windows.h"

namespace FredEmmott::Audio {

namespace {

std::wstring Utf8ToUtf16(const std::string& utf8) {
  if (utf8.empty()) {
    return std::wstring();
  }
  int wchar_len
    = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(), 0, 0);
  std::wstring buf(wchar_len, 0);
  MultiByteToWideChar(
    CP_UTF8, 0, utf8.data(), utf8.size(), buf.data(), wchar_len);
  return buf;
}

}// namespace

void PlayWavFile(const std::string& path) {
  const auto utf16 = Utf8ToUtf16(path);
  PlaySound(utf16.c_str(), NULL, SND_ASYNC | SND_NODEFAULT);
}

}// namespace FredEmmott::Audio
