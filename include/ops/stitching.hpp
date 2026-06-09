#pragma once
#include "operation.hpp"

class StitchingOp : public Operation {
public:
    std::string name() const override { return "Panorama / Stitching"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

    // Static callback function triggered when the trackbar value changes
    static void onTrackbarChange(int state, void* userdata);

private:
    mutable bool isTriggered_ = false; // Tracks if the trackbar has been moved to 1
    mutable cv::Mat secondImg_;       // Stores the loaded second image
    mutable std::string statusMsg_;   // Status message rendered on the screen
};