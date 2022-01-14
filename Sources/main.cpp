/* Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include <StreamDeckSDK/ESDMain.h>

#include "AudioMuteStreamDeckPlugin.h"

int main(int argc, const char** argv) {
  return esd_main(argc, argv, new AudioMuteStreamDeckPlugin());
}
