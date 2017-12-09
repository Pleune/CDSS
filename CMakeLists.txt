cmake_minimum_required(VERSION 3.1)
project(CDSS)

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIBRARY_NAME        cdss)
set(LIBRARY_NAME_STATIC cdss_static)
set(TESTER_NAME         cdss_tester)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (LIBRARY_OUTPUT_PATH    ${EXECUTABLE_OUTPUT_PATH})

add_subdirectory(src)
add_subdirectory(test)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/cdss DESTINATION include)

#Export a CDSSConfig.cmake to the user package directory (~/.cmake)
export(TARGETS ${LIBRARY_NAME} ${LIBRARY_NAME_STAIC} FILE ${CMAKE_BINARY_DIR}/CDSSConfig.cmake)
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
