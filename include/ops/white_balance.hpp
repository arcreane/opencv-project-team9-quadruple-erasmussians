#pragma once
#include "operation.hpp"

//   Method 0 = Gray-World : assume the scene averages to gray, so scale each BGR
//                           channel until its mean matches the overall mean. A
//                           colour cast (e.g. too-warm light) is pulled neutral.
//   Method 1 = Simple WB  : per channel, ignore a few % of the darkest/brightest
//                           pixels and stretch the rest to [0,255]. Uses
//                           cv::xphoto::SimpleWB when opencv_contrib is present,
//                           and a hand-rolled percentile-clip otherwise 

class AutoWhiteBalanceOp : public Operation {
public:
    std::string name() const override { return "Auto White Balance"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    int method_   = 0;    // 0 = Gray-World, 1 = Simple WB (percentile)
    int strength_ = 100;  // 0..100 -> blend original (0) .. fully balanced (1)
};