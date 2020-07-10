#include "PlayWavFile.h"

#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>

#include <map>

void PlayWavFile(const std::string& path) {
  static std::map<std::string, SystemSoundID> ids;
  const auto it = ids.find(path);
  if (it == ids.end()) {
    CFStringRef cfpath
      = CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8);
    CFURLRef cfurl
      = CFURLCreateWithFileSystemPath(NULL, cfpath, kCFURLPOSIXPathStyle, false);
    SystemSoundID id;
    AudioServicesCreateSystemSoundID(cfurl, &id);
    ids.emplace(path, id);

    CFRelease(cfurl);
    CFRelease(cfpath);
  }
  const auto id = ids.at(path);
  AudioServicesPlaySystemSound(id);
}
