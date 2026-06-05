#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Operation{
public:
    virtual ~Operation() = default;
    virtual std::string name() const = 0;

    virtual void setupTrackbars(const std::string& controlsWindow) = 0;
    virtual cv::Mat apply(const cv::Mat& src) const = 0;
};