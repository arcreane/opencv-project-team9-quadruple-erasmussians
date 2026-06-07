#pragma once
#include "operation.hpp"

class PencilSketchOp : public Operation {
public:
    std::string name() const override { return "Pencil Sketch"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    int mode_      = 0;    // 0=grayscale sketch  1=colour sketch
    int blurRadius_= 10;   // slider 1..30 -> controls line thickness
    int shadeFactor_ = 25; // slider 1..100 -> shade intensity for colour mode
};