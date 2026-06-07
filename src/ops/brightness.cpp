#pragma once
#include "ops/brightness.hpp"
#include "app.hpp"


void BrightnessOp::setupTrackbars(const std::string& win) {
    cv::createTrackbar("Brightness", win, &brightness_, 200, appTrackbarCb);
    cv::createTrackbar("Contrast", win, &contrast_, 200, appTrackbarCb);
}

cv::Mat BrightnessOp::apply(const cv::Mat& src) const {
    const double alpha = contrast_ / 100.0 ;
    const int    beta = brightness_ - 100 ;   
    cv::Mat out;
    src.convertTo(out, -1, alpha, beta);
    return out;
}