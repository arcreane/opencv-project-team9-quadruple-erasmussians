#pragma once
#include "operation.hpp"

// Class represents cartoon effect operation, inheriting from Operation base class
class CartoonOp : public Operation {
public:
    // Returns name of operation, can be shown in program interface
    std::string name() const override { return "Cartoon"; }

    // Function creates trackbars for cartoon effect settings
    void setupTrackbars(const std::string& controlsWindow) override;

    // Function applies cartoon effect to input image
    cv::Mat apply(const cv::Mat& src) const override;

private:
    // Controls block size used for edge detection
    mutable int edgeBlock_ = 9;

    // Controls edge detail value used in adaptive thresholding
    mutable int edgeC_ = 2;

    // Controls how much smoothing is applied to image
    mutable int smooth_ = 3;

    // Controls number of color levels for cartoon effect: 0 means color quantization is not applied at beginning
    mutable int quant_ = 0;
};
