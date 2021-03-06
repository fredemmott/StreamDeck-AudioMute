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
    PlayWavFileWindows.cpp
  )
endif()
if(APPLE)
  list(
    APPEND SOURCES
    PlayWavFileMacOS.cpp
  )
endif()

add_executable(
  sdmicmute
  ${SOURCES}
)

target_link_libraries(sdmicmute StreamDeckSDK AudioDeviceLib)
if(APPLE)
  find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
  target_link_libraries(
    sdmicmute
    ${AUDIOTOOLBOX_FRAMEWORK}
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

if(WIN32)
  install(FILES "$<TARGET_PDB_FILE:sdmicmute>" DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
