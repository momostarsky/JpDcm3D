//
// Created by dhz on 24-7-29.
//

#include <iostream>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
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

void vtkHelper::MakeWWWCInfo(double ww, double wc, char *info) {
    sprintf(info, "WW/WC: %.0f/%.0f\n", ww, wc);
}
