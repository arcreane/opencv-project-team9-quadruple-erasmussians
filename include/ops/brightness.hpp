#pragma once
#include "operation.hpp"

class BrightnessOp : public Operation {
public:
    std::string name() const override { return "Brightness / Contrast"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    int brightness_ = 100;  // slider 0..200  -> beta  -100..+100
    int contrast_   = 100;  // slider 0..200  -> alpha 0.0..2.0
};
