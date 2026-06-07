#pragma once
#include "ops/canny.hpp"
#include "app.hpp"

void CannyOp::setupTrackbars(const std::string& win) {
    cv::createTrackbar("Threshold 1", win, &threshold1_,  255, appTrackbarCb);
    cv::createTrackbar("Threshold 2", win, &threshold2_,  255, appTrackbarCb);
    // Aperture: slider 0→3, 1→5, 2→7
    cv::createTrackbar("Aperture(0-2)",win, &apertureIdx_, 2,  appTrackbarCb);
}

cv::Mat CannyOp::apply(const cv::Mat& src) const {
    // Aperture must be an odd number in {3, 5, 7}
    const int aperture = 3 + 2 * std::max(0, std::min(apertureIdx_, 2));

    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src;

    cv::Mat edges;
    cv::Canny(gray, edges, threshold1_, threshold2_, aperture);

    // Return a 3-channel image so the rest of the pipeline stays consistent
    cv::Mat out;
    cv::cvtColor(edges, out, cv::COLOR_GRAY2BGR);
    return out;
}