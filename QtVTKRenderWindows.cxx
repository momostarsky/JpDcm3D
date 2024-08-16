// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "QtVTKRenderWindows.h"
#include "ui_QtVTKRenderWindows.h"
#include "vtkImageData.h"
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkDistanceRepresentation.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkDistanceWidget.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageSlabReslice.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLookupTable.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkResliceCursor.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include "vtkStringArray.h"
#include "vtkDICOMMetaData.h"
#include "vtkDICOMMetaDataAdapter.h"
#include "vtkHelper.h"
#include <vtkDICOMReader.h>
#include <vtkDICOMDirectory.h>
#include <vtkDICOMItem.h>
#include <vtkMatrix3x3.h>
#include <vtkImageFlip.h>
#include <vtkNamedColors.h>


//------------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand {
public:
    static vtkResliceCursorCallback *New() { return new vtkResliceCursorCallback; }

    void Execute(vtkObject *caller, unsigned long ev, void *callData) override {
        if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
            ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent) {
            // Render everything
            for (int i = 0; i < 3; i++) {
                this->resliceCursorWidget[i]->Render();
            }
            //view4 update
            this->imagePlabeWidget[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }

        vtkImagePlaneWidget *ipw =
                dynamic_cast<vtkImagePlaneWidget *>(caller);
        if (ipw) {
            double *wl = static_cast<double *>(callData);

            if (ipw == this->imagePlabeWidget[0]) {
                this->imagePlabeWidget[1]->SetWindowLevel(wl[0], wl[1], 1);
                this->imagePlabeWidget[2]->SetWindowLevel(wl[0], wl[1], 1);
            } else if (ipw == this->imagePlabeWidget[1]) {
                this->imagePlabeWidget[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->imagePlabeWidget[2]->SetWindowLevel(wl[0], wl[1], 1);
            } else if (ipw == this->imagePlabeWidget[2]) {
                this->imagePlabeWidget[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->imagePlabeWidget[1]->SetWindowLevel(wl[0], wl[1], 1);
            }
        }

        vtkResliceCursorWidget *rcw = dynamic_cast<
                vtkResliceCursorWidget *>(caller);
        if (rcw) {
            vtkResliceCursorLineRepresentation *rep = dynamic_cast<
                    vtkResliceCursorLineRepresentation *>(rcw->GetRepresentation());
            // Although the return value is not used, we keep the get calls
            // in case they had side-effects
            rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
            for (int i = 0; i < 3; i++) {
                vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(
                        this->imagePlabeWidget[i]->GetPolyDataAlgorithm());
                ps->SetOrigin(this->resliceCursorWidget[i]->GetResliceCursorRepresentation()->
                        GetPlaneSource()->GetOrigin());
                ps->SetPoint1(this->resliceCursorWidget[i]->GetResliceCursorRepresentation()->
                        GetPlaneSource()->GetPoint1());
                ps->SetPoint2(this->resliceCursorWidget[i]->GetResliceCursorRepresentation()->
                        GetPlaneSource()->GetPoint2());

                // If the reslice plane has modified, update it on the 3D widget
                this->imagePlabeWidget[i]->UpdatePlacement();
            }
        }

        // Render everything
        for (int i = 0; i < 3; i++) {
            this->resliceCursorWidget[i]->Render();
        }
        this->imagePlabeWidget[0]->GetInteractor()->GetRenderWindow()->Render();

    }

    vtkResliceCursorCallback() = default;

    vtkImagePlaneWidget *imagePlabeWidget[3]{};
    vtkResliceCursorWidget *resliceCursorWidget[3]{};
};

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char *argv[]) {
    this->ui = new Ui_QtVTKRenderWindows;
    this->ui->setupUi(this);

//
//    vtkStringArray *stringArray = reader->GetFileNames();
//    // 获取数组的大小
//    auto  size = stringArray->GetNumberOfValues();
//     std::cout << " siz is :" << size << std::endl;
//
//    // 遍历数组
//    for (int i = 0; i < size; ++i) {
//        std::string value = stringArray->GetValue(i);
//        std::cout << "Value at index " << i << ": " << value << std::endl;
//    }

    //
    //https://dgobbi.github.io/vtk-dicom/doc/api/directory.html
    //
    vtkNew<vtkDICOMDirectory> dicomdir;
    dicomdir->SetDirectoryName("/home/dhz/v4486");
    dicomdir->RequirePixelDataOff();
    dicomdir->Update();

    int n = dicomdir->GetNumberOfSeries();
    if (n == 0) {

        std::cerr << "No DICOM images in directory!" << std::endl;
        return;


    }

    int firstStudy = 0;
    // Get information related to the patient study
    vtkDICOMItem patient = dicomdir->GetPatientRecordForStudy(firstStudy);
    vtkDICOMItem study = dicomdir->GetStudyRecord(firstStudy);
    std::cout << patient.Get(DC::PatientName) << " ";
    std::cout << patient.Get(DC::PatientID) << " ";
    std::cout << study.Get(DC::StudyDate) << " ";
    std::cout << study.Get(DC::StudyTime) << std::endl;

    // Iterate through all the series in this study.
    int j1 = dicomdir->GetFirstSeriesForStudy(firstStudy);
    //  int j2 = dicomdir->GetLastSeriesForStudy(i);
    // get some of the series attributes as a vtkDICOMItem
    vtkDICOMItem series = dicomdir->GetSeriesRecord(j1);
    // get all the files in the series
    vtkStringArray *sortedFiles = dicomdir->GetFileNamesForSeries(j1);
    std::cout << sortedFiles->GetNumberOfValues() << " files: ";
    std::cout << series.Get(DC::SeriesInstanceUID) << std::endl;
    vtkSmartPointer<vtkDICOMReader> xreader = vtkSmartPointer<vtkDICOMReader>::New();
    xreader->SetMemoryRowOrderToFileNative();
    xreader->SetDataByteOrderToLittleEndian();
    xreader->SetFileNames(dicomdir->GetFileNamesForSeries(0));
    xreader->Update();
    vtkSmartPointer<vtkDICOMMetaData> meta = xreader->GetMetaData();
    meta->PrintSelf(std::cout, vtkIndent());

    std::cout << "===========================" << std::endl;
    auto wc = meta->Get(DC::WindowCenter);
    auto ww = meta->Get(DC::WindowWidth);
    auto ImagePositionPatient = meta->Get(DC::ImagePositionPatient);
    auto ImageOrientationPatient = meta->Get(DC::ImageOrientationPatient);
    std::cout << "Window/Level is :" << ww << "," << wc << std::endl;
    std::cout << "ImagePositionPatient is :" << ImagePositionPatient << std::endl;
    std::cout << "ImageOrientationPatient is :" << ImageOrientationPatient << std::endl;

    auto patName = meta->Get(DC::PatientName);
    auto patId = meta->Get(DC::PatientID);
    auto patAge = meta->Get(DC::PatientAge);
    auto patDate = meta->Get(DC::PatientBirthDate);


    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDataByteOrderToLittleEndian();
    reader->SetDirectoryName("/home/dhz/v4486");


//    vtkSmartPointer<vtkImageFlip> flip = vtkSmartPointer<vtkImageFlip>::New();
//    flip->SetInputConnection(reader->GetOutputPort());
//    flip->SetFilteredAxes(1);
    vtkSmartPointer<vtkImageReslice> flip = vtkSmartPointer<vtkImageReslice>::New();
    flip->SetInputConnection(reader->GetOutputPort());
    flip->SetResliceAxesDirectionCosines(
            1, 0, 0,
            0, -1, 0,
            0, 0, -1);

    flip->Update();
    int imageDims[3];
    vtkImageData *imageData = flip->GetOutput();

    imageData->GetDimensions(imageDims);
    {
        float *arr = reader->GetImagePositionPatient();
        std::cout << "ImagePositionPatient: " << arr[0] << ", " << arr[1] << ", " << arr[2] << ")" << std::endl;
        float *arr2 = reader->GetImageOrientationPatient();
        std::cout << "GetImageOrientationPatient:ROW " << arr2[0] << ", " << arr2[1] << ", " << arr2[2] << ")"
                  << std::endl;
        std::cout << "GetImageOrientationPatient:COL  " << arr2[3] << ", " << arr2[4] << ", " << arr2[5] << ")"
                  << std::endl;
        double origin[3] = {0.0, 0.0, 0.0};
        double spacing[3] = {1.0, 1.0, 1.0};

        imageData->GetOrigin(origin);
        imageData->GetSpacing(spacing);
        vtkMatrix3x3 *imageDirection = imageData->GetDirectionMatrix();
        int extentInfo[6];
        imageData->GetExtent(extentInfo);

        std::cout << "Origin: (" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
        std::cout << "Spacing: (" << spacing[0] << ", " << spacing[1] << ", " << spacing[2] << ")" << std::endl;
        std::cout << "Extent: (" << extentInfo[0] << ", " << extentInfo[1] << ", " << extentInfo[2] << ","
                  << extentInfo[3] << ", " << extentInfo[4] << ", " << extentInfo[5] << ")" << std::endl;

    }
    {
        //------------------InitUI -----------------------
        for (int i = 0; i < 3; ++i) {
            mResliceViewer[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
            mResliceRenderWin[i] = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
            mResliceViewer[i]->SetRenderWindow(mResliceRenderWin[i]);
        }

        ui->view1->setRenderWindow(mResliceRenderWin[0]);
        mResliceViewer[0]->SetupInteractor(mResliceRenderWin[0]->GetInteractor());

        ui->view2->setRenderWindow(mResliceRenderWin[1]);
        mResliceViewer[1]->SetupInteractor(mResliceRenderWin[1]->GetInteractor());

        ui->view3->setRenderWindow(mResliceRenderWin[2]);
        mResliceViewer[2]->SetupInteractor(mResliceRenderWin[2]->GetInteractor());

        for (const auto & i : mResliceViewer) {
            // make them all share the same reslice cursor object.
            i->SetResliceCursor(mResliceViewer[0]->GetResliceCursor());
        }

        mPlanePicker = vtkSmartPointer<vtkCellPicker>::New();
        mPlanePicker->SetTolerance(0.005);

        mPlaneRenderWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        ui->view4->setRenderWindow(mPlaneRenderWin);
        mPlaneRender = vtkSmartPointer<vtkRenderer>::New();
        ui->view4->renderWindow()->AddRenderer(mPlaneRender);
        double planeBkColor[3] = {0.5, 0.5, 0.5};
        mPlaneRender->SetBackground(planeBkColor);

        vtkRenderWindowInteractor *iren = ui->view4->interactor();
        mPlaneProperty = vtkSmartPointer<vtkProperty>::New();
        for (int i = 0; i < 3; ++i) {
            mPlaneWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
            mPlaneWidget[i]->SetInteractor(iren);
            mPlaneWidget[i]->SetPicker(mPlanePicker);
            double color[3] = {0.0, 0.0, 0.0};
            color[i] = 1.0;
            mPlaneWidget[i]->GetPlaneProperty()->SetColor(color);

            color[0] /= 4.0;
            color[1] /= 4.0;
            color[2] /= 4.0;
            mResliceViewer[i]->GetRenderer()->SetBackground(color);

            mPlaneWidget[i]->SetTexturePlaneProperty(mPlaneProperty);
            mPlaneWidget[i]->SetDefaultRenderer(mPlaneRender);

            // Make them all share the same color map.
            mResliceViewer[i]->SetLookupTable(mResliceViewer[0]->GetLookupTable());
            mPlaneWidget[i]->GetColorMap()->SetLookupTable(mResliceViewer[0]->GetLookupTable());
            mPlaneWidget[i]->SetColorMap(mResliceViewer[i]->GetResliceCursorWidget()
                                                 ->GetResliceCursorRepresentation()->GetColorMap());
        }

        mResliceCallback = vtkSmartPointer<vtkResliceCursorCallback>::New();
        for (int i = 0; i < 3; ++i) {
            mResliceCallback->imagePlabeWidget[i] = mPlaneWidget[i];
            mResliceCallback->resliceCursorWidget[i] = mResliceViewer[i]->GetResliceCursorWidget();
            mResliceViewer[i]->GetResliceCursorWidget()->AddObserver(
                    vtkResliceCursorWidget::ResliceAxesChangedEvent, mResliceCallback);
            mResliceViewer[i]->GetResliceCursorWidget()->AddObserver(
                    vtkResliceCursorWidget::WindowLevelEvent, mResliceCallback);
            mResliceViewer[i]->GetResliceCursorWidget()->AddObserver(
                    vtkResliceCursorWidget::ResliceThicknessChangedEvent, mResliceCallback);
            mResliceViewer[i]->GetResliceCursorWidget()->AddObserver(
                    vtkResliceCursorWidget::ResetCursorEvent, mResliceCallback);
            mResliceViewer[i]->GetInteractorStyle()->AddObserver(
                    vtkCommand::WindowLevelEvent, mResliceCallback);
        }

        for (const auto &i: mResliceRenderWin) {
            i->Render();
        }
        mPlaneRenderWin->Render();

    }
    {
        //---------------readDicomData
        for (int i = 0; i < 3; ++i) {
            auto rep = vtkResliceCursorLineRepresentation::SafeDownCast(
                    mResliceViewer[i]->GetResliceCursorWidget()->GetRepresentation());
            rep->GetResliceCursorActor()->
                    GetCursorAlgorithm()->SetReslicePlaneNormal(i);

            mResliceViewer[i]->SetInputData(imageData);

            mResliceViewer[i]->SetSliceOrientation(i);
            mResliceViewer[i]->SetResliceModeToAxisAligned();
        }

        for (int i = 0; i < 3; ++i) {
            mPlaneWidget[i]->RestrictPlaneToVolumeOn();

            mPlaneWidget[i]->TextureInterpolateOff();
            mPlaneWidget[i]->SetResliceInterpolateToLinear();
            mPlaneWidget[i]->SetInputConnection(flip->GetOutputPort());
            mPlaneWidget[i]->SetPlaneOrientation(i);
            mPlaneWidget[i]->SetSliceIndex(imageDims[i] / 2);
            mPlaneWidget[i]->DisplayTextOff();
            mPlaneWidget[i]->SetWindowLevel(1358, -27);
            mPlaneWidget[i]->On();
            mPlaneWidget[i]->InteractionOn();
        }

        for (int i = 0; i < 3; ++i) {
            mResliceViewer[i]->GetRenderer()->ResetCamera();
            mResliceRenderWin[i]->Render();
        }
        mPlaneRender->ResetCamera();
        mPlaneRenderWin->Render();

    }
    this->ui->view1->show();
    this->ui->view2->show();
    this->ui->view3->show();
    std::cout << "View1 Height2 :" << ui->view1->height() << std::endl;
    std::cout << "View2 Height2 :" << ui->view2->height() << std::endl;
    std::cout << "View3 Height2 :" << ui->view3->height() << std::endl;
    // Set up action signals and slots
    QMetaObject::Connection cnn = connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
    if (cnn) {
        std::cout << "Connect   successful!" << std::endl;
    }

    cnn = connect(this->ui->btnCrossHair, SIGNAL(clicked(bool)), this, SLOT(slotCrossHair(bool)));
    if (cnn) {
        std::cout << "Connect2   successful!" << std::endl;
    }

//    connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
//    connect(this->ui->thickModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(thickMode(int)));
//    this->ui->thickModeCheckBox->setEnabled(false);
//
//    connect(this->ui->radioButton_Max, SIGNAL(pressed()), this, SLOT(SetBlendModeToMaxIP()));
//    connect(this->ui->radioButton_Min, SIGNAL(pressed()), this, SLOT(SetBlendModeToMinIP()));
//    connect(this->ui->radioButton_Mean, SIGNAL(pressed()), this, SLOT(SetBlendModeToMeanIP()));
//    this->ui->blendModeGroupBox->setEnabled(false);
//
//    connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
//    connect(
//            this->ui->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));
};

void QtVTKRenderWindows::slotExit() {
    qApp->exit();
}

void QtVTKRenderWindows::resliceMode(int mode) {
//    this->ui->thickModeCheckBox->setEnabled(mode != 0);
//    this->ui->blendModeGroupBox->setEnabled(mode != 0);

    for (const auto &i: mResliceViewer) {
        i->SetResliceMode(mode ? 1 : 0);
        i->GetRenderer()->ResetCamera();
        i->Render();
    }
}

void QtVTKRenderWindows::thickMode(int mode) {
    for (const auto &i: mResliceViewer) {
        i->SetThickMode(mode ? 1 : 0);
        if(i->GetThickMode()){
           auto rep = vtkResliceCursorThickLineRepresentation::SafeDownCast(   i->GetResliceCursorWidget()->GetResliceCursorRepresentation());
           rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();
           rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();
           rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();
        }

        i->Render();
    }
}

void QtVTKRenderWindows::SetBlendMode(int m) {
    for (const auto &i: mResliceViewer) {
        vtkImageSlabReslice *thickSlabReslice = vtkImageSlabReslice::SafeDownCast(
                vtkResliceCursorThickLineRepresentation::SafeDownCast(
                        i->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
        thickSlabReslice->SetBlendMode(m);
        i->Render();
    }
}

void QtVTKRenderWindows::SetBlendModeToMaxIP() {
    this->SetBlendMode(VTK_IMAGE_SLAB_MAX);
}

void QtVTKRenderWindows::SetBlendModeToMinIP() {
    this->SetBlendMode(VTK_IMAGE_SLAB_MIN);
}

void QtVTKRenderWindows::SetBlendModeToMeanIP() {
    this->SetBlendMode(VTK_IMAGE_SLAB_MEAN);
}

void QtVTKRenderWindows::ResetViews() {
    // Reset the reslice image views
    // Reset the reslice image views
    for (const auto &i: mResliceViewer) {
        i->Reset();
    }

    // Also sync the Image plane widget on the 3D top right view with any
    // changes to the reslice cursor.
    for (int i = 0; i < 3; ++i) {
        auto *ps = dynamic_cast<vtkPlaneSource *>(
                mPlaneWidget[i]->GetPolyDataAlgorithm());
        ps->SetNormal(mResliceViewer[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
        ps->SetCenter(mResliceViewer[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

        // If the reslice plane has modified, update it on the 3D widget
        mPlaneWidget[i]->UpdatePlacement();
    }

    // Render in response to changes.
    for (const auto &i: mResliceViewer) {
        i->Render();
    }
    ui->view4->renderWindow()->Render();
}

void QtVTKRenderWindows::Render() {
    for (const auto &i: mResliceViewer) {
        i->Render();
    }
    this->ui->view3->renderWindow()->Render();
}

void QtVTKRenderWindows::AddDistanceMeasurementToView1() {
    this->AddDistanceMeasurementToView(1);
}

void QtVTKRenderWindows::AddDistanceMeasurementToView(int i) {
    // remove existing widgets.
    if (this->DistanceWidget[i]) {
        this->DistanceWidget[i]->SetEnabled(0);
        this->DistanceWidget[i] = nullptr;
    }

    // add new widget
    this->DistanceWidget[i] = vtkSmartPointer<vtkDistanceWidget>::New();
    this->DistanceWidget[i]->SetInteractor(this->mResliceViewer[i]->GetResliceCursorWidget()->GetInteractor());

    // Set a priority higher than our reslice cursor widget
    this->DistanceWidget[i]->SetPriority(
            this->mResliceViewer[i]->GetResliceCursorWidget()->GetPriority() + 0.01f);

    vtkSmartPointer<vtkPointHandleRepresentation2D> handleRep =
            vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    vtkSmartPointer<vtkDistanceRepresentation2D> distanceRep =
            vtkSmartPointer<vtkDistanceRepresentation2D>::New();
    distanceRep->SetHandleRepresentation(handleRep);
    this->DistanceWidget[i]->SetRepresentation(distanceRep);
    distanceRep->InstantiateHandleRepresentation();
    distanceRep->GetPoint1Representation()->SetPointPlacer(mResliceViewer[i]->GetPointPlacer());
    distanceRep->GetPoint2Representation()->SetPointPlacer(mResliceViewer[i]->GetPointPlacer());

    // Add the distance to the list of widgets whose visibility is managed based
    // on the reslice plane by the ResliceImageViewerMeasurements class
    this->mResliceViewer[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

    this->DistanceWidget[i]->CreateDefaultRepresentation();
    this->DistanceWidget[i]->EnabledOn();
}

void QtVTKRenderWindows::slotCrossHair(bool mode) {
    std::cout << "CrossHaire" << std::endl;
    resliceMode(1);
    thickMode(1);
    SetBlendModeToMeanIP();

}
