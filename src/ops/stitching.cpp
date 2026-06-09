#include "ops/stitching.hpp"
#include "app.hpp"
#include "ui/file_dialog.hpp"
#include <opencv2/stitching.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <string>

// Callback is triggered whenever the trackbar is moved
void StitchingOp::onTrackbarChange(int state, void* userdata) {
    auto* self = static_cast<StitchingOp*>(userdata);
    // When the trackbar is slided to 1, trigger the image stitching workflow
    if (state == 1) {
        self->isTriggered_ = true;
        appTrackbarCb(0, nullptr); // Notify main app to re-render and call apply()
    }
}

void StitchingOp::setupTrackbars(const std::string& controlsWindow) {
    isTriggered_ = false;
    statusMsg_ = "Slide to 1 to load 2nd image.";

    cv::createTrackbar("Trigger Stitch (0->1)", controlsWindow, nullptr, 1, StitchingOp::onTrackbarChange, this);
    cv::setTrackbarPos("Trigger Stitch (0->1)", controlsWindow, 0);
}

cv::Mat StitchingOp::apply(const cv::Mat& src) const {
    // If the trackbar was slided to 1
    if (isTriggered_) {
        isTriggered_ = false; // Reset the workflow trigger immediately
        
        cv::Mat temp;
        statusMsg_ = "Opening file dialog...";
        
        // Call the cross-platform file dialog (Launches AppleScript dialog on Mac)
        if (ui::openImage(temp) && !temp.empty()) {
            secondImg_ = temp;
            statusMsg_ = "2nd image loaded successfully! Stitching...";
        } else {
            statusMsg_ = "Failed or canceled loading.";
        }

        // Snap the trackbar back to 0 on the "Controls" window to reset its visual state
        cv::setTrackbarPos("Trigger Stitch (0->1)", "Controls", 0);
    }

    // If the second image hasn't been loaded yet, render the status message in YELLOW
    if (secondImg_.empty()) {
        cv::Mat out = src.clone();
        std::string hint = "[Stitch] " + statusMsg_;
        cv::putText(out, hint, cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 255, 255), 2);
        return out;
    }

    // Automatically resize the second image if dimensions do not match the source image
    cv::Mat readyImg2;
    if (secondImg_.size() != src.size()) {
        cv::resize(secondImg_, readyImg2, src.size());
    } else {
        readyImg2 = secondImg_;
    }

    std::vector<cv::Mat> imgs = {src, readyImg2};
    cv::Mat panorama;

    // Create OpenCV Panorama Stitcher
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::PANORAMA);
    cv::Stitcher::Status status = stitcher->stitch(imgs, panorama);

    if (status == cv::Stitcher::OK) {
        statusMsg_ = "Stitching successful! Press 's' to save.";
        return panorama;
    } else {
        // Handle various error status codes
        std::string err = "Fail: ";
        if (status == cv::Stitcher::ERR_NEED_MORE_IMGS) {
            err += "Need more images.";
        } else if (status == cv::Stitcher::ERR_HOMOGRAPHY_EST_FAIL) {
            err += "No overlap / features found.";
        } else {
            err += "Error code " + std::to_string(status);
        }
        
        statusMsg_ = err;

        // Render the error status message in RED if stitching fails
        cv::Mat failOut = src.clone();
        std::string hint = "[Stitch] " + statusMsg_;
        cv::putText(failOut, hint, cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 0, 255), 2);
        return failOut;
    }
}