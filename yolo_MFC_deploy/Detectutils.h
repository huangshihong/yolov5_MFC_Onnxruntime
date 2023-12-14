#pragma once
#include <codecvt>
#include <fstream>
#include <opencv2/opencv.hpp>


class Timer_ms {
public:
    Timer_ms();
    ~Timer_ms();

    void tic();
    int toc(const char* str);

private:
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::microseconds res;

    clock::time_point t1;
    clock::time_point t2;
};

struct Detection
{
    cv::Rect box;
    float conf{};
    int classId{};
};
struct Detection2f
{
    cv::Rect2f box;
    float conf{};
    int classId{};
};

namespace Detectutils
{
    size_t vectorProduct(const std::vector<int64_t>& vector);
    std::wstring charToWstring(const char* str);
    std::vector<std::string> loadNames(const std::string& path);
    void visualizeDetection(cv::Mat& image, std::vector<Detection>& detections,
        const std::vector<std::string>& classNames);

   

    void letterbox(const cv::Mat& image, cv::Mat& outImage,
        const cv::Size& newShape,
        const cv::Scalar& color,
        bool auto_,
        bool scaleFill,
        bool scaleUp,
        int stride);

    void scaleCoords(const cv::Size& imageShape, cv::Rect& coords, const cv::Size& imageOriginalShape);
    void scaleCoords_f(const cv::Size& imageShape, cv::Rect2f& coords, const cv::Size& imageOriginalShape);

    template <typename T>
    T clip(const T& n, const T& lower, const T& upper);
}
