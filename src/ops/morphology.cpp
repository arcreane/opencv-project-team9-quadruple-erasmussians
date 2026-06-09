#pragma once
#include "ops/morphology.hpp"
#include "app.hpp"

void MorphologyOp::setupTrackbars(const std::string& win) {
    // Mode: 0=Dilate 1=Erode 2=Open 3=Close 4=Gradient
    cv::createTrackbar("Mode (0-4)", win, &mode_,        4, appTrackbarCb);
    // Kernel size: slider 0-10 → actual size 1,3,5,...,21
    cv::createTrackbar("Kernel size",win, &kernelSize_,  10, appTrackbarCb);
    // Shape: 0=Rect 1=Cross 2=Ellipse
    cv::createTrackbar("Shape (0-2)",win, &kernelShape_, 2,  appTrackbarCb);
}

cv::Mat MorphologyOp::apply(const cv::Mat& src) const {
    // Map slider value to an odd kernel size (1,3,5,...,21)
    // OpenCV requires odd kernel sizes, map slider with 1+2n formula
    const int ksize = 1 + 2 * std::max(0, kernelSize_);

    const int shape = (kernelShape_ == 1) ? cv::MORPH_CROSS
                    : (kernelShape_ == 2) ? cv::MORPH_ELLIPSE
                                          : cv::MORPH_RECT;

    cv::Mat kernel = cv::getStructuringElement(
        shape, cv::Size(ksize, ksize));

    cv::Mat out;
    switch (mode_) {
        case 1:  cv::erode   (src, out, kernel); break;
        case 2:  cv::morphologyEx(src, out, cv::MORPH_OPEN,     kernel); break;
        case 3:  cv::morphologyEx(src, out, cv::MORPH_CLOSE,    kernel); break;
        case 4:  cv::morphologyEx(src, out, cv::MORPH_GRADIENT, kernel); break;
        default: cv::dilate  (src, out, kernel); break;  // 0 = dilate
    }
    return out;
}