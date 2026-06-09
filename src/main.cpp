
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "operation.hpp"
#include "app.hpp"
#include "ui/file_dialog.hpp"
#include "ui/hud.hpp"

#include "ops/identity.hpp"
#include "ops/brightness.hpp"
#include "ops/geometry.hpp"
#include "ops/morphology.hpp"
#include "ops/canny.hpp"
#include "ops/unsharpmask.hpp"
#include "ops/pencilsketch.hpp"
#include "ops/stitching.hpp"


#include "ops/threshold.hpp"
#include "ops/histeq.hpp"
#include "ops/cartoon.hpp"

#include "ops/white_balance.hpp"

namespace {

    const char* kMainWindow = "MyEditor";
    const char* kControlsWindow = "Controls";
    constexpr int kPreviewMaxDim = 720;   // longest 

    cv::Mat g_original;          
    cv::Mat g_preview;           
    double  g_previewScale = 1.0;

    std::vector<std::unique_ptr<Operation>> g_ops;
    int g_mode = 0;   //index of active operation 
    int g_pickerPos   = 0;    // backing value for the "Operation" trackbar
    int g_pendingMode = -1; // set by onPickOp, consumed (and cleared) in the loop      

    // Transient confirmation toast (e.g. after Save / Open). render() draws it while
    // active; the loop erases it once it expires.
    std::string g_toast;
    std::chrono::steady_clock::time_point g_toastUntil{};

    bool toastActive() {
        return !g_toast.empty() && std::chrono::steady_clock::now() < g_toastUntil;
    }

    void rebuildPreview() {
    g_previewScale = std::min(
        1.0, static_cast<double>(kPreviewMaxDim) /
                 std::max(g_original.cols, g_original.rows));
    cv::resize(g_original, g_preview, cv::Size(), g_previewScale, g_previewScale,
               cv::INTER_AREA);
    }

    void render() {
        if (g_preview.empty()) return;
        cv::Mat out = g_ops[g_mode]->apply(g_preview);
        // The HUD is display-only. drawn on the shown copy here, never on the image
        // saveResult() writes  so the full-resolution save stays clean.
        ui::drawStatusBar(out, g_ops[g_mode]->name(), g_mode,
                        static_cast<int>(g_ops.size()));

        if (toastActive()) ui::drawToast(out, g_toast);
        cv::imshow(kMainWindow, out);
    }

    void onPickOp(int pos, void*) { g_pendingMode = pos; }

    // Show a brief confirmation message and refresh so it appears immediately. The
    // loop clears it once g_toastUntil passes.
    void setToast(const std::string& msg) {
        g_toast = msg;
        g_toastUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(1500);
        render();
    }


    void rebuildControls() {
        static bool created = false;
        if (created) cv::destroyWindow(kControlsWindow);
        cv::namedWindow(kControlsWindow, cv::WINDOW_NORMAL);
        cv::resizeWindow(kControlsWindow, 440, 260);
        created = true;
        g_ops[g_mode]->setupTrackbars(kControlsWindow);
        cv::setWindowTitle(kMainWindow,
                       std::string("MyEditor  -  ") + g_ops[g_mode]->name());
                       std::string("MyEditor  -  ") + std::to_string(g_mode + 1) +
                       "/" + std::to_string(g_ops.size()) + "  " +
                       g_ops[g_mode]->name();
        render();
}

    void saveResult() {
        // pick where to save via a native "Save As" dialog. Confirm on screen,
        if (ui::saveImage(g_ops[g_mode]->apply(g_original))) setToast("Saved");
    }

}  

void appTrackbarCb(int, void*) { render(); }
void appRequestRender() { render(); }
const char* appMainWindow() { return kMainWindow; }

int main(int argc, char** argv) {

    if (argc > 1) g_original = ui::loadImage(argv[1]);
    if (g_original.empty()) ui::openImage(g_original);
    if (g_original.empty()) {
        ui::errorBox("No image was opened, so MyEditor will close.");
        return 0;
    }

    g_ops.push_back(std::make_unique<IdentityOp>());
    g_ops.push_back(std::make_unique<BrightnessOp>());
    g_ops.push_back(std::make_unique<GeometryOp>());
    g_ops.push_back(std::make_unique<AutoWhiteBalanceOp>());
    g_ops.push_back(std::make_unique<MorphologyOp>());  
    g_ops.push_back(std::make_unique<CannyOp>());     
    g_ops.push_back(std::make_unique<UnsharpMaskOp>());
    g_ops.push_back(std::make_unique<PencilSketchOp>());  
    g_ops.push_back(std::make_unique<ThresholdOp>());
    g_ops.push_back(std::make_unique<HistEqOp>());
    g_ops.push_back(std::make_unique<CartoonOp>());
    g_ops.push_back(std::make_unique<StitchingOp>());

    rebuildPreview();

    cv::namedWindow(kMainWindow, cv::WINDOW_AUTOSIZE);
    rebuildControls();

    cv::createTrackbar("Operation", kMainWindow, &g_pickerPos,
                       static_cast<int>(g_ops.size()) - 1, onPickOp);

    std::cout << "MyEditor ready.\n"
              << "  o     : open an image\n"
              << "  n / p : next / previous operation\n"
              << "  s     : save full-resolution result to output.png\n"
              << "  s     : save (choose location)\n"
              << "  q/ESC : quit\n";
    
    bool toastWasVisible = false;

    for (;;) {
        if (g_pendingMode >= 0) {
            const int target = g_pendingMode;
            g_pendingMode = -1;
            if (target != g_mode) { g_mode = target; rebuildControls(); }
        }
        //erase the toast once it expires
        const bool toastNow = toastActive();
        if (toastWasVisible && !toastNow) render();
        toastWasVisible = toastNow;

        const int key = cv::waitKey(20) & 0xFF;
        if (key == 'q' || key == 27) break;
        if (key == 'n') {
            g_mode = (g_mode + 1) % static_cast<int>(g_ops.size());
            rebuildControls();
            cv::setTrackbarPos("Operation", kMainWindow, g_mode);  // keep slider in sync
        }
        else if (key == 'p') {
            g_mode = (g_mode - 1 + static_cast<int>(g_ops.size())) %
                static_cast<int>(g_ops.size());
            rebuildControls();
            cv::setTrackbarPos("Operation", kMainWindow, g_mode);  // keep slider in sync
        } else if (key == 'o') {
            if (ui::openImage(g_original)) { rebuildPreview(); setToast("Opened"); }
        } else if (key == 's') {
            saveResult();
        }
    }
    return 0;
}