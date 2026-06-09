#include "ui/hud.hpp"

#include <opencv2/imgproc.hpp>


namespace {

constexpr int    kBarH = 28;                         // status bar height px
constexpr int    kFont = cv::FONT_HERSHEY_SIMPLEX;
const cv::Scalar kWhite (255, 255, 255);
const cv::Scalar kGrey  (200, 200, 200);
const cv::Scalar kGreen (180, 255, 180);             

// Blend a filled dark rectangle into the frame so overlaid text stays legible
void darkenPanel(cv::Mat& frame, cv::Rect rect, double alpha) {
    rect &= cv::Rect(0, 0, frame.cols, frame.rows);
    if (rect.empty()) return;
    cv::Mat roi = frame(rect);
    cv::Mat shade(roi.size(), roi.type(), cv::Scalar::all(0));
    cv::addWeighted(shade, alpha, roi, 1.0 - alpha, 0.0, roi);
}

int textWidth(const std::string& s, double fs, int th) {
    int baseline = 0;
    return cv::getTextSize(s, kFont, fs, th, &baseline).width;
}

}  // namespace

namespace ui {

void drawStatusBar(cv::Mat& frame, const std::string& opName, int index, int total) {
    if (frame.empty()) return;
    const int W = frame.cols, H = frame.rows;
    const int pad = 10;
    const double fs = 0.45;
    const int th = 1;

    darkenPanel(frame, cv::Rect(0, H - kBarH, W, kBarH), 0.55);
    const int baseline = H - 9;                      // text baseline inside the bar

    // Left: position + operation name (the "where am I" half).
    const std::string left = std::to_string(index + 1) + "/" +
                             std::to_string(total) + "  " + opName;
    cv::putText(frame, left, {pad, baseline}, kFont, fs, kWhite, th, cv::LINE_AA);

    // Right: the key legend, right-aligned. Shrink to a compact form, then drop
    // it entirely, if the preview is too narrow to show it without overlapping
    // the name -- the name always wins.
    const int leftW = textWidth(left, fs, th);
    const std::string legendFull  = "[a] Apply  [u/r] Undo/Redo  [n/p] Switch Mode   [o] Open   [s] Save   [q] Quit";
    const std::string legendShort = "a  u/r  n/p  o  s  q";
    auto fits = [&](const std::string& s) {
        return leftW + pad * 3 + textWidth(s, fs, th) <= W - pad;
    };
    const std::string legend = fits(legendFull)  ? legendFull
                             : fits(legendShort) ? legendShort
                                                 : std::string();
    if (!legend.empty()) {
        const int w = textWidth(legend, fs, th);
        cv::putText(frame, legend, {W - pad - w, baseline}, kFont, fs, kGrey, th,
                    cv::LINE_AA);
    }
}

void drawToast(cv::Mat& frame, const std::string& msg) {
    if (frame.empty() || msg.empty()) return;
    const double fs = 0.6;
    const int th = 2;
    int baseline = 0;
    const cv::Size ts = cv::getTextSize(msg, kFont, fs, th, &baseline);

    const int padX = 16, padY = 9;
    const int boxW = ts.width + 2 * padX;
    const int boxH = ts.height + 2 * padY;
    const int x = (frame.cols - boxW) / 2;           // centered horizontally
    int y = frame.rows - kBarH - boxH - 8;           // sit just above the status bar
    if (y < 8) y = 8;                                // clamp on very short previews

    const cv::Rect box(x, y, boxW, boxH);
    darkenPanel(frame, box, 0.65);
    cv::rectangle(frame, box & cv::Rect(0, 0, frame.cols, frame.rows),
                  cv::Scalar(140, 200, 140), 1, cv::LINE_AA);
    cv::putText(frame, msg, {x + padX, y + padY + ts.height}, kFont, fs, kGreen, th,
                cv::LINE_AA);
}

}  // namespace ui