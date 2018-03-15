include_directories(${PROJECT_SOURCE_DIR}/packages/LITE-utils/src)
include_directories(${PROJECT_SOURCE_DIR}/sdk-encap)

file(GLOB_RECURSE C_SOURCES "*.c")
add_library(ilop_base OBJECT ${C_SOURCES})

