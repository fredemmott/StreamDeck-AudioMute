include(FetchContent)

FetchContent_Declare(
  StreamDeckSDK
  GIT_REPOSITORY https://github.com/fredemmott/StreamDeck-CPPSDK.git
  GIT_TAG 61dc7696f7da6cc73e374935a0b4cc6f9c529148
)

FetchContent_GetProperties(StreamDeckSDK)
if(NOT streamdecksdk_POPULATED)
  FetchContent_Populate(StreamDeckSDK)
  add_subdirectory("${streamdecksdk_SOURCE_DIR}" "${streamdecksdk_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()

if(APPLE)
  set(
    STREAMDECK_PLUGIN_DIR
    "$ENV{HOME}/Library/ApplicationSupport/com.elgato.StreamDeck/Plugins"
  )
elseif(WIN32)
  string(
    REPLACE
    "\\"
    "/"
    STREAMDECK_PLUGIN_DIR
    "$ENV{appdata}/Elgato/StreamDeck/Plugins"
  )
elseif(UNIX AND NOT APPLE)
  target_link_libraries(StreamDeckSDK INTERFACE pthread)
endif()

set(
  STREAMDECK_PLUGIN_DIR
  ${STREAMDECK_PLUGIN_DIR}
  CACHE PATH "Path to this system's streamdeck plugin directory"
)

function(set_default_install_dir_to_streamdeck_plugin_dir)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(
      CMAKE_INSTALL_PREFIX
      "${STREAMDECK_PLUGIN_DIR}/${CMAKE_PROJECT_NAME}"
      CACHE PATH "See cmake documentation"
      FORCE
    )
  endif()
endfunction()
