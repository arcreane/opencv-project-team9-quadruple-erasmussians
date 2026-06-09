#pragma once
#include "operation.hpp"

class ThresholdOp : public Operation {
public:
    std::string name() const override { return "Threshold"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    mutable int method_ = 0;
    mutable int thresh_ = 128;
    mutable int block_  = 11;
    mutable int c_      = 2;
};