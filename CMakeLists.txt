cmake_minimum_required(VERSION 3.15)

# CMAKE_MSVC_RUNTIME_POLICY support
cmake_policy(SET CMP0091 NEW)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum MacOS version")

if(DEFINED ENV{GITHUB_RUN_NUMBER})
  set(DEFAULT_VERSION_BUILD $ENV{GITHUB_RUN_NUMBER})
else()
  set(DEFAULT_VERSION_BUILD 0)
endif()

set(VERSION_BUILD "${DEFAULT_VERSION_BUILD}" CACHE STRING "patch component of version number")

project(
  com.fredemmott.micmutetoggle.sdPlugin
  VERSION 3.3.0.${VERSION_BUILD}
  LANGUAGES CXX
)
message(STATUS "Configuring v${CMAKE_PROJECT_VERSION}")

if(MSVC)
  # Statically link the MSVC runtime
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  # ... but dynamically link the UCRT - the ABI-stable part
  add_link_options(
    "/DEFAULTLIB:ucrt$<$<CONFIG:Debug>:d>.lib"
    "/NODEFAULTLIB:libucrt$<$<CONFIG:Debug>:d>.lib"
  )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

if(WIN32)
  add_compile_definitions(
    "UNICODE=1"
    "_UNICODE=1"
    "WIN32_LEAN_AND_MEAN=1"
    "WINVER=0xa00" # Windows 10
    "_WIN32_WINNT=0xa00"
  )
endif()

if(MSVC)
  add_compile_options(
    "/diagnostics:caret" # more detailed error messages
    "/EHsc" # standard-compliant exception handling
    "/Zc:__cplusplus" # standard-compliant __cplusplus macro
    "/await:strict" # standard--compliant coroutines
  )
endif()

include("AudioDeviceLib.cmake")
include("StreamDeckSDK.cmake")

set_default_install_dir_to_streamdeck_plugin_dir()

include_directories("${CMAKE_SOURCE_DIR}")

include(sign_target.cmake)
add_subdirectory(Sources)
add_subdirectory(sdPlugin)

install(FILES README.md LICENSE LICENSE.GLYPHICONS LICENSE.AUDIO-FILES DESTINATION ${CMAKE_INSTALL_PREFIX})
