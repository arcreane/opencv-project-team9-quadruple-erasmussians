#include "ops/histeq.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <vector>

// Function creates trackbars for histogram equalization settings
void HistEqOp::setupTrackbars(const std::string& controlsWindow) {
    // Trackbar selects method: 0 for normal global histogram equalization, 1 for CLAHE method
    cv::createTrackbar("Method 0G/1C", controlsWindow, &method_, 1, appTrackbarCb);

    // Controls clip limit value for CLAHE
    cv::createTrackbar("CLAHE clip", controlsWindow, &clip_, 40, appTrackbarCb);

    // Controls number of tiles used in CLAHE
    cv::createTrackbar("CLAHE tiles", controlsWindow, &tiles_, 16, appTrackbarCb);
}

// Function applies histogram equalization to image
cv::Mat HistEqOp::apply(const cv::Mat& src) const {
    cv::Mat ycrcb;

    // Convert image from BGR to YCrCb color space because histogram equalization should be applied only on brightness
    cv::cvtColor(src, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> ch;

    // Split image into Y, Cr and Cb channels: 
    // ch[0] is Y channel, which represents brightness
    cv::split(ycrcb, ch);

    if (method_ == 1) {
        // CLAHE is local histogram equalization method improving contrast in small regions of image

        // Clip limit is divided by 10 to make trackbar value easier to control
        double clipLimit = std::max(1, clip_) / 10.0;

        // Number of tiles must be at least 1
        int tiles = std::max(1, tiles_);

        // Create CLAHE object with selected clip limit and tile size
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(
            clipLimit,
            cv::Size(tiles, tiles)
        );

        // Apply CLAHE only to brightness channel
        clahe->apply(ch[0], ch[0]);
    } else {
        // Apply normal histogram equalization to brightness channel
        cv::equalizeHist(ch[0], ch[0]);
    }

    // Merge channels back after changing brightness channel
    cv::merge(ch, ycrcb);

    cv::Mat out;

    // Convert image back to BGR so it can be displayed normally
    cv::cvtColor(ycrcb, out, cv::COLOR_YCrCb2BGR);

    return out;
}
