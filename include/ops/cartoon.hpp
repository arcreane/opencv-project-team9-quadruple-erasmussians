#pragma once
#include "operation.hpp"

class CartoonOp : public Operation {
public:
    std::string name() const override { return "Cartoon"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    mutable int edgeBlock_ = 9; // adaptiveThreshold block size -> outline thickness
    mutable int edgeC_     = 2; // adaptiveThreshold C -> how much detail survives
    mutable int smooth_    = 3; // number of bilateralFilter passes (color flattening)
    mutable int quant_     = 0; // colour levels per channel; 0/1 = quantization off
};
