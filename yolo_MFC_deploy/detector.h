#pragma once
#include <utility>
#include "Detectutils.h"
#include <vector>


class YOLODetector
{
public:
    YOLODetector(const std::string& modelPath, const bool& isGPU, const cv::Size& inputSize, int set_batch_);

    std::vector<Detection> detect(cv::Mat& image, const float& confThreshold, const float& iouThreshold);
    std::vector<std::vector<Detection>> detect_batch(std::vector<cv::Mat>& image, const float& confThreshold, const float& iouThreshold);
    std::vector<std::string> GetdeteClassName();
    

private:
    Ort::Env env{ nullptr };
    Ort::SessionOptions sessionOptions{ nullptr };
    Ort::Session session{ nullptr };

    void preprocessing(cv::Mat& image, float*& blob, std::vector<int64_t>& inputTensorShape);
    std::vector<Detection> postprocessing(const cv::Size& resizedImageShape,
        const cv::Size& originalImageShape,
        std::vector<Ort::Value>& outputTensors,
        const float& confThreshold, const float& iouThreshold);

    

    std::vector<std::vector<Detection>> postprocessing_batch(cv::Size& resizedImageShape, std::vector<cv::Mat>& orgImage, std::vector<Ort::Value>& outputTensors, const float& confThreshold, const float& iouThreshold);




    void batch_preprocessing(std::vector<cv::Mat>& image, std::vector<cv::Mat>& output, std::vector<int64_t>& inputTensorShape);



    static void getBestClassInfo(std::vector<float>::iterator it, const int& numClasses,
        float& bestConf, int& bestClassId);

    std::vector<const char*> inputNames;
    std::vector<const char*> outputNames;
    bool isDynamicInputShape{};
    cv::Size2f inputImageShape;
    int infer_batch;

};
