#include <QApplication>
#include <vtkAutoInit.h>

//VTK_MODULE_INIT(vtkRenderingOpenGL2);
//VTK_MODULE_INIT(vtkInteractionStyle);
//VTK_MODULE_INIT(vtkRenderingFreeType);
//VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);

#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkJPEGReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkNIFTIImageReader.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorWidget.h>
#include <vtkWidgetRepresentation.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkNIFTIImageWriter.h>
#include "vtkActor.h"
#include "vtkBiDimensionalWidget.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkImagePlaneWidget.h"
#include "vtkImageReader.h"
#include "vtkImageReslice.h"
#include "vtkInteractorEventRecorder.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLookupTable.h"
#include "vtkOutlineFilter.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkResliceCursor.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursorThickLineRepresentation.h"

class vtkResliceCursorCallback3 : public vtkCommand
{
public:
    static vtkResliceCursorCallback3* New() { return new vtkResliceCursorCallback3; }

    void Execute(vtkObject* caller, unsigned long , void* callData) override
    {
        auto* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
        if (ipw)
        {
            auto* wl = static_cast<double*>(callData);

            if (ipw == this->IPW[0])
            {
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[1])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[2])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
            }
        }

        auto* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
        if (rcw)
        {
            auto* rep =
                    dynamic_cast<vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
            vtkResliceCursor* rc = rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
            for (int i = 0; i < 3; i++)
            {
                auto* ps = dynamic_cast<vtkPlaneSource*>(this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetNormal(rc->GetPlane(i)->GetNormal());
                ps->SetCenter(rc->GetPlane(i)->GetOrigin());

                // If the reslice plane has modified, update it on the 3D widget
                this->IPW[i]->UpdatePlacement();

                // std::cout << "Updating placement of plane: " << i << " " <<
                //  rc->GetPlane(i)->GetNormal()[0] << " " <<
                //  rc->GetPlane(i)->GetNormal()[1] << " " <<
                //  rc->GetPlane(i)->GetNormal()[2] << std::endl;
                // this->IPW[i]->GetReslice()->Print(cout);
                // rep->GetReslice()->Print(cout);
                // std::cout << "---------------------" << std::endl;
            }
        }

        // Render everything
        this->RCW[0]->Render();
    }

    vtkResliceCursorCallback3() = default;
    vtkImagePlaneWidget* IPW[3]{nullptr};
    vtkResliceCursorWidget* RCW[3]{nullptr};
};

void  RunApp3( )
{

    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDataByteOrderToLittleEndian();
    reader->SetDirectoryName("/home/dhz/v4486");
    reader->Update();

    vtkSmartPointer<vtkOutlineFilter> outline = vtkSmartPointer<vtkOutlineFilter>::New();
    outline->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outline->GetOutputPort());

    vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(outlineMapper);

    vtkSmartPointer<vtkRenderer> ren[4];

    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
    renWin->SetMultiSamples(0);

    for (auto & i : ren)
    {
        i = vtkSmartPointer<vtkRenderer>::New();
        renWin->AddRenderer(i);
    }

    vtkSmartPointer<vtkRenderWindowInteractor> iren =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    // assign default props to the ipw's texture plane actor
    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims);

    for (int i = 0; i < 3; i++)
    {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i]->SetInteractor(iren);
        planeWidget[i]->SetPicker(picker);
        planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = { 0, 0, 0 };
        color[i] = 1;
        planeWidget[i]->GetPlaneProperty()->SetColor(color);
        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToLinear();
        planeWidget[i]->SetInputConnection(reader->GetOutputPort());
        planeWidget[i]->SetPlaneOrientation(i);
        planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
        planeWidget[i]->DisplayTextOn();
        planeWidget[i]->SetDefaultRenderer(ren[3]);
        planeWidget[i]->SetWindowLevel(1358, -27);
        planeWidget[i]->On();
        planeWidget[i]->InteractionOn();
    }

    planeWidget[1]->SetLookupTable(planeWidget[0]->GetLookupTable());
    planeWidget[2]->SetLookupTable(planeWidget[0]->GetLookupTable());

    vtkSmartPointer<vtkResliceCursorCallback3> cbk =
            vtkSmartPointer<vtkResliceCursorCallback3>::New();

    // Create the reslice cursor, widget and rep

    vtkSmartPointer<vtkResliceCursor> resliceCursor = vtkSmartPointer<vtkResliceCursor>::New();
    resliceCursor->SetCenter(reader->GetOutput()->GetCenter());
    resliceCursor->SetThickMode(1);
    resliceCursor->SetThickness(10, 10, 10);
    resliceCursor->SetImage(reader->GetOutput());

    vtkSmartPointer<vtkResliceCursorWidget> ResliceCursorWidget[3];
    vtkSmartPointer<vtkResliceCursorThickLineRepresentation> resliceCursorRep[3];

    double viewUp[3][3] = { { 0, 0, -1 },{ 0, 0, 1 },{ 0, 1, 0 } };
    for (int i = 0; i < 3; i++)
    {
        ResliceCursorWidget[i] = vtkSmartPointer<vtkResliceCursorWidget>::New();
        ResliceCursorWidget[i]->SetInteractor(iren);

        resliceCursorRep[i] = vtkSmartPointer<vtkResliceCursorThickLineRepresentation>::New();
        ResliceCursorWidget[i]->SetRepresentation(resliceCursorRep[i]);
        resliceCursorRep[i]->GetResliceCursorActor()->GetCursorAlgorithm()->SetResliceCursor(
                resliceCursor);
        resliceCursorRep[i]->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);


        resliceCursorRep[i]->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();
        resliceCursorRep[i]->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();
        resliceCursorRep[i]->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();


        const double minVal = reader->GetOutput()->GetScalarRange()[0];
        if (vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(resliceCursorRep[i]->GetReslice()))
        {
            reslice->SetBackgroundColor(minVal, minVal, minVal, minVal);
        }

        ResliceCursorWidget[i]->SetDefaultRenderer(ren[i]);
        ResliceCursorWidget[i]->SetEnabled(1);

        ren[i]->GetActiveCamera()->SetFocalPoint(0, 0, 0);
        double camPos[3] = { 0, 0, 0 };
        camPos[i] = 1;
        ren[i]->GetActiveCamera()->SetPosition(camPos);

        ren[i]->GetActiveCamera()->ParallelProjectionOn();
        ren[i]->GetActiveCamera()->SetViewUp(viewUp[i]);
        ren[i]->ResetCamera();
        // ren[i]->ResetCameraClippingRange();

        // Tie the Image plane widget and the reslice cursor widget together
        cbk->IPW[i] = planeWidget[i];
        cbk->RCW[i] = ResliceCursorWidget[i];
        ResliceCursorWidget[i]->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);

        // Initialize the window level to a sensible value
        double range[2];
        reader->GetOutput()->GetScalarRange(range);
        resliceCursorRep[i]->SetWindowLevel(range[1] - range[0], (range[0] + range[1]) / 2.0);
        planeWidget[i]->SetWindowLevel(range[1] - range[0], (range[0] + range[1]) / 2.0);

        // Make them all share the same color map.
        resliceCursorRep[i]->SetLookupTable(resliceCursorRep[0]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(resliceCursorRep[0]->GetLookupTable());

        ResliceCursorWidget[i]->On();
    }

    // Add the actors
    //
    ren[0]->SetBackground(0.3, 0.1, 0.1);
    ren[1]->SetBackground(0.1, 0.3, 0.1);
    ren[2]->SetBackground(0.1, 0.1, 0.3);
    ren[3]->AddActor(outlineActor);
    ren[3]->SetBackground(0.1, 0.1, 0.1);
    renWin->SetSize(600, 600);
    // renWin->SetFullScreen(1);

    ren[0]->SetViewport(0, 0, 0.5, 0.5);
    ren[1]->SetViewport(0.5, 0, 1, 0.5);
    ren[2]->SetViewport(0, 0.5, 0.5, 1);
    ren[3]->SetViewport(0.5, 0.5, 1, 1);

    // Set the actors' positions
    //
    renWin->Render();

    ren[3]->GetActiveCamera()->Elevation(110);
    ren[3]->GetActiveCamera()->SetViewUp(0, 0, -1);
    ren[3]->GetActiveCamera()->Azimuth(45);
    ren[3]->GetActiveCamera()->Dolly(1.15);
    ren[3]->ResetCameraClippingRange();

    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    iren->SetInteractorStyle(style);

    iren->Initialize();
    iren->Start();

}
