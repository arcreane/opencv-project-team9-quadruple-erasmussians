#include "ops/threshold.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

void ThresholdOp::setupTrackbars(const std::string& controlsWindow) {
    cv::createTrackbar("Method 0B/1O/2A", controlsWindow, &method_, 2,  appTrackbarCb);
    cv::createTrackbar("Threshold",       controlsWindow, &thresh_, 255, appTrackbarCb);
    cv::createTrackbar("Block (adapt)",   controlsWindow, &block_,  99,  appTrackbarCb);
    cv::createTrackbar("C (adapt)",       controlsWindow, &c_,      50,  appTrackbarCb);
}

cv::Mat ThresholdOp::apply(const cv::Mat& src) const {
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    cv::Mat dst;
    switch (method_) {
        case 1:
            cv::threshold(gray, dst, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            break;

        case 2: {
            int bs = block_;
            if (bs < 3)      bs = 3;
            if (bs % 2 == 0) bs += 1;
            cv::adaptiveThreshold(gray, dst, 255,
                                  cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY,
                                  bs, static_cast<double>(c_));
            break;
        }

        case 0:
        default:
            cv::threshold(gray, dst, thresh_, 255, cv::THRESH_BINARY);
            break;
    }

    cv::Mat out;
    cv::cvtColor(dst, out, cv::COLOR_GRAY2BGR);
    return out;
}