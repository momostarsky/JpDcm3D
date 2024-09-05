cmake_minimum_required(VERSION 3.8...3.12 FATAL_ERROR)
project(JpDcm3DX)

find_package(VTK REQUIRED )



if (NOT VTK_FOUND)
  message("Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
else ()

endif ()

find_package(VTKm REQUIRED)
message("Found VTKm_ENABLE_RENDERING： ${VTKm_ENABLE_RENDERING}")

if ("ON"  STREQUAL  "${VTKm_ENABLE_RENDERING}")
  message(STATUS "VTK-m  built with rendering on.")
endif()

message(STATUS "Qt5")

find_package("Qt5" COMPONENTS Widgets)
if (NOT TARGET "Qt5::Widgets")
  message("Skipping example: ${Qt5_NOT_FOUND_MESSAGE}")
  return ()
endif ()

# Set your files and resources here
set(SRCS
  QtVTKRenderWindowsApp.cxx
  QtVTKRenderWindows.cxx  vtkHelper.cpp)

set(HEADERS   App3.h
  QtVTKRenderWindows.h    vtkHelper.h
        MprD1.h)

set(UIS
  QtVTKRenderWindows.ui
        Jp3DInteractorStyleImage.h)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# CMAKE_AUTOMOC in ON so the MocHEADERS will be automatically wrapped.
add_executable(JpDcm3DX  ${SRCS} ${HEADERS} ${UIS})
target_link_libraries(JpDcm3DX
  PRIVATE   ${VTK_LIBRARIES}
            "Qt5::Widgets"  ${VTKm_LIBRARIES} )
vtk_module_autoinit(
  TARGETS JpDcm3DX
  MODULES ${VTK_LIBRARIES} )
