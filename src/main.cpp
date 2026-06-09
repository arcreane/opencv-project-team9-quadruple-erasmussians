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
#include "ops/floodfill.hpp"
#include "ops/white_balance.hpp"

const char* kMainWindow = "MyEditor";
const char* kControlsWindow = "Controls";
constexpr int kPreviewMaxDim = 720;   // longest 

cv::Mat g_original;          
cv::Mat g_preview;           
double  g_previewScale = 1.0;

std::vector<std::unique_ptr<Operation>> g_ops;
int g_mode = 0;   // index of active operation 
int g_pickerPos   = 0;    // backing value for the "Operation" trackbar
int g_pendingMode = -1; // set by onPickOp, consumed (and cleared) in the loop      

std::vector<cv::Mat> g_undoStack;
std::vector<cv::Mat> g_redoStack;
constexpr size_t MAX_HISTORY = 30; // limit history steps: 30
// Transient confirmation toast (e.g. after Save / Open). render() draws it while
// active; the loop erases it once it expires.
std::string g_toast;
std::chrono::steady_clock::time_point g_toastUntil{};

bool toastActive() {
    return !g_toast.empty() && std::chrono::steady_clock::now() < g_toastUntil;
}

bool imagesEqual(const cv::Mat& a, const cv::Mat& b) {
    if (a.size() != b.size() || a.type() != b.type()) return false;
    cv::Mat diff;
    cv::absdiff(a, b, diff);
    return cv::countNonZero(diff.reshape(1)) == 0;
}

void commitToHistory(const cv::Mat& currentPreviewImage) {
    // Keep the working image 3-channel BGR so baked effects can be chained
    cv::Mat state = currentPreviewImage;
    if (state.channels() == 1) cv::cvtColor(state, state, cv::COLOR_GRAY2BGR);

    // Current image state to the Undo stack
    g_undoStack.push_back(state.clone());
    if (g_undoStack.size() > MAX_HISTORY) {
        g_undoStack.erase(g_undoStack.begin());
    }
    // Clear the Redo stack whenever a new action is committed
    g_redoStack.clear();
    // Update the preview
    g_preview = state.clone();
}

void performUndo() {
    if (g_undoStack.size() <= 1) { // at least 2 states to Undo (current + previous)
        g_toast = "Cannot Undo further";
        g_toastUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        return;
    }
    // Push the current state to the Redo stack
    g_redoStack.push_back(g_undoStack.back());
    g_undoStack.pop_back();
    
    // Switch the preview canvas back to the previous state
    g_preview = g_undoStack.back().clone();
}

void performRedo() {
    if (g_redoStack.empty()) return;
    
    g_undoStack.push_back(g_redoStack.back());
    g_preview = g_redoStack.back().clone();
    g_redoStack.pop_back();
}

void rebuildPreview() {
    g_previewScale = std::min(
        1.0, static_cast<double>(kPreviewMaxDim) /
                 std::max(g_original.cols, g_original.rows));
    cv::resize(g_original, g_preview, cv::Size(), g_previewScale, g_previewScale,
               cv::INTER_AREA);

    // Reset history stacks whenever a new image is loaded
    g_undoStack.clear();
    g_redoStack.clear();
    g_undoStack.push_back(g_preview.clone());
}

void render() {
    if (g_preview.empty()) return;
    
    cv::Mat out = g_ops[g_mode]->apply(g_preview);
    
    // The HUD is display-only. drawn on the shown copy here, never on the image
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
                   std::string("MyEditor  -  ") + std::to_string(g_mode + 1) +
                   "/" + std::to_string(g_ops.size()) + "  " +
                   g_ops[g_mode]->name());
    render();
}

void saveResult() {
    // Apply the current operation to the preview image and save the result
    if (ui::saveImage(g_ops[g_mode]->apply(g_preview))) setToast("Saved");
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
    g_ops.push_back(std::make_unique<FloodFillOp>());
    
    rebuildPreview(); // Generate the initial preview image (resets history stacks)

    cv::namedWindow(kMainWindow, cv::WINDOW_AUTOSIZE);
    rebuildControls();

    cv::createTrackbar("Operation", kMainWindow, &g_pickerPos,
                       static_cast<int>(g_ops.size()) - 1, onPickOp);

    std::cout << "MyEditor ready.\n"
              << "  o     : open an image\n"
              << "  n / p : next / previous operation\n"
              << "  a     : apply current effect (bake it in)\n"
              << "  u / r : UNDO / REDO last action\n"
              << "  s     : save (choose location)\n"
              << "  q/ESC : quit\n";
    
    bool toastWasVisible = false;

    for (;;) {
        if (g_pendingMode >= 0) {
            const int target = g_pendingMode;
            g_pendingMode = -1;
            if (target != g_mode) { g_mode = target; rebuildControls(); }
        }
        // erase the toast once it expires
        const bool toastNow = toastActive();
        if (toastWasVisible && !toastNow) render();
        toastWasVisible = toastNow;

        const int key = cv::waitKey(20) & 0xFF;
        if (key == 'q' || key == 27) break;
        
        // Bake the current effect into the working image and record it so it
        // can be undone. Flood Fill records its own states on click, so skip it.
        if (key == 'a') {
            if (!g_ops[g_mode]->managesOwnHistory()) {
                cv::Mat baked = g_ops[g_mode]->apply(g_preview);
                if (imagesEqual(baked, g_preview)) {
                    setToast("Nothing to apply");
                } else {
                    commitToHistory(baked);   // updates g_preview + pushes undo state
                    g_mode = 0;               // show the baked result as-is (Original)
                    rebuildControls();
                    cv::setTrackbarPos("Operation", kMainWindow, g_mode);
                    setToast("Applied");
                }
            }
        }
        // Undo/Redo with 'u' and 'r' keys
        else if (key == 'u') {
            performUndo();
            render();
        } else if (key == 'r') {
            performRedo();
            render();
        } else if (key == 'n') {
            g_mode = (g_mode + 1) % static_cast<int>(g_ops.size());
            rebuildControls();
            cv::setTrackbarPos("Operation", kMainWindow, g_mode);  // keep slider in sync
        } else if (key == 'p') {
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