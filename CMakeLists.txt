cmake_minimum_required(VERSION 3.15)

# CMAKE_MSVC_RUNTIME_POLICY support
cmake_policy(SET CMP0091 NEW)
# Statically link the MSVC runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(
  com.fredemmott.micmutetoggle.sdPlugin
  VERSION 3.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)
if (MSVC)
  add_definitions("/Zc:__cplusplus -DUNICODE=1")
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX
    "${STREAMDECK_PLUGIN_DIR}/${CMAKE_PROJECT_NAME}"
    CACHE PATH "..." FORCE
  )
endif()

include_directories("${CMAKE_SOURCE_DIR}")

include("AudioDeviceLib.cmake")
include("StreamDeckSDK.cmake")

include(sign_target.cmake)
add_subdirectory(Sources)
add_subdirectory(sdPlugin)

install(FILES README.md LICENSE LICENSE.GLYPHICONS LICENSE.AUDIO-FILES DESTINATION ${CMAKE_INSTALL_PREFIX})
