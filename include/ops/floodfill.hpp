#pragma once
#include "operation.hpp"
#include <opencv2/core.hpp>

class FloodFillOp : public Operation {
public:
    std::string name() const override { return "Interactive Flood Fill"; }
    void setupTrackbars(const std::string& controlsWindow) override;
    cv::Mat apply(const cv::Mat& src) const override;

    // Flood Fill commits a new state on every click, so it owns its history.
    bool managesOwnHistory() const override { return true; }

    // Mouse clicks on the image window
    static void onMouseClick(int event, int x, int y, int flags, void* userdata);

private:
    mutable cv::Point seedPoint_ = cv::Point(-1, -1); // Stores the clicked coordinates
    mutable bool isClicked_ = false;                 // Flag to trigger the fill operation
    
    mutable int loDiff_ = 20;   // Lower brightness/color difference threshold
    mutable int upDiff_ = 20;   // Upper brightness/color difference threshold
    mutable int fillR_ = 255;   // Red component of the fill color
    mutable int fillG_ = 0;     // Green component of the fill color
    mutable int fillB_ = 0;     // Blue component of the fill color
};