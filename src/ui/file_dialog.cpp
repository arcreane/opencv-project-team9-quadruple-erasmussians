#include "ui/file_dialog.hpp"

#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::vector<uchar> readFileBytes(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return {};
    return std::vector<uchar>((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
}

bool writeFileBytes(const std::filesystem::path& p, const std::vector<uchar>& bytes) {
    std::ofstream f(p, std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    return f.good();
}

cv::Mat decodeImageFile(const std::filesystem::path& p) {
    const std::vector<uchar> bytes = readFileBytes(p);
    if (bytes.empty()) return {};
    return cv::imdecode(bytes, cv::IMREAD_COLOR);
}

}  //namespace

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")  
#pragma comment(lib, "user32.lib")    
namespace {

std::wstring toWide(const std::string& s) {
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n > 0 ? n - 1 : 0, L'\0');
    if (n > 0) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
    return w;
}

}  //namespace

namespace ui {

bool openImage(cv::Mat& out) {
    wchar_t file[4096] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Images\0*.jpg;*.jpeg;*.png;*.bmp;*.tif;*.tiff;*.webp\0All files\0*.*\0";
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = ARRAYSIZE(file);
    ofn.lpstrTitle  = L"MyEditor - Open image";
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn)) return false;  // cancelled / closed
    cv::Mat img = decodeImageFile(std::filesystem::path(file));
    if (img.empty()) { errorBox("Could not open that image file."); return false; }
    out = img;
    return true;
}

bool saveImage(const cv::Mat& img) {
    if (img.empty()) return false;
    wchar_t file[4096] = L"edited.png";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"PNG\0*.png\0JPEG\0*.jpg;*.jpeg\0BMP\0*.bmp\0TIFF\0*.tif;*.tiff\0";
    ofn.lpstrFile   = file;
    ofn.nMaxFile    = ARRAYSIZE(file);
    ofn.lpstrTitle  = L"MyEditor - Save image as";
    ofn.lpstrDefExt = L"png";
    ofn.Flags       = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameW(&ofn)) return false;  // cancelled or closed
    const std::filesystem::path path(file);
    std::string ext = path.extension().string();  // ".png
    if (ext.empty()) ext = ".png";
    std::vector<uchar> bytes;
    if (!cv::imencode(ext, img, bytes)) { errorBox("Could not encode the image."); return false; }
    if (!writeFileBytes(path, bytes))   { errorBox("Could not write the file.");   return false; }
    return true;
}

cv::Mat loadImage(const std::string& path) {
    return decodeImageFile(std::filesystem::path(toWide(path)));
}

void errorBox(const std::string& message) {
    MessageBoxW(nullptr, toWide(message).c_str(), L"MyEditor", MB_OK | MB_ICONERROR);
}

}  //namespace ui

#else  # non-windows fallback
#include <iostream>

namespace ui {

bool openImage(cv::Mat&) {
    std::cerr << "Open dialog is Windows-only; pass an image path on the command line.\n";
    return false;
}

bool saveImage(const cv::Mat& img) {
    if (img.empty()) return false;
    std::vector<uchar> bytes;
    if (!cv::imencode(".png", img, bytes)) return false;
    return writeFileBytes(std::filesystem::path("edited.png"), bytes);
}

cv::Mat loadImage(const std::string& path) {
    return decodeImageFile(std::filesystem::path(path));
}

void errorBox(const std::string& message) {
    std::cerr << "MyEditor: " << message << "\n";
}

}  // namespace ui
#endif