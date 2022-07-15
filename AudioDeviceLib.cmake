include(FetchContent)

FetchContent_Declare(
  AudioDeviceLib
  GIT_REPOSITORY https://github.com/fredemmott/AudioDeviceLib.git
  GIT_TAG bc759f3a4dc88555b103cfb90f54096d5d33f9cd
)

FetchContent_GetProperties(AudioDeviceLib)
if(NOT audiodevicelib_POPULATED)
  FetchContent_Populate(AudioDeviceLib)
  add_subdirectory("${audiodevicelib_SOURCE_DIR}" "${audiodevicelib_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()
