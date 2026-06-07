
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "operation.hpp"
#include "app.hpp"


#include "ops/identity.hpp"
#include "ops/brightness.hpp"
#include "ops/geometry.hpp"

// New implemented features' headers will be added here
// #include "ops/threshold.hpp"

namespace {

    const char* kMainWindow = "MyEditor";
    const char* kControlsWindow = "Controls";
    constexpr int kPreviewMaxDim = 720;   // longest 

    cv::Mat g_original;          
    cv::Mat g_preview;           
    double  g_previewScale = 1.0;

    std::vector<std::unique_ptr<Operation>> g_ops;
    int g_mode = 0;   //index of active operation 

    void render() {
        if (g_preview.empty()) return;
        cv::Mat out = g_ops[g_mode]->apply(g_preview);
        cv::imshow(kMainWindow, out);
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
        render();
    }

    void saveResult() {
        cv::Mat full = g_ops[g_mode]->apply(g_original);
        const std::string outPath = "output.png";
        if (cv::imwrite(outPath, full))
            std::cout << "Saved full-resolution result to " << outPath << "\n";
        else
            std::cerr << "Failed to write " << outPath << "\n";
    }

}  

void appTrackbarCb(int, void*) { render(); }
void appRequestRender() { render(); }
const char* appMainWindow() { return kMainWindow; }

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "samples/sample.jpg";

    g_original = cv::imread(path, cv::IMREAD_COLOR);
    if (g_original.empty()) {
        std::cerr << "Could not load image: " << path << "\n"
            << "Usage: myeditor <image-path>\n";
        return 1;
    }

    
    g_ops.push_back(std::make_unique<IdentityOp>());
    g_ops.push_back(std::make_unique<BrightnessOp>());
    g_ops.push_back(std::make_unique<GeometryOp>()); 

    g_previewScale = std::min(
        1.0, static_cast<double>(kPreviewMaxDim) /
        std::max(g_original.cols, g_original.rows));
    cv::resize(g_original, g_preview, cv::Size(), g_previewScale, g_previewScale,
        cv::INTER_AREA);

    cv::namedWindow(kMainWindow, cv::WINDOW_AUTOSIZE);
    rebuildControls();

    std::cout << "MyEditor ready.\n"
        << "  n / p : next / previous operation\n"
        << "  s     : save full-resolution result to output.png\n"
        << "  q/ESC : quit\n";

    for (;;) {
        const int key = cv::waitKey(20) & 0xFF;
        if (key == 'q' || key == 27) break;
        if (key == 'n') {
            g_mode = (g_mode + 1) % static_cast<int>(g_ops.size());
            rebuildControls();
        }
        else if (key == 'p') {
            g_mode = (g_mode - 1 + static_cast<int>(g_ops.size())) %
                static_cast<int>(g_ops.size());
            rebuildControls();
        }
        else if (key == 's') {
            saveResult();
        }
    }
    return 0;
}