#pragma once
#include "ops/unsharpmask.hpp"
#include "app.hpp"

void UnsharpMaskOp::setupTrackbars(const std::string& win) {
    // Strength: slider 0-300 -> sharpening factor 0.0..3.0
    cv::createTrackbar("Strength", win, &strength_, 300, appTrackbarCb);
    // Radius: slider 1-15 (we force odd in apply)
    cv::createTrackbar("Radius",   win, &radius_,   15,  appTrackbarCb);
}

cv::Mat UnsharpMaskOp::apply(const cv::Mat& src) const {
    // Force radius to be at least 1 and odd
    int r = std::max(1, radius_);
    if (r % 2 == 0) r++;

    const double amount = strength_ / 100.0;  // 0.0 .. 3.0

    // Blur the image
    cv::Mat blurred;
    cv::GaussianBlur(src, blurred, cv::Size(r * 2 + 1, r * 2 + 1), 0);

    // out = src + amount * (src - blurred)
    cv::Mat mask;
    cv::subtract(src, blurred, mask);

    cv::Mat out;
    cv::addWeighted(src, 1.0, mask, amount, 0, out);

    return out;
}