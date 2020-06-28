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

if(APPLE)
FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
add_executable(audiodevices_test AudioFunctionsMacOS.cpp audiodevices_test.cpp)
target_link_libraries(audiodevices_test StreamDeckSDK ${COREAUDIO_LIBRARY})
endif()

add_executable(
  sdmicmute
  ${SOURCES}
)
target_link_libraries(sdmicmute StreamDeckSDK Winmm)
set_target_properties(sdmicmute PROPERTIES CXX_STANDARD 20)
target_compile_definitions(sdmicmute PRIVATE -DHAVE_FEEDBACK_SOUNDS=1)
install(TARGETS sdmicmute DESTINATION ${CMAKE_INSTALL_PREFIX})
sign_target(sdmicmute)
