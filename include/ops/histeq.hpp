#pragma once
#include "operation.hpp"

class HistEqOp : public Operation {
public:
    std::string name() const override { return "Histogram Equalization"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    mutable int method_ = 1;
    mutable int clip_   = 20;
    mutable int tiles_  = 8;
};
