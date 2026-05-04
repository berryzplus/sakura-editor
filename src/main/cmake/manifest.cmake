# manifest.cmake - Generate manifest at build time
#
# arguments(required):
#   ${SOURCE_DIR}
#   ${EXE_NAME}
#   ${EXE_ARCH}
#   ${OUTPUT_FILE}
#

# Configure the file
configure_file(
  ${SOURCE_DIR}/src/main/cmake/manifest.in
  ${OUTPUT_FILE}
  @ONLY
)

message(STATUS "sakura.exe.manifest : ${OUTPUT_FILE}")
message(STATUS "EXE_NAME            : ${EXE_NAME}")
message(STATUS "EXE_ARCH            : ${EXE_ARCH}")
