#pragma once
#include "operation.hpp"

class UnsharpMaskOp : public Operation {
public:
    std::string name() const override { return "Unsharp Mask"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    int strength_ = 150;  // slider 0..300 -> factor 0.0..3.0
    int radius_   = 3;    // slider 1..15 -> Gaussian blur radius (odd)
};