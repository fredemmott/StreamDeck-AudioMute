include(ExternalProject)

ExternalProject_Add(
  AudioDeviceLib_build
  URL https://github.com/fredemmott/AudioDeviceLib/releases/download/v1.0beta4/AudioDeviceLib-v1.0beta4.zip
  URL_HASH SHA512=82b6e572f7bedde72b9997369e9169e11e33b8287c52b634ac09082745f265bdeb0b9f82914ac30c59e14af241fc941937e11cc058115d0749e250d837885449
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_Property(
  AudioDeviceLib_build
  INSTALL_DIR
)
add_library(AudioDeviceLib INTERFACE)
add_dependencies(AudioDeviceLib AudioDeviceLib_build)
target_link_libraries(
  AudioDeviceLib
  INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}AudioDeviceLib${CMAKE_STATIC_LIBRARY_SUFFIX}
)
target_include_directories(AudioDeviceLib INTERFACE ${INSTALL_DIR}/include)

if(APPLE)
  find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
  find_library(COREAUDIO_FRAMEWORK CoreAudio)
  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
  target_link_libraries(
    AudioDeviceLib
    INTERFACE
    ${AUDIOTOOLBOX_FRAMEWORK}
    ${COREAUDIO_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
  )
elseif(WIN32)
  target_link_libraries(AudioDeviceLib INTERFACE Winmm)
endif()
