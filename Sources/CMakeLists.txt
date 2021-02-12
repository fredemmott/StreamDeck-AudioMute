set(
  SOURCES
  BaseMuteAction.cpp
  DefaultAudioDevices.cpp
  MuteAction.cpp
  AudioMuteStreamDeckPlugin.cpp
  ToggleMuteAction.cpp
  UnmuteAction.cpp
  main.cpp
)

if(WIN32)
  list(
    APPEND SOURCES
    StringEncoding.cpp
    PlayWavFileWindows.cpp
    AudioDevicesWindows.cpp
  )
endif()
if(APPLE)
  list(
    APPEND SOURCES
    PlayWavFileMacOS.cpp
    AudioDevicesMacOS.cpp
  )
endif()

add_executable(
  sdmicmute
  ${SOURCES}
)

target_link_libraries(sdmicmute StreamDeckSDK)
if(APPLE)
  find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
  find_library(COREAUDIO_FRAMEWORK CoreAudio)
  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
  target_link_libraries(
    sdmicmute
    ${AUDIOTOOLBOX_FRAMEWORK}
    ${COREAUDIO_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
  )
elseif(WIN32)
  target_link_libraries(sdmicmute Winmm)
endif()

set_target_properties(
  sdmicmute
  PROPERTIES
  CXX_STANDARD 20
  CXX_EXTENSIONS OFF
  CXX_STANDARD_REQUIRED ON
)
target_compile_definitions(sdmicmute PRIVATE -DHAVE_FEEDBACK_SOUNDS=1)
install(TARGETS sdmicmute DESTINATION ${CMAKE_INSTALL_PREFIX})
sign_target(sdmicmute)
