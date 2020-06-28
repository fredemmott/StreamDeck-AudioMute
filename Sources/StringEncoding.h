#pragma once

#include <string>

namespace FredEmmott::Encoding {
  std::string Utf16ToUtf8(const std::wstring& utf16);
  std::wstring Utf8ToUtf16(const std::string& utf8);
}
