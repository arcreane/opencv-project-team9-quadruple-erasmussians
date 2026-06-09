#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Operation{
public:
    virtual ~Operation() = default;
    virtual std::string name() const = 0;

    virtual void setupTrackbars(const std::string& controlsWindow) = 0;
    virtual cv::Mat apply(const cv::Mat& src) const = 0;

    // True for operations that push their own undo states (e.g. click-driven
    // Flood Fill). The "apply / bake" key skips these so they aren't committed
    // twice or baked together with their on-image hint text.
    virtual bool managesOwnHistory() const { return false; }
};