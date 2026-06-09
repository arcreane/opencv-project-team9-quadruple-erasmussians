#include "ops/histeq.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <vector>

void HistEqOp::setupTrackbars(const std::string& controlsWindow) {
    cv::createTrackbar("Method 0G/1C", controlsWindow, &method_, 1,  appTrackbarCb);
    cv::createTrackbar("CLAHE clip",   controlsWindow, &clip_,   40, appTrackbarCb);
    cv::createTrackbar("CLAHE tiles",  controlsWindow, &tiles_,  16, appTrackbarCb);
}

cv::Mat HistEqOp::apply(const cv::Mat& src) const {
    cv::Mat ycrcb;
    cv::cvtColor(src, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> ch;
    cv::split(ycrcb, ch);

    if (method_ == 1) {
        double clipLimit = std::max(1, clip_) / 10.0;
        int    tiles     = std::max(1, tiles_);
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, cv::Size(tiles, tiles));
        clahe->apply(ch[0], ch[0]);
    } else {
        cv::equalizeHist(ch[0], ch[0]);
    }

    cv::merge(ch, ycrcb);

    cv::Mat out;
    cv::cvtColor(ycrcb, out, cv::COLOR_YCrCb2BGR);
    return out;
}
