#pragma once
#include <opencv2/core.hpp>
#include <string>

namespace ui {
bool openImage(cv::Mat& out);
bool saveImage(const cv::Mat& img);

cv::Mat loadImage(const std::string& path);

void errorBox(const std::string& message);

}  // namespace ui