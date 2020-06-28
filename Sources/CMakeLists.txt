add_executable(
  sdmicmute
  Action.cpp
  AudioFunctions.cpp
  DefaultAudioDevices.cpp
  MuteAction.cpp
  MyStreamDeckPlugin.cpp
  PlayWavFile.cpp
  StringEncoding.cpp
  ToggleMuteAction.cpp
  UnmuteAction.cpp
  main.cpp
)
target_link_libraries(sdmicmute StreamDeckSDK Winmm)
set_target_properties(sdmicmute PROPERTIES CXX_STANDARD 20)
target_compile_definitions(sdmicmute PRIVATE -DHAVE_FEEDBACK_SOUNDS=1)
install(TARGETS sdmicmute DESTINATION ${CMAKE_INSTALL_PREFIX})
sign_target(sdmicmute)
