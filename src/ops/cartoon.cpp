#include "ops/cartoon.hpp"
#include "app.hpp"
#include <opencv2/imgproc.hpp>  
#include <opencv2/highgui.hpp>  
#include <algorithm>            

// Function creates trackbars for cartoon effect settings
void CartoonOp::setupTrackbars(const std::string& controlsWindow) {
    // Controls block size used for edge detection
    cv::createTrackbar("Edge thickness", controlsWindow, &edgeBlock_, 50, appTrackbarCb);

    // Controls C value used in adaptive thresholding for edges
    cv::createTrackbar("Edge detail", controlsWindow, &edgeC_, 30, appTrackbarCb);

    // Controls how many times smoothing is applied
    cv::createTrackbar("Smoothing", controlsWindow, &smooth_, 8, appTrackbarCb);

    // Controls number of color levels for quantization
    cv::createTrackbar("Quantize lvls", controlsWindow, &quant_, 32, appTrackbarCb);
}

// Function applies cartoon effect to input image
cv::Mat CartoonOp::apply(const cv::Mat& src) const {
    int bs = edgeBlock_;

    // Block size for adaptive threshold must be at least 3
    if (bs < 3)
        bs = 3;

    // Block size must be odd
    if (bs % 2 == 0)
        bs += 1;

    cv::Mat gray;

    // Convert image to grayscale for edge detection
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    // Apply median blur to reduce noise before detecting edges
    cv::medianBlur(gray, gray, 7);

    cv::Mat edges;

    // Detect edges using adaptive thresholdin helping create black cartoon-like outlines
    cv::adaptiveThreshold(
        gray,
        edges,
        255,
        cv::ADAPTIVE_THRESH_MEAN_C,
        cv::THRESH_BINARY,
        bs,
        static_cast<double>(edgeC_)
    );

    // Make copy of original image to process colors
    cv::Mat color = src.clone();

    // Apply bilateral filtering several times to smooth image wile still keeping edges visible
    int iters = std::max(0, smooth_);
    for (int i = 0; i < iters; ++i) {
        cv::Mat tmp;
        cv::bilateralFilter(color, tmp, 9, 75, 75);
        color = tmp;
    }

    // Reduce number of color levels to make image look flatter, which gives more cartoon-like style
    if (quant_ >= 2) {
        const int levels = quant_;
        const float step = 256.0f / levels;

        // Create lookup table for color quantization
        cv::Mat lut(1, 256, CV_8U);

        for (int i = 0; i < 256; ++i) {
            int center = static_cast<int>((static_cast<int>(i / step) + 0.5f) * step);
            lut.at<uchar>(i) = static_cast<uchar>(std::min(255, center));
        }

        // Apply quantization to image
        cv::LUT(color, lut, color);
    }

    cv::Mat edges3, out;

    // Convert edge image to BGR so it matches color image format
    cv::cvtColor(edges, edges3, cv::COLOR_GRAY2BGR);

    // Combine smooth color image with detected edges, giving final cartoon effect
    cv::bitwise_and(color, edges3, out);

    return out;
}
