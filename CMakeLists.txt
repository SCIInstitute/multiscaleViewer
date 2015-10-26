cmake_minimum_required(VERSION 2.8)

PROJECT(multiscale)

find_package(VTK REQUIRED)
set(VTK_DIR /usr/local/lib/cmake/vtk-6.3)

include(${VTK_USE_FILE})
SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++11")
add_executable(multiscale
               MACOSX_BUNDLE
               cartoon
               loadedVolumes
               mouse_interact
               seg3dHandler
               PracticalSocket)

CONFIGURE_FILE("load_volumes.txt" "load_volumes.txt")

if(VTK_LIBRARIES)
  target_link_libraries(multiscale ${VTK_LIBRARIES})
else()
  target_link_libraries(multiscale vtkHybrid vtkWidgets)
endif()
