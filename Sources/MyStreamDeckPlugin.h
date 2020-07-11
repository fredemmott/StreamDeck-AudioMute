//==============================================================================
/**
@file       MyStreamDeckPlugin.h

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include <StreamDeckSDK/ESDBasePlugin.h>

#include "ESDPlugin.h"

#include <map>
#include <mutex>

class MyStreamDeckPlugin : public ESDPlugin {
 public:
  MyStreamDeckPlugin();
  virtual ~MyStreamDeckPlugin();

 protected:
  std::shared_ptr<ESDAction> GetOrCreateAction(
    const std::string& action,
    const std::string& context);

 private:
  std::mutex mActionsMutex;
  std::map<std::string, std::shared_ptr<ESDAction>> mActions;
};
