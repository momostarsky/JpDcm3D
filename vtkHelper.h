//
// Created by dhz on 24-7-29.
//

#ifndef JPDCM3D_VTKHELPER_H
#define JPDCM3D_VTKHELPER_H


#include <vtkImageActor.h>
#include <vtkViewport.h>

class vtkHelper {
public:
    static void PrintRASDirection(const vtkSmartPointer<vtkImageData> &imageData);

    static void MakeWWWCInfo(double ww, double wc, char *info, size_t maxSize);
    static void MakeSliceInfo(double ww, double wc, int sliceIndex,int sliceNums, double  zoomRate , char *info, size_t maxSize);

    static double ComputeZoomScale(vtkImageActor* actor, vtkViewport *viewport, int dicomImageWidth);
};


#endif //JPDCM3D_VTKHELPER_H
