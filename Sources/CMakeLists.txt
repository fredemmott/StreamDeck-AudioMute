set(
  SOURCES
  Action.cpp
  DefaultAudioDevices.cpp
  MuteAction.cpp
  MyStreamDeckPlugin.cpp
  ToggleMuteAction.cpp
  UnmuteAction.cpp
  main.cpp
)

if(WIN32)
  list(
    APPEND SOURCES
    StringEncoding.cpp
    PlayWavFileWindows.cpp
    AudioFunctionsWindows.cpp
  )
endif()
if(APPLE)
  list(
    APPEND SOURCES
    PlayWavFileMacOS.cpp
    AudioFunctionsMacOS.cpp
  )
endif()

add_executable(
  sdmicmute
  ${SOURCES}
)

target_link_libraries(sdmicmute StreamDeckSDK)
if(APPLE)
  find_library(COREAUDIO_FRAMEWORK CoreAudio)
  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
  add_executable(audiodevices_test AudioFunctionsMacOS.cpp audiodevices_test.cpp)
  target_link_libraries(audiodevices_test StreamDeckSDK ${COREAUDIO_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
  target_link_libraries(sdmicmute ${COREAUDIO_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
elseif(WIN32)
  target_link_libraries(sdmicmute Winmm)
endif()

set_target_properties(sdmicmute PROPERTIES CXX_STANDARD 20)
target_compile_definitions(sdmicmute PRIVATE -DHAVE_FEEDBACK_SOUNDS=1)
install(TARGETS sdmicmute DESTINATION ${CMAKE_INSTALL_PREFIX})
sign_target(sdmicmute)
