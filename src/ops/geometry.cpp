#include "ops/geometry.hpp"
#include "app.hpp"

// points required by the current mode
static int neededPoints(int mode) { return mode == 0 ? 3 : 4; }

void GeometryOp::setupTrackbars(const std::string& win) {
    // Mode selector sits in the Controls window like every other op.
    cv::createTrackbar("Mode 0Aff 1Persp", win, &mode_, 1, appTrackbarCb);

    
    cv::setMouseCallback(appMainWindow(), &GeometryOp::onMouse, this);
}

void GeometryOp::onMouse(int event, int x, int y, int /*flags*/, void* userdata) {
    auto* self = static_cast<GeometryOp*>(userdata);
    if (self->shownSize_.width <= 0) return;        // nothing rendered yet: scale unknown

    if (event == cv::EVENT_LBUTTONDOWN) {

        if (static_cast<int>(self->ptsNorm_.size()) < neededPoints(self->mode_)) {
            self->ptsNorm_.emplace_back(
                x / static_cast<float>(self->shownSize_.width),
                y / static_cast<float>(self->shownSize_.height));
            appRequestRender();                      // redraw so the marker appears now
        }
    } else if (event == cv::EVENT_RBUTTONDOWN) {
        self->ptsNorm_.clear();                      // reset all points
        appRequestRender();
    }
}

cv::Mat GeometryOp::apply(const cv::Mat& src) const {
    const int W = src.cols, H = src.rows;
    shownSize_ = src.size();
    const int need = neededPoints(mode_);                         // record scale for the callback

    std::vector<cv::Point2f> pts;
    pts.reserve(ptsNorm_.size());
    for (const auto& n : ptsNorm_) pts.emplace_back(n.x * W, n.y * H);

    if (static_cast<int>(pts.size()) < need) {
        cv::Mat out = src.clone();
        for (size_t i = 0; i < pts.size(); ++i) {
            cv::circle(out, pts[i], 5, cv::Scalar(0, 255, 0), -1);
            if (i > 0) cv::line(out, pts[i - 1], pts[i], cv::Scalar(0, 255, 0), 1);
            cv::putText(out, std::to_string(i + 1), pts[i] + cv::Point2f(8, -8),
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
        }
        const std::string hint = (mode_ == 0)
            ? "Affine: click 3 pts (TL,TR,BL). Right-click=reset."
            : "Perspective: click 4 pts (TL,TR,BR,BL). Right-click=reset.";
        cv::putText(out, hint, {10, 24}, cv::FONT_HERSHEY_SIMPLEX, 0.55,
                    cv::Scalar(0, 0, 255), 2);
        return out;                                  // already BGR
    }

    cv::Mat out;
    if (mode_ == 0) {
        cv::Point2f srcTri[3] = {pts[0], pts[1], pts[2]};
        cv::Point2f dstTri[3] = {{0, 0}, {float(W), 0}, {0, float(H)}};
        cv::Mat M = cv::getAffineTransform(srcTri, dstTri);
        cv::warpAffine(src, out, M, src.size());
    } else {
        cv::Point2f srcQuad[4] = {pts[0], pts[1], pts[2], pts[3]};
        cv::Point2f dstQuad[4] = {{0, 0}, {float(W), 0}, {float(W), float(H)}, {0, float(H)}};
        cv::Mat M = cv::getPerspectiveTransform(srcQuad, dstQuad);
        cv::warpPerspective(src, out, M, src.size());
    }
    return out;                                      
}