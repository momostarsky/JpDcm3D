// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLStereoWidget.h>


#include "QtVTKRenderWindows.h"

int main(int argc, char** argv)
{
  // needed to ensure appropriate OpenGL context is created for VTK rendering.
//  QSurfaceFormat::setDefaultFormat(QSurfaceFormat::FormatOption::StereoBuffers);

  // QT Stuff
  QApplication app(argc, argv);

  QtVTKRenderWindows myQtVTKRenderWindows(argc, argv);
  myQtVTKRenderWindows.show();

  return QApplication::exec();
}
