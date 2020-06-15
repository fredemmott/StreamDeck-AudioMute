configure_file(
  manifest.json.in
  manifest.json
  NEWLINE_STYLE UNIX
)

file(GLOB STATIC_FILES *.png *.wav)
install(
  FILES
  ${STATIC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}
)
install(
  DIRECTORY
  propertyinspector
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}
)
