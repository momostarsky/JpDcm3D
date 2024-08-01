cmake_minimum_required(VERSION 3.8...3.12 FATAL_ERROR)
project(JpDcm3DX)

find_package(VTK REQUIRED )
if (NOT VTK_FOUND)
  message("Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
else ()
  message("Found VTK ${VTK_DIR}")
endif ()

message(STATUS "Qt${VTK_QT_VERSION}")

find_package("Qt${VTK_QT_VERSION}" COMPONENTS Widgets)
if (NOT TARGET "Qt${VTK_QT_VERSION}::Widgets")
  message("Skipping example: ${Qt${VTK_QT_VERSION}_NOT_FOUND_MESSAGE}")
  return ()
endif ()

# Set your files and resources here
set(SRCS
  QtVTKRenderWindowsApp.cxx
  QtVTKRenderWindows.cxx  vtkHelper.cpp)

set(HEADERS
  QtVTKRenderWindows.h    vtkHelper.h)

set(UIS
  QtVTKRenderWindows.ui )

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# CMAKE_AUTOMOC in ON so the MocHEADERS will be automatically wrapped.
add_executable(JpDcm3DX  ${SRCS} ${HEADERS} ${UIS})
target_link_libraries(JpDcm3DX
  PRIVATE   ${VTK_LIBRARIES}
            "Qt${VTK_QT_VERSION}::Widgets")
vtk_module_autoinit(
  TARGETS JpDcm3DX
  MODULES ${VTK_LIBRARIES})
