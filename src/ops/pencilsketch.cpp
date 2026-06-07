#pragma once
#include "ops/pencilsketch.hpp"
#include "app.hpp"

void PencilSketchOp::setupTrackbars(const std::string& win) {
    cv::createTrackbar("Mode 0G 1C",   win, &mode_,        1,   appTrackbarCb);
    cv::createTrackbar("Blur radius",  win, &blurRadius_,  30,  appTrackbarCb);
    cv::createTrackbar("Shade(colour)",win, &shadeFactor_, 100, appTrackbarCb);
    // Set initial slider positions to safe non-zero defaults
    cv::setTrackbarPos("Blur radius",   win, 10);
    cv::setTrackbarPos("Shade(colour)", win, 25);
}

cv::Mat PencilSketchOp::apply(const cv::Mat& src) const {
    // sigma_s must be > 0, clamp to safe range 1..200
    const float sigma_s = static_cast<float>(std::max(1, std::min(blurRadius_, 200)));
    // shade_factor must be in (0,0.1] range for pencilSketch
    const float shade   = std::max(0.01f, std::min(shadeFactor_ / 100.0f, 0.1f));

    try {
        if (mode_ == 0) {
            cv::Mat graySketch, colourSketch;
            cv::pencilSketch(src, graySketch, colourSketch, sigma_s, shade, 0.07f);
            cv::Mat out;
            cv::cvtColor(graySketch, out, cv::COLOR_GRAY2BGR);
            return out;
        } else {
            cv::Mat graySketch, colourSketch;
            cv::pencilSketch(src, graySketch, colourSketch, sigma_s, shade, 0.07f);
            return colourSketch;
        }
    } catch (...) {
        // If pencilSketch fails for any reason, return original
        return src.clone();
    }
}