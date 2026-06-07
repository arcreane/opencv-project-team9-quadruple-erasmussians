#pragma once
#include "operation.hpp"

// CartoonOp: gives a photo a comic / cartoon look. The trick is to treat edges
// and colors separately: detect bold black outlines on one path, flatten the
// colors on another, then multiply them back together. Bilateral filtering is
// the expensive part, so the defaults stay modest to keep the live preview fast.
class CartoonOp : public Operation {
public:
    std::string name() const override { return "Cartoon"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    // Trackbars write straight into these -> mutable; apply() only READS them so
    // it stays pure (same result on the preview and the full-res image).
    mutable int edgeBlock_ = 9; // adaptiveThreshold block size -> outline thickness
    mutable int edgeC_     = 2; // adaptiveThreshold C -> how much detail survives
    mutable int smooth_    = 3; // number of bilateralFilter passes (color flattening)
    mutable int quant_     = 0; // colour levels per channel; 0/1 = quantization off
};
