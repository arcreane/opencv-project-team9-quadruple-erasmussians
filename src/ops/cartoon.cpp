#include "ops/cartoon.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>  // cvtColor, medianBlur, adaptiveThreshold, bilateralFilter, LUT
#include <opencv2/highgui.hpp>  // createTrackbar
#include <algorithm>            // std::max, std::min

void CartoonOp::setupTrackbars(const std::string& controlsWindow) {
    // Shared callback so any change re-renders. Ranges chosen to stay responsive:
    // Smooth iterations are capped because each bilateral pass is costly.
    cv::createTrackbar("Edge thickness", controlsWindow, &edgeBlock_, 50, appTrackbarCb);
    cv::createTrackbar("Edge detail",    controlsWindow, &edgeC_,     30, appTrackbarCb);
    cv::createTrackbar("Smoothing",      controlsWindow, &smooth_,    8,  appTrackbarCb);
    cv::createTrackbar("Quantize lvls",  controlsWindow, &quant_,     32, appTrackbarCb);
}

cv::Mat CartoonOp::apply(const cv::Mat& src) const {
    // --- 1) Edge mask: bold black outlines on a white field ------------------
    // adaptiveThreshold needs an odd block size >= 3; the slider can give any
    // value, so sanitise. medianBlur first kills speckle so we get clean lines.
    int bs = edgeBlock_;
    if (bs < 3)      bs = 3;
    if (bs % 2 == 0) bs += 1;

    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 7);

    cv::Mat edges; // 255 in flat areas, 0 along edges -> the dark cartoon outline
    cv::adaptiveThreshold(gray, edges, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY, bs, static_cast<double>(edgeC_));

    // --- 2) Colour smoothing: flatten into cartoon-like patches --------------
    // Bilateral smooths colour while keeping borders crisp. It cannot run
    // in-place, so we ping-pong between two buffers across the iterations.
    cv::Mat color = src.clone();
    int iters = std::max(0, smooth_);
    for (int i = 0; i < iters; ++i) {
        cv::Mat tmp;
        cv::bilateralFilter(color, tmp, 9, 75, 75);
        color = tmp;
    }

    // Optional posterization: snap each channel to N levels for a flatter look.
    // Done with a 256-entry LUT (applied per channel) so it stays cheap.
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

    // --- 3) Combine: keep colour where the mask is white, draw the outlines ---
    cv::Mat edges3, out;
    cv::cvtColor(edges, edges3, cv::COLOR_GRAY2BGR); // mask must match channels
    cv::bitwise_and(color, edges3, out);             // 0 in mask -> black outline
    return out;
}
