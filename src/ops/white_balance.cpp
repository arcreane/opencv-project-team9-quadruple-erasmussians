#include "ops/white_balance.hpp"
#include "app.hpp"


#include <opencv2/opencv_modules.hpp>
#ifdef HAVE_OPENCV_XPHOTO
#include <opencv2/xphoto/white_balance.hpp>
#endif

#include <algorithm>
#include <vector>

namespace {

// --- Gray world
// Compute a per-channel gain that pulls each channel's mean to the common gray
// level, then multiply (convertTo saturates back into [0,255]).
cv::Mat grayWorld(const cv::Mat& bgr) {
    const cv::Scalar mean = cv::mean(bgr);                  // per-channel averages
    const double gray = (mean[0] + mean[1] + mean[2]) / 3.0;
    std::vector<cv::Mat> ch;
    cv::split(bgr, ch);
    for (int i = 0; i < 3; ++i) {
        const double gain = mean[i] > 1e-6 ? gray / mean[i] : 1.0;
        ch[i].convertTo(ch[i], CV_8U, gain);                // multiply + saturate
    }
    cv::Mat out;
    cv::merge(ch, out);
    return out;
}

// --- Simple WB fallback (no opencv_contrib) 
// Per channel: drop the darkest and brightest `clipFraction` of pixels (robust
// to a handful of black/hot pixels), then linearly stretch the surviving
// [lo, hi] range to the full [0, 255]. Same idea as cv::xphoto::SimpleWB.
cv::Mat simpleWBfallback(const cv::Mat& bgr, double clipFraction) {
    std::vector<cv::Mat> ch;
    cv::split(bgr, ch);
    const double total = static_cast<double>(bgr.rows) * bgr.cols;
    const double loCut = total * clipFraction;
    const double hiCut = total * (1.0 - clipFraction);
    for (int c = 0; c < 3; ++c) {
        int histSize = 256;
        float range[] = {0.0f, 256.0f};
        const float* ranges = range;
        int channel0 = 0;
        cv::Mat hist;
        cv::calcHist(&ch[c], 1, &channel0, cv::Mat(), hist, 1, &histSize, &ranges);

        double cum = 0.0;
        int lo = 0, hi = 255;
        bool loSet = false;
        for (int i = 0; i < 256; ++i) {
            cum += hist.at<float>(i);
            if (!loSet && cum >= loCut) { lo = i; loSet = true; }
            if (cum >= hiCut)           { hi = i; break; }
        }
        if (hi <= lo) hi = std::min(255, lo + 1);           // guard a flat channel

        const double scale = 255.0 / (hi - lo);
        ch[c].convertTo(ch[c], CV_8U, scale, -lo * scale);  // (x - lo) * scale, clipped
    }
    cv::Mat out;
    cv::merge(ch, out);
    return out;
}

cv::Mat simpleWB(const cv::Mat& bgr) {
    const double kClip = 0.02;                              // ignore 2% at each end
#ifdef HAVE_OPENCV_XPHOTO
    cv::Ptr<cv::xphoto::SimpleWB> wb = cv::xphoto::createSimpleWB();
    wb->setP(static_cast<float>(kClip * 100.0));            // P is a percentage
    cv::Mat out;
    wb->balanceWhite(bgr, out);
    return out;
#else
    return simpleWBfallback(bgr, kClip);
#endif
}

}  // namespace

void AutoWhiteBalanceOp::setupTrackbars(const std::string& win) {
    cv::createTrackbar("Method 0Gray 1Simple", win, &method_,   1,   appTrackbarCb);
    cv::createTrackbar("Strength %",            win, &strength_, 100, appTrackbarCb);
}

cv::Mat AutoWhiteBalanceOp::apply(const cv::Mat& src) const {
    if (src.empty()) return src;

    const cv::Mat balanced = (method_ == 0) ? grayWorld(src) : simpleWB(src);

    const double a = strength_ / 100.0;                     // 0 = original, 1 = full
    if (a >= 1.0) return balanced;
    cv::Mat out;
    cv::addWeighted(src, 1.0 - a, balanced, a, 0.0, out);
    return out;                                             // BGR in, BGR out
}