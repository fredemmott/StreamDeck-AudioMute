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
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
    @ONLY
  )

  list(
    APPEND SOURCES
    PlayWavFileWindows.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
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

target_compile_definitions(sdmicmute PRIVATE -DHAVE_FEEDBACK_SOUNDS=1)
install(TARGETS sdmicmute DESTINATION ${CMAKE_INSTALL_PREFIX})
sign_target(sdmicmute)

if(WIN32)
  install(FILES "$<TARGET_PDB_FILE:sdmicmute>" DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
