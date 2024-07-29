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


//------------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand {
public:
    static vtkResliceCursorCallback *New() { return new vtkResliceCursorCallback; }

    void Execute(vtkObject *caller, unsigned long ev, void *callData) override {

        if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent) {
            // Render everything
            for (auto &i: this->RCW) {
                i->Render();
            }
            this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }

        auto *ipw = dynamic_cast<vtkImagePlaneWidget *>(caller);
        if (ipw) {
            auto *wl = static_cast<double *>(callData);

            if (ipw == this->IPW[0]) {
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            } else if (ipw == this->IPW[1]) {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            } else if (ipw == this->IPW[2]) {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
            }
        }

        auto *rcw = dynamic_cast<vtkResliceCursorWidget *>(caller);
        if (rcw) {
            auto *rep =
                    dynamic_cast<vtkResliceCursorLineRepresentation *>(rcw->GetRepresentation());
            // Although the return value is not used, we keep the get calls
            // in case they had side effects
            rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
            for (int i = 0; i < 3; i++) {
                auto *ps = dynamic_cast<vtkPlaneSource *>(this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetOrigin(
                        this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
                ps->SetPoint1(
                        this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
                ps->SetPoint2(
                        this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());

                // If the reslice plane has modified, update it on the 3D widget
                this->IPW[i]->UpdatePlacement();
            }
        }

        // Render everything
        for (auto &i: this->RCW) {
            i->Render();
        }
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
    }

    vtkResliceCursorCallback() = default;

    vtkImagePlaneWidget *IPW[3]{};
    vtkResliceCursorWidget *RCW[3]{};
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


    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDataByteOrderToLittleEndian();
    reader->SetDirectoryName("/home/dhz/v4486");
    reader->Update();

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

        std::cout << "Origin: (" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
        std::cout << "Spacing: (" << spacing[0] << ", " << spacing[1] << ", " << spacing[2] << ")" << std::endl;
        std::cout << "Direction matrix:" << std::endl;
        imageDirection->Print(std::cout);
//        for (int i = 0; i < 3; ++i)
//        {
//            for (int j = 0; j < 3; ++j)
//            {
//                std::cout << imageDirection[i * 3 + j] << " ";
//            }
//            std::cout << std::endl;
//        }
        vtkHelper::PrintRASDirection(imageData);
    }

    for (auto &i: riw) {
        i = vtkSmartPointer<vtkResliceImageViewer>::New();
        vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        i->SetRenderWindow(renderWindow);

    }

    this->ui->view1->setRenderWindow(riw[0]->GetRenderWindow());
    riw[0]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());

    this->ui->view2->setRenderWindow(riw[1]->GetRenderWindow());
    riw[1]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

    this->ui->view3->setRenderWindow(riw[2]->GetRenderWindow());
    riw[2]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

    for (int i = 0; i < 3; i++) {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(
                riw[i]->GetResliceCursorWidget()->GetRepresentation());
        riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

        riw[i]->SetInputData(imageData);
        riw[i]->SetSliceOrientation(i);
        riw[i]->SetResliceModeToAxisAligned();
    }

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    this->ui->view4->setRenderWindow(renderWindow);
    this->ui->view4->renderWindow()->AddRenderer(ren);
    vtkRenderWindowInteractor *iren = this->ui->view4->interactor();

    for (int i = 0; i < 3; i++) {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i]->SetInteractor(iren);
        planeWidget[i]->SetPicker(picker);
        planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = {0, 0, 0};
        color[i] = 0;
        planeWidget[i]->GetPlaneProperty()->SetColor(color);

        color[0] /= 0.0;
        color[1] /= 0.0;
        color[2] /= 0.0;
        riw[i]->GetRenderer()->SetBackground(color);

        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToNearestNeighbour();
        planeWidget[i]->SetInputConnection(reader->GetOutputPort());
        planeWidget[i]->SetPlaneOrientation(i);
        planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
        planeWidget[i]->DisplayTextOn();
        planeWidget[i]->SetDefaultRenderer(ren);

//        planeWidget[i]->SetWindowLevel(wc.AsDouble(), ww.AsDouble());



        planeWidget[i]->On();
        planeWidget[i]->InteractionOn();

    }

    vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

    for (int i = 0; i < 3; i++) {
        cbk->IPW[i] = planeWidget[i];
        cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
        riw[i]->GetResliceCursorWidget()->AddObserver(
                vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(
                vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
        riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);
        riw[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, cbk);

        // Make them all share the same color map.
        riw[i]->SetLookupTable(riw[0]->GetLookupTable());
        riw[i]->SetColorLevel(wc.AsDouble());
        riw[i]->SetColorWindow(ww.AsDouble());

        planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
        // planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
        planeWidget[i]->SetColorMap(
                riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
        riw[i]->Render();
    }

    this->ui->view1->show();
    this->ui->view2->show();
    this->ui->view3->show();

    // Set up action signals and slots
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
    connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
    connect(this->ui->thickModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(thickMode(int)));
    this->ui->thickModeCheckBox->setEnabled(false);

    connect(this->ui->radioButton_Max, SIGNAL(pressed()), this, SLOT(SetBlendModeToMaxIP()));
    connect(this->ui->radioButton_Min, SIGNAL(pressed()), this, SLOT(SetBlendModeToMinIP()));
    connect(this->ui->radioButton_Mean, SIGNAL(pressed()), this, SLOT(SetBlendModeToMeanIP()));
    this->ui->blendModeGroupBox->setEnabled(false);

    connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
    connect(
            this->ui->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));
};

void QtVTKRenderWindows::slotExit() {
    qApp->exit();
}

void QtVTKRenderWindows::resliceMode(int mode) {
    this->ui->thickModeCheckBox->setEnabled(mode != 0);
    this->ui->blendModeGroupBox->setEnabled(mode != 0);

    for (const auto &i: riw) {
        i->SetResliceMode(mode ? 1 : 0);
        i->GetRenderer()->ResetCamera();
        i->Render();
    }
}

void QtVTKRenderWindows::thickMode(int mode) {
    for (const auto &i: riw) {
        i->SetThickMode(mode ? 1 : 0);
        i->Render();
    }
}

void QtVTKRenderWindows::SetBlendMode(int m) {
    for (const auto &i: riw) {
        vtkImageSlabReslice *thickSlabReslice =
                vtkImageSlabReslice::SafeDownCast(vtkResliceCursorThickLineRepresentation::SafeDownCast(
                        i->GetResliceCursorWidget()->GetRepresentation())
                                                          ->GetReslice());
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
    for (const auto &i: riw) {
        i->Reset();
    }

    // Also sync the Image plane widget on the 3D top right view with any
    // changes to the reslice cursor.
    for (int i = 0; i < 3; i++) {
        auto *ps = dynamic_cast<vtkPlaneSource *>(planeWidget[i]->GetPolyDataAlgorithm());
        ps->SetNormal(riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
        ps->SetCenter(riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

        // If the reslice plane has modified, update it on the 3D widget
        this->planeWidget[i]->UpdatePlacement();
    }

    // Render in response to changes.
    this->Render();
}

void QtVTKRenderWindows::Render() {
    for (const auto &i: riw) {
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
    this->DistanceWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());

    // Set a priority higher than our reslice cursor widget
    this->DistanceWidget[i]->SetPriority(
            this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01f);

    vtkSmartPointer<vtkPointHandleRepresentation2D> handleRep =
            vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    vtkSmartPointer<vtkDistanceRepresentation2D> distanceRep =
            vtkSmartPointer<vtkDistanceRepresentation2D>::New();
    distanceRep->SetHandleRepresentation(handleRep);
    this->DistanceWidget[i]->SetRepresentation(distanceRep);
    distanceRep->InstantiateHandleRepresentation();
    distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
    distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

    // Add the distance to the list of widgets whose visibility is managed based
    // on the reslice plane by the ResliceImageViewerMeasurements class
    this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

    this->DistanceWidget[i]->CreateDefaultRepresentation();
    this->DistanceWidget[i]->EnabledOn();
}
