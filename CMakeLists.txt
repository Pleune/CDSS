cmake_minimum_required(VERSION 3.1)
project(CDSS)

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIBRARY_NAME        cdss)
set(LIBRARY_NAME_STATIC cdss_static)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH    ${EXECUTABLE_OUTPUT_PATH})

if(WIN32 AND NOT CYGWIN)
  set(INSTALL_CMAKE_DIR CMake)
else()
  set(INSTALL_CMAKE_DIR lib/cmake/cdss)
endif()

add_subdirectory(src)
add_subdirectory(test)

# Ex[prt all targets for use from the build-tree]
export(TARGETS ${LIBRARY_NAME} ${LIBRARY_NAME_STATIC} ${LIBRARY_NAME_STAIC} FILE ${CMAKE_BINARY_DIR}/CDSSTargets.cmake)
# And export to local package database
export(PACKAGE CDSS)

# Create CDSSConfig.cmake files for the build tree,
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/include")
configure_file("${CMAKE_SOURCE_DIR}/cmake/CDSSConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/CDSSConfig.cmake" @ONLY)
# ... and for the install tree
set(CONF_INCLUDE_DIRS "")
configure_file("${CMAKE_SOURCE_DIR}/cmake/CDSSConfig.cmake.in"
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CDSSConfig.cmake" @ONLY)

# Install the CDSSConfig.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CDSSConfig.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT CDSSTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)
