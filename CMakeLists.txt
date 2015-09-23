cmake_minimum_required(VERSION 2.8)

PROJECT(cartoon)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++11")
add_executable(cartoon
               MACOSX_BUNDLE
               cartoon
               loadedVolumes
               mouse_interact
               PracticalSocket)

CONFIGURE_FILE("load_volumes.txt" "load_volumes.txt")

if(VTK_LIBRARIES)
  target_link_libraries(cartoon ${VTK_LIBRARIES})
else()
  target_link_libraries(cartoon vtkHybrid vtkWidgets)
endif()
