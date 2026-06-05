#pragma once 
#include "operation.hpp"

class IdentityOp: public Operation{
public:
    std::string name() const override { return "Original"; }
    void setupTrackbars(const std::string& /*win*/) override {}
    cv::Mat apply(const cv::Mat& src) const override { return src.clone(); }

} ;