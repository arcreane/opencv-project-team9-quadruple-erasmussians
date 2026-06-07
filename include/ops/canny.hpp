#pragma once
#include "operation.hpp"

class CannyOp : public Operation {
public:
    std::string name() const override { return "Canny Edge Detection"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    int threshold1_   = 50;   // lower hysteresis threshold  (0‥255)
    int threshold2_   = 150;  // upper hysteresis threshold  (0‥255)
    int apertureIdx_  = 1;    // 0=3, 1=5, 2=7  → aperture sizes for Sobel
};