#pragma once
#include "operation.hpp"

// Class represents histogram equalization operation, inherits from Operation base class
class HistEqOp : public Operation {
public:
    // Returns name of operation, can be used to show operation name in program
    std::string name() const override { return "Histogram Equalization"; }

    // Function creates trackbars for controlling settings
    void setupTrackbars(const std::string& controlsWindow) override;

    // Function applies histogram equalization to input image
    cv::Mat apply(const cv::Mat& src) const override;

private:
    // Chooses the histogram equalization method: 0 for normal histogram equalization, 1 for CLAHE
    mutable int method_ = 1;

    // Controls CLAHE clip limit value
    mutable int clip_ = 20;

    // Controls number of tiles used for CLAHE
    mutable int tiles_ = 8;
};
