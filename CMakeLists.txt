cmake_minimum_required(VERSION 3.1)
project(CDSS)

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIBRARY_NAME        cdss)
set(LIBRARY_NAME_STATIC cdss_static)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (LIBRARY_OUTPUT_PATH    ${EXECUTABLE_OUTPUT_PATH})

set(CDSS_LIBRARIES cdss)
set(CDSS_INCLUDE_DIRECTORIES ${CMAKE_SRC_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/cdss DESTINATION include)

#Copy the header for the export Config.cmake file, creating a dependency between the src and dest files
configure_file(${CMAKE_SOURCE_DIR}/cmake/CDSSConfig-EXPORT.cmake ${CMAKE_BINARY_DIR}/CDSSConfig.cmake COPYONLY)

export(TARGETS ${LIBRARY_NAME} ${LIBRARY_NAME_STAIC} APPEND FILE ${CMAKE_BINARY_DIR}/CDSSConfig.cmake)
export(PACKAGE CDSS)

set(ConfigPackageLocation lib/cmake/CDSS)
install(EXPORT CDSSTargets
  FILE CDSSTargets.cmake
  NAMESPACE CDSS::
  DESTINATION ${ConfigPackageLocation})
install(
  FILES
    cmake/CDSSConfig.cmake
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)
