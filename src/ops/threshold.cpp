#include "ops/threshold.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// Function creates trackbars that control threshold operation
void ThresholdOp::setupTrackbars(const std::string& controlsWindow) {
    // Trackbar is used to choose thresholding method: 0 for Binary, 1 for Otsu, 2 for Adaptive
    cv::createTrackbar("Method 0B/1O/2A", controlsWindow, &method_, 2, appTrackbarCb);

    // Controls threshold value for normal binary thresholding
    cv::createTrackbar("Threshold", controlsWindow, &thresh_, 255, appTrackbarCb);

    // Controls block size used in adaptive thresholding
    cv::createTrackbar("Block (adapt)", controlsWindow, &block_, 99, appTrackbarCb);

    // This value is subtracted from local mean in adaptive thresholding
    cv::createTrackbar("C (adapt)", controlsWindow, &c_, 50, appTrackbarCb);
}

// Function applies selected threshold method to image
cv::Mat ThresholdOp::apply(const cv::Mat& src) const {
    cv::Mat gray;

    // Image is converted to grey first because thresholding works on greyscale images
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    cv::Mat dst;

    // method_ value decides which thresholding technique will be used
    switch (method_) {
        case 1:
            // Otsu automatically finds a good threshold value
            // Useful when do not want to set threshold manually
            cv::threshold(gray, dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            break;

        case 2: {
            // Adaptive thresholding uses different threshold values for different parts of image
            int bs = block_;

            // Block size must be at least 3
            if (bs < 3)
                bs = 3;

            // Block size must be an odd number
            if (bs % 2 == 0)
                bs += 1;

            // Apply adaptive mean thresholding
            cv::adaptiveThreshold(
                gray,
                dst,
                255,
                cv::ADAPTIVE_THRESH_MEAN_C,
                cv::THRESH_BINARY,
                bs,
                static_cast<double>(c_)
            );
            break;
        }

        case 0:
        default:
            // Normal binary thresholding
            // Pixels above thresh_ become white, and others become black
            cv::threshold(gray, dst, thresh_, 255, cv::THRESH_BINARY);
            break;
    }

    cv::Mat out;

    // Convert result back to BGR so it can be shown like other operations
    cv::cvtColor(dst, out, cv::COLOR_GRAY2BGR);

    return out;
}
