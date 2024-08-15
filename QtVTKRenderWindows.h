// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include "vtkDistanceWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkSmartPointer.h"
#include <QMainWindow>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkResliceCursorRepresentation.h>
#include <vtkResliceCursorLineRepresentation.h>
#include "vtkTextActor.h"
#include "vtkTextProperty.h"

// Forward Qt class declarations
class Ui_QtVTKRenderWindows;

class QtVTKRenderWindows : public QMainWindow {
Q_OBJECT
public:
    // Constructor/Destructor
    QtVTKRenderWindows(int argc, char *argv[]);

    ~QtVTKRenderWindows() override = default;

protected   slots:
    virtual void slotExit();
    virtual void slotCrossHair(bool );

    virtual void resliceMode(int);

    virtual void thickMode(int);

    virtual void SetBlendModeToMaxIP();

    virtual void SetBlendModeToMinIP();

    virtual void SetBlendModeToMeanIP();

    virtual void SetBlendMode(int);

    virtual void ResetViews();

    virtual void Render();

    virtual void AddDistanceMeasurementToView1();

    virtual void AddDistanceMeasurementToView(int);

protected:
    vtkSmartPointer<vtkResliceImageViewer> riw[3];
    vtkSmartPointer<vtkResliceCursorWidget> rcw[3];
    vtkSmartPointer<vtkResliceCursorRepresentation>          mRep[3]  ;
    vtkSmartPointer<vtkResliceCursorLineRepresentation>      mResliceCursorRep[3]  ;

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin[3];
    vtkSmartPointer<vtkTextActor> textActor[3];                                     //文本信息
    vtkSmartPointer<vtkTextActor> pLeftTopTextActor[3];                          //文本信息
    vtkSmartPointer<vtkTextActor> pWWWCInfo[3];                          //文本信息

    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
    vtkSmartPointer<vtkDistanceWidget> DistanceWidget[3];
    vtkSmartPointer<vtkResliceImageViewerMeasurements> ResliceMeasurements;
    int dicomImageWidth;
    int dicomImageHeight;


private:
    // Designer form
    Ui_QtVTKRenderWindows *ui;
};

#endif // QtVTKRenderWindows_H
