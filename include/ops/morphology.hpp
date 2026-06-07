#pragma once
#include "operation.hpp"

class MorphologyOp : public Operation {
public:
    std::string name() const override { return "Morphology"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    int mode_        = 0;   // 0=dilate 1=erode 2=open 3=close 4=gradient
    int kernelSize_  = 3;   // 1‥21 (odd values only; we map slider→odd)
    int kernelShape_ = 0;   // 0=rect 1=cross 2=ellipse
};