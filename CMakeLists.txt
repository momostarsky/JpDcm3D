cmake_minimum_required(VERSION 3.8...3.12 FATAL_ERROR)
project(JpDcm3D)

find_package(VTK REQUIRED
  )
if (NOT VTK_FOUND)
  message("Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
else ()
  message("Found VTK ${VTK_DIR}")
endif ()

find_package("Qt${VTK_QT_VERSION}" COMPONENTS Widgets)
if (NOT TARGET "Qt${VTK_QT_VERSION}::Widgets")
  message("Skipping example: ${Qt${VTK_QT_VERSION}_NOT_FOUND_MESSAGE}")
  return ()
endif ()

# Set your files and resources here
set(Srcs
  QtVTKRenderWindowsApp.cxx
  QtVTKRenderWindows.cxx)

set(Hdrs
  QtVTKRenderWindows.h)

set(UIs
  QtVTKRenderWindows.ui
        vtkHelper.cpp
        vtkHelper.h)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_executable(JpDcm3D
  ${Srcs} ${Hdrs} ${UIs})
target_link_libraries(JpDcm3D
  PRIVATE
    ${VTK_LIBRARIES}
    "Qt${VTK_QT_VERSION}::Widgets")
vtk_module_autoinit(
  TARGETS JpDcm3D
  MODULES ${VTK_LIBRARIES})
