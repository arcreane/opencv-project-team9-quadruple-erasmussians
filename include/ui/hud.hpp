#pragma once
#include <opencv2/core.hpp>
#include <string>

// Display only.the shell calls them inside render ()
namespace ui {

void drawStatusBar(cv::Mat& frame, const std::string& opName, int index, int total);


void drawToast(cv::Mat& frame, const std::string& msg);

}  