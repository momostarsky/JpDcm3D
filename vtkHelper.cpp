//
// Created by dhz on 24-7-29.
//

#include <iostream>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkCoordinate.h>
#include <vtkRenderWindow.h>
#include "vtkHelper.h"
#include "vtkMatrix3x3.h"
#include "vtkStdString.h"

void vtkHelper::PrintRASDirection(const vtkSmartPointer<vtkImageData> &imageData) {
    auto matrix = imageData->GetDirectionMatrix();
    double *directionData = matrix->GetData();

    std::cout << "RAS Direction:" << std::endl;

    // 输出 RAS 方向
    if (directionData[0] > 0 && directionData[4] > 0 && directionData[8] > 0) {
        std::cout << "Right: Positive X" << std::endl;
        std::cout << "Anterior: Positive Y" << std::endl;
        std::cout << "Superior: Positive Z" << std::endl;
    } else if (directionData[0] < 0 && directionData[4] > 0 && directionData[8] > 0) {
        std::cout << "Left: Negative X" << std::endl;
        std::cout << "Anterior: Positive Y" << std::endl;
        std::cout << "Superior: Positive Z" << std::endl;
    } else if (directionData[0] > 0 && directionData[4] < 0 && directionData[8] > 0) {
        std::cout << "Right: Positive X" << std::endl;
        std::cout << "Posterior: Negative Y" << std::endl;
        std::cout << "Superior: Positive Z" << std::endl;
    } else if (directionData[0] < 0 && directionData[4] < 0 && directionData[8] > 0) {
        std::cout << "Left: Negative X" << std::endl;
        std::cout << "Posterior: Negative Y" << std::endl;
        std::cout << "Superior: Positive Z" << std::endl;
    } else if (directionData[0] > 0 && directionData[4] > 0 && directionData[8] < 0) {
        std::cout << "Right: Positive X" << std::endl;
        std::cout << "Anterior: Positive Y" << std::endl;
        std::cout << "Inferior: Negative Z" << std::endl;
    } else if (directionData[0] < 0 && directionData[4] > 0 && directionData[8] < 0) {
        std::cout << "Left: Negative X" << std::endl;
        std::cout << "Anterior: Positive Y" << std::endl;
        std::cout << "Inferior: Negative Z" << std::endl;
    } else if (directionData[0] > 0 && directionData[4] < 0 && directionData[8] < 0) {
        std::cout << "Right: Positive X" << std::endl;
        std::cout << "Posterior: Negative Y" << std::endl;
        std::cout << "Inferior: Negative Z" << std::endl;
    } else if (directionData[0] < 0 && directionData[4] < 0 && directionData[8] < 0) {
        std::cout << "Left: Negative X" << std::endl;
        std::cout << "Posterior: Negative Y" << std::endl;
        std::cout << "Inferior: Negative Z" << std::endl;
    }
}

void vtkHelper::MakeWWWCInfo(double ww, double wc, char *info, size_t maxSize) {
    snprintf(info, maxSize, "WW/WC: %.0f/%.0f\n", ww, wc);
}

double vtkHelper::ComputeZoomScale(vtkImageActor *dcmActor, vtkViewport *viewport, int dicomImageWidth) {

    // vtkSmartPointer<vtkImageActor> dcmActor = vtkSmartPointer<vtkImageActor>::New();

// 获取显示的边界
    double *pBounds = dcmActor->GetBounds();
    std::cout << "pBounds: " << pBounds[0] << " " << pBounds[1] << " " << pBounds[2] << " " << pBounds[3] << std::endl;

// 获取右下角坐标
    vtkSmartPointer<vtkCoordinate> coordMax = vtkSmartPointer<vtkCoordinate>::New();
    coordMax->SetCoordinateSystemToWorld();
    coordMax->SetValue(pBounds[1], pBounds[3]);
    int *pDisp = coordMax->GetComputedDisplayValue(viewport);
    std::cout << "pDisp: " << pDisp[0] << " " << pDisp[1] << std::endl;
// 获取左上角坐标
    vtkSmartPointer<vtkCoordinate> coordMin = vtkSmartPointer<vtkCoordinate>::New();
    coordMin->SetCoordinateSystemToWorld();
    coordMin->SetValue(pBounds[0], pBounds[2]);
    int *pDisp1 = coordMin->GetComputedDisplayValue(viewport);

    std::cout << "pDisp1: " << pDisp1[0] << " " << pDisp1[1] << std::endl;

// 计算倍率
    int nWidth = pDisp[0] - pDisp1[0];
    double scale = nWidth * 1.0 / dicomImageWidth;
    std::cout << "scale: " << scale << std::endl;
    return scale;
}

void vtkHelper::MakeSliceInfo(double ww, double wc, int sliceIndex, int sliceNums, double zoomRate, char *info,
                              size_t maxSize) {
    snprintf(info, maxSize, "WW/WC: %.0f/%.0f\nFrame:%d/%d\nZoom:%0.1f", ww, wc, sliceIndex, sliceNums, zoomRate);

}
