//
// Created by dhz on 24-8-1.
//

#ifndef JPDCM3DX_JP3DINTERACTORSTYLEIMAGE_H
#define JPDCM3DX_JP3DINTERACTORSTYLEIMAGE_H


#include <vtkInteractorStyleImage.h>
#include <vtkResliceImageViewer.h>

class Jp3DInteractorStyleImage : public vtkInteractorStyleImage {
public:
    static Jp3DInteractorStyleImage *New() {
        return new Jp3DInteractorStyleImage;
    }

    void GetWindowLevel(double &window, double &level) {
        vtkResliceImageViewer *reslice = GetImageReslice();
            window = reslice->GetColorWindow();
            level = reslice->GetColorLevel();
    }

protected:
    vtkResliceImageViewer *GetImageReslice() {
        // 这里你需要实现获取 vtkImageReslice 对象的逻辑
        // 这取决于你的应用程序的具体实现
        return nullptr;
    }
};


#endif //JPDCM3DX_JP3DINTERACTORSTYLEIMAGE_H
