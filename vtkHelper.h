//
// Created by dhz on 24-7-29.
//

#ifndef JPDCM3D_VTKHELPER_H
#define JPDCM3D_VTKHELPER_H


class vtkHelper {
public:
    static void PrintRASDirection(const vtkSmartPointer<vtkImageData>& imageData);
    static void MakeWWWCInfo(double  ww, double  wc , char* info,size_t maxSize);
};


#endif //JPDCM3D_VTKHELPER_H
