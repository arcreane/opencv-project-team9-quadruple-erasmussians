#pragma once
#include "operation.hpp"
#include <vector>

class GeometryOp : public Operation {
public:
    std::string name() const override { return "Geometric Transforms"; }
    void setupTrackbars(const std::string& win) override;
    cv::Mat apply(const cv::Mat& src) const override;

private:
    static void onMouse(int event, int x, int y, int flags, void* userdata);

    int mode_ = 0;
    std::vector<cv::Point2f> ptsNorm_;   
    mutable cv::Size shownSize_{0, 0};   
};