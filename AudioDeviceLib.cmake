include(FetchContent)

FetchContent_Declare(
  AudioDeviceLib
  GIT_REPOSITORY https://github.com/fredemmott/AudioDeviceLib.git
  GIT_TAG 2c9e84aad63583cc4618e2b25a703b669de7dd1f
)

FetchContent_GetProperties(AudioDeviceLib)
if(NOT audiodevicelib_POPULATED)
  FetchContent_Populate(AudioDeviceLib)
  add_subdirectory("${audiodevicelib_SOURCE_DIR}" "${audiodevicelib_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()
