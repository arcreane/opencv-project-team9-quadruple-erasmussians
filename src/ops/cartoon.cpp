#include "ops/cartoon.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>  
#include <opencv2/highgui.hpp>  
#include <algorithm>            

void CartoonOp::setupTrackbars(const std::string& controlsWindow) {
    cv::createTrackbar("Edge thickness", controlsWindow, &edgeBlock_, 50, appTrackbarCb);
    cv::createTrackbar("Edge detail",    controlsWindow, &edgeC_,     30, appTrackbarCb);
    cv::createTrackbar("Smoothing",      controlsWindow, &smooth_,    8,  appTrackbarCb);
    cv::createTrackbar("Quantize lvls",  controlsWindow, &quant_,     32, appTrackbarCb);
}

cv::Mat CartoonOp::apply(const cv::Mat& src) const {
    int bs = edgeBlock_;
    if (bs < 3)      bs = 3;
    if (bs % 2 == 0) bs += 1;

    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 7);

    cv::Mat edges; 
    cv::adaptiveThreshold(gray, edges, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY, bs, static_cast<double>(edgeC_));

    cv::Mat color = src.clone();
    int iters = std::max(0, smooth_);
    for (int i = 0; i < iters; ++i) {
        cv::Mat tmp;
        cv::bilateralFilter(color, tmp, 9, 75, 75);
        color = tmp;
    }

    if (quant_ >= 2) {
        const int   levels = quant_;
        const float step   = 256.0f / levels;
        cv::Mat lut(1, 256, CV_8U);
        for (int i = 0; i < 256; ++i) {
            int center = static_cast<int>((static_cast<int>(i / step) + 0.5f) * step);
            lut.at<uchar>(i) = static_cast<uchar>(std::min(255, center));
        }
        cv::LUT(color, lut, color);
    }

    cv::Mat edges3, out;
    cv::cvtColor(edges, edges3, cv::COLOR_GRAY2BGR); 
    cv::bitwise_and(color, edges3, out);             
    return out;
}
