#include "ops/floodfill.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <string>

extern const char* kMainWindow;
extern void commitToHistory(const cv::Mat& currentPreviewImage);

// Triggered whenever the user clicks on the main image window
void FloodFillOp::onMouseClick(int event, int x, int y, int /*flags*/, void* userdata) {
    // Left mouse button press
    if (event == cv::EVENT_LBUTTONDOWN) {

        auto* self = static_cast<FloodFillOp*>(userdata);
        self->seedPoint_ = cv::Point(x, y);
        self->isClicked_ = true;
        appTrackbarCb(0, nullptr); // Notify the main app to re-render and call apply()
    }
}

void FloodFillOp::setupTrackbars(const std::string& controlsWindow) {
    isClicked_ = false;
    seedPoint_ = cv::Point(-1, -1);

    // Create trackbars for thresholds
    cv::createTrackbar("Lower Diff", controlsWindow, nullptr, 255, nullptr, nullptr);
    cv::setTrackbarPos("Lower Diff", controlsWindow, loDiff_);

    cv::createTrackbar("Upper Diff", controlsWindow, nullptr, 255, nullptr, nullptr);
    cv::setTrackbarPos("Upper Diff", controlsWindow, upDiff_);

    // Create trackbars for the fill color (RGB)
    cv::createTrackbar("Fill - R", controlsWindow, nullptr, 255, nullptr, nullptr);
    cv::setTrackbarPos("Fill - R", controlsWindow, fillR_);

    cv::createTrackbar("Fill - G", controlsWindow, nullptr, 255, nullptr, nullptr);
    cv::setTrackbarPos("Fill - G", controlsWindow, fillG_);

    cv::createTrackbar("Fill - B", controlsWindow, nullptr, 255, nullptr, nullptr);
    cv::setTrackbarPos("Fill - B", controlsWindow, fillB_);
}

cv::Mat FloodFillOp::apply(const cv::Mat& src) const {
    cv::Mat out = src.clone();

    static bool isMouseBound = false;
    if (!isMouseBound) {
        cv::setMouseCallback(kMainWindow, FloodFillOp::onMouseClick, const_cast<FloodFillOp*>(this));
        isMouseBound = true; 
    }

    // Fetch the latest parameters 
    loDiff_ = cv::getTrackbarPos("Lower Diff", "Controls");
    upDiff_ = cv::getTrackbarPos("Upper Diff", "Controls");
    fillR_ = cv::getTrackbarPos("Fill - R", "Controls");
    fillG_ = cv::getTrackbarPos("Fill - G", "Controls");
    fillB_ = cv::getTrackbarPos("Fill - B", "Controls");

    // If the user has clicked on the image, execute the floodFill algorithm
    if (isClicked_ && seedPoint_.x >= 0 && seedPoint_.y >= 0 && seedPoint_.x < src.cols && seedPoint_.y < src.rows) {
        isClicked_ = false; // Reset the trigger flag
        
        // BGR format
        cv::Scalar fillColor(fillB_, fillG_, fillR_);
        
        int flags = 4 | cv::FLOODFILL_FIXED_RANGE; 
        
        // Execute the native OpenCV floodFill algorithm
        cv::floodFill(out, seedPoint_, fillColor, nullptr, 
                      cv::Scalar(loDiff_, loDiff_, loDiff_), 
                      cv::Scalar(upDiff_, upDiff_, upDiff_), 
                      flags);

        commitToHistory(out); // Commit the new image state to the history stack for undo/redo
    }

    // Always render a friendly helper hint text on top of the image
    std::string hint = "[FloodFill] Adjust sliders and click on the image to fill color.";
    cv::putText(out, hint, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.50, cv::Scalar(0, 255, 255), 2);

    return out;
}