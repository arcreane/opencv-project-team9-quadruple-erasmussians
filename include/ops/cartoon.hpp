#pragma once
#include "operation.hpp"

class CartoonOp : public Operation {
public:
    std::string name() const override { return "Cartoon"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    mutable int edgeBlock_ = 9;
    mutable int edgeC_     = 2;
    mutable int smooth_    = 3;
    mutable int quant_     = 0;
};
