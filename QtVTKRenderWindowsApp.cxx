// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLStereoWidget.h>
#include <QVTKOpenGLNativeWidget.h>
#include "QtVTKRenderWindows.h"
#include <QVTKRenderWidget.h>
#include "MprD1.h"
#include "App3.h"

int main(int argc, char **argv) {

    // needed to ensure appropriate OpenGL context is created for VTK rendering.
    QSurfaceFormat::setDefaultFormat(QVTKRenderWidget::defaultFormat());

    // QT Stuff
    QApplication app(argc, argv);

    RunApp3();

//  QtVTKRenderWindows myQtVTKRenderWindows(argc, argv);
//  myQtVTKRenderWindows.show();

    return QApplication::exec();
}
