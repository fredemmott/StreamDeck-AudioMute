include(FetchContent)

FetchContent_Declare(
  AudioDeviceLib
  GIT_REPOSITORY https://github.com/fredemmott/AudioDeviceLib.git
  GIT_TAG bc759f3a4dc88555b103cfb90f54096d5d33f9cd
  EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(AudioDeviceLib)
