#include "pch.h"
#include "detector.h"

YOLODetector::YOLODetector(const std::string& modelPath,
    const bool& isGPU = true,
    const cv::Size& inputSize = cv::Size(640, 640), int set_batch_ = 1)
{
    env = Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "ONNX_DETECTION");
    sessionOptions = Ort::SessionOptions();

    std::vector<std::string> availableProviders = Ort::GetAvailableProviders();
    auto cudaAvailable = std::find(availableProviders.begin(), availableProviders.end(), "CUDAExecutionProvider");
    OrtCUDAProviderOptions cudaOption;

    if (isGPU && (cudaAvailable == availableProviders.end()))
    {
        std::cout << "GPU is not supported by your ONNXRuntime build. Fallback to CPU." << std::endl;
        std::cout << "Inference device: CPU" << std::endl;
    }
    else if (isGPU && (cudaAvailable != availableProviders.end()))
    {
        std::cout << "Inference device: GPU" << std::endl;
        sessionOptions.AppendExecutionProvider_CUDA(cudaOption);
    }
    else
    {
        std::cout << "Inference device: CPU" << std::endl;
    }

#ifdef _WIN32
    std::wstring w_modelPath = Detectutils::charToWstring(modelPath.c_str());
    session = Ort::Session(env, w_modelPath.c_str(), sessionOptions);


    //session2= Ort::Session(env, w_modelPath.c_str(), sessionOptions);
#else
    session = Ort::Session(env, modelPath.c_str(), sessionOptions);
#endif

    Ort::AllocatorWithDefaultOptions allocator;

    Ort::TypeInfo inputTypeInfo = session.GetInputTypeInfo(0);
    std::vector<int64_t> inputTensorShape = inputTypeInfo.GetTensorTypeAndShapeInfo().GetShape();
    this->isDynamicInputShape = false;
    // checking if width and height are dynamic
    if (inputTensorShape[2] == -1 && inputTensorShape[3] == -1)
    {
        std::cout << "Dynamic input shape" << std::endl;
        this->isDynamicInputShape = true;
        this->inputImageShape = cv::Size2f(inputSize);
    }
    if (inputTensorShape[0] == -1)
    {
        inputTensorShape[0] = set_batch_;
        std::cout << "Dynamic input batch shape" << std::endl;
    }
    else
    {
        inputTensorShape[0] = set_batch_;
        infer_batch = set_batch_;
        this->inputImageShape = cv::Size2f(inputSize);

    }

    for (auto shape : inputTensorShape)
        std::cout << "Input shape: " << shape << std::endl;

    auto input_num = session.GetInputCount(), output_num = session.GetOutputCount();

    for (size_t i = 0; i < input_num; ++i)
    {

        inputNames.push_back(_strdup(session.GetInputNameAllocated(i, allocator).get()));

    }

    for (size_t i = 0; i < output_num; ++i)
    {

        outputNames.push_back(_strdup(session.GetOutputNameAllocated(i, allocator).get()));
    }
    std::cout << "Input name: " << inputNames[0] << std::endl;
    std::cout << "Output name: " << outputNames[0] << std::endl;

   
}

void YOLODetector::getBestClassInfo(std::vector<float>::iterator it, const int& numClasses,
    float& bestConf, int& bestClassId)
{
    // first 5 element are box and obj confidence
    bestClassId = 5;
    bestConf = 0;

    for (int i = 5; i < numClasses + 5; i++)
    {
        if (it[i] > bestConf)
        {
            bestConf = it[i];
            bestClassId = i - 5;
        }
    }

}

void YOLODetector::preprocessing(cv::Mat& image, float*& blob, std::vector<int64_t>& inputTensorShape)
{
    cv::Mat resizedImage, floatImage;

    cv::cvtColor(image, resizedImage, cv::COLOR_BGR2RGB);
    Detectutils::letterbox(resizedImage, resizedImage, this->inputImageShape,
        cv::Scalar(114, 114, 114), this->isDynamicInputShape,
        false, true, 32);

    inputTensorShape[2] = resizedImage.rows;
    inputTensorShape[3] = resizedImage.cols;

    resizedImage.convertTo(floatImage, CV_32FC3, 1 / 255.0);
    blob = new float[floatImage.cols * floatImage.rows * floatImage.channels()];
    cv::Size floatImageSize{ floatImage.cols, floatImage.rows };

    // hwc -> chw
    std::vector<cv::Mat> chw(floatImage.channels());
    for (long i = 0; i < floatImage.channels(); ++i)
    {
        chw[i] = cv::Mat(floatImageSize, CV_32FC1, blob + i * floatImageSize.width * floatImageSize.height);

    }

    cv::split(floatImage, chw);
}





std::vector<std::string> YOLODetector::GetdeteClassName()//获取类别
{

    Ort::AllocatorWithDefaultOptions allocator;
    std::string metadata = session.GetModelMetadata().LookupCustomMetadataMapAllocated("names", allocator).get();
    size_t begin = 0, end = 0;
    std::vector<std::string> label_name;
    for (int i = 0; i < metadata.size(); i++)
    {
        if (metadata[i] == '\'')
        {
            int x = i - 1;
            if (metadata[x] == ' ') {
                begin = i + 1;

            }
            else
            {
                end = i;
                label_name.push_back(metadata.substr(begin, end - begin));
            }
        }

    }
    return label_name;

}

std::vector<Detection> YOLODetector::postprocessing(const cv::Size& resizedImageShape, const cv::Size& originalImageShape, std::vector<Ort::Value>& outputTensors, const float& confThreshold, const float& iouThreshold)
{



    auto* rawOutput = outputTensors[0].GetTensorData<float>();

    std::vector<int64_t> outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
    size_t count = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
    std::vector<float> output(rawOutput, rawOutput + count);

    // for (const int64_t& shape : outputShape)
    //     std::cout << "Output Shape: " << shape << std::endl;

    // first 5 elements are box[4] and obj confidence
    int numClasses = (int)outputShape[2] - 5;
    int elementsInBatch = (int)(outputShape[1] * outputShape[2]);

    // only for batch size = 1
    std::map<int, std::vector<cv::Rect>> classBoxes;//分类别的nms
    std::map<int, std::vector<float>>  classConfs;
    for (auto it = output.begin(); it != output.begin() + elementsInBatch; it += outputShape[2])
    {
        float clsConf = it[4];

        if (clsConf > confThreshold)
        {
            int centerX = (int)(it[0]);
            int centerY = (int)(it[1]);
            int width = (int)(it[2]);
            int height = (int)(it[3]);
            int left = centerX - width / 2;
            int top = centerY - height / 2;

            float objConf;
            int classId;
            this->getBestClassInfo(it, numClasses, objConf, classId);

            float confidence = clsConf * objConf;
            //float confidence = objConf;

            classBoxes[classId].emplace_back(left, top, width, height);
            classConfs[classId].emplace_back(confidence);
            //boxes.emplace_back(left, top, width, height);
            //confs.emplace_back(confidence);
            //classIds.emplace_back(classId);

        }
    }


    std::vector<Detection> detections;
    // Apply class-wise NMS
    for (const auto& kv : classBoxes)
    {
        int classId = kv.first;
        const std::vector<cv::Rect>& boxes = kv.second;
        const std::vector<float>& confs = classConfs.at(classId);

        std::vector<int> nmsIndices;
        cv::dnn::NMSBoxes(boxes, confs, confThreshold, iouThreshold, nmsIndices);

        for (int idx : nmsIndices)
        {
            Detection det;
            det.box = cv::Rect(boxes[idx]);
            Detectutils::scaleCoords(resizedImageShape, det.box, originalImageShape);

            det.conf = confs[idx];
            det.classId = classId;
            detections.emplace_back(det);
        }
    }


    return detections;

}



std::vector<std::vector<Detection>> YOLODetector::postprocessing_batch(cv::Size& resizedImageShape, std::vector<cv::Mat>& orgImage, std::vector<Ort::Value>& outputTensors, const float& confThreshold, const float& iouThreshold)
{


    auto* rawOutput = outputTensors[0].GetTensorData<float>();
    std::vector<std::vector<Detection>> result;

    std::vector<int64_t> outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
    size_t count = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();


    int batch = outputShape[0];
    int elementsInBatch = (int)(outputShape[1] * outputShape[2]);



    for (int i = 0; i < batch; i++)
    {
        std::vector<float> output(rawOutput + elementsInBatch * i, rawOutput + elementsInBatch * (i + 1));



        cv::Size originalImageShape = orgImage[i].size();








        // first 5 elements are box[4] and obj confidence
        int numClasses = (int)outputShape[2] - 5;

        std::map<int, std::vector<cv::Rect>> classBoxes;//分类别的nms
        std::map<int, std::vector<float>>  classConfs;
        // only for batch size = 1
        for (auto it = output.begin(); it != output.begin() + elementsInBatch; it += outputShape[2])
        {
            float clsConf = it[4];

            if (clsConf > confThreshold)
            {
                int centerX = (int)(it[0]);
                int centerY = (int)(it[1]);
                int width = (int)(it[2]);
                int height = (int)(it[3]);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                float objConf;
                int classId;
                this->getBestClassInfo(it, numClasses, objConf, classId);

                float confidence = clsConf * objConf;
                //float confidence = objConf;

                classBoxes[classId].emplace_back(left, top, width, height);
                classConfs[classId].emplace_back(confidence);
            }
        }

        std::vector<int> indices;        
        std::vector<Detection> detections;
        // Apply class-wise NMS
        for (const auto& kv : classBoxes)
        {
            int classId = kv.first;
            const std::vector<cv::Rect>& boxes = kv.second;
            const std::vector<float>& confs = classConfs.at(classId);

            std::vector<int> nmsIndices;
            cv::dnn::NMSBoxes(boxes, confs, confThreshold, iouThreshold, nmsIndices);
            for (int idx : nmsIndices)
            {
                Detection det;
                det.box = cv::Rect(boxes[idx]);
                Detectutils::scaleCoords(resizedImageShape, det.box, originalImageShape);

                det.conf = confs[idx];
                det.classId = classId;
                detections.emplace_back(det);
            }
        }
        result.push_back(detections);
    }



    return result;
}




void YOLODetector::batch_preprocessing(std::vector<cv::Mat>& image, std::vector<cv::Mat>& output, std::vector<int64_t>& inputTensorShape)
{


    std::vector<cv::Mat> output_vec(image.size());

    for (int i = 0; i < image.size(); i++)
    {
        cv::Mat resizedImage;

        Detectutils::letterbox(image[i], resizedImage, this->inputImageShape,
            cv::Scalar(114, 114, 114), this->isDynamicInputShape,
            false, true, 32);

        inputTensorShape[2] = resizedImage.rows;
        inputTensorShape[3] = resizedImage.cols;
        output_vec[i] = resizedImage;



    }
    output = output_vec;

}



std::vector<Detection> YOLODetector::detect(cv::Mat& image, const float& confThreshold = 0.3,
    const float& iouThreshold = 0.45)
{
    float* blob = nullptr;

  

    std::vector<int64_t> inputTensorShape{ 1, 3, -1, -1 };
    this->preprocessing(image, blob, inputTensorShape);
    //t1.toc("The protect time is");

    auto startTime1 = clock();




    size_t inputTensorSize = Detectutils::vectorProduct(inputTensorShape);

    std::vector<float> inputTensorValues(blob, blob + inputTensorSize);

    std::vector<Ort::Value> inputTensors;



    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    inputTensors.push_back(Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensorValues.data(), inputTensorSize,
        inputTensorShape.data(), inputTensorShape.size()
        ));

    //T_cpu2gpu.toc("the cpu 2 gpu time");

    std::vector<Ort::Value> outputTensors = this->session.Run(Ort::RunOptions{ nullptr },
        inputNames.data(),
        inputTensors.data(),
        1,
        outputNames.data(),
        1);
    //T_infer.toc("the infer time is");


    //std::cout << inputTensors.size() << std::endl;
    cv::Size resizedShape = cv::Size((int)inputTensorShape[3], (int)inputTensorShape[2]);
    auto endTime1 = clock();
    //t2.toc("run time is");

    //t3.tic();
    //std::cout << "The run time is: " << (double)(endTime1 - startTime1) / CLOCKS_PER_SEC << "s" << std::endl;
    auto startTime2 = clock();
    std::vector<Detection> result = this->postprocessing(resizedShape,
        image.size(),
        outputTensors,
        confThreshold, iouThreshold);

    delete[] blob;
    auto endTime2 = clock();
    //t3.toc("post time is");
    //std::cout << "The post time is: " << (double)(endTime2 - startTime2) / CLOCKS_PER_SEC << "s" << std::endl;



    return result;
}


std::vector<std::vector<Detection>> YOLODetector::detect_batch(std::vector<cv::Mat>& image, const float& confThreshold = 0.3,
    const float& iouThreshold = 0.45)
{
    //float* blob = nullptr;
    std::vector<cv::Mat> resize_images;


    //std::vector<float> inputTensorValues;

    std::vector<int64_t> inputTensorShape{ infer_batch, 3, -1, -1 };

    this->batch_preprocessing(image, resize_images, inputTensorShape);

    cv::Mat blob = cv::dnn::blobFromImages(resize_images, 1 / 255.0, this->inputImageShape, cv::Scalar(0, 0, 0), true, false);


    auto startTime1 = clock();




    size_t inputTensorSize = Detectutils::vectorProduct(inputTensorShape);



    std::vector<Ort::Value> inputTensors;
    std::vector<Ort::Value> outputTensors;


    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    inputTensors.push_back(Ort::Value::CreateTensor<float>(
        memoryInfo, (float*)blob.data, inputTensorSize,
        inputTensorShape.data(), inputTensorShape.size()
        ));

    outputTensors = this->session.Run(Ort::RunOptions{ nullptr },
        inputNames.data(),
        inputTensors.data(),
        1,
        outputNames.data(),
        1);

    //T_infer.toc("the infer time is");


    //std::cout << inputTensors.size() << std::endl;
    cv::Size resizedShape = cv::Size((int)inputTensorShape[3], (int)inputTensorShape[2]);
    auto endTime1 = clock();
    //t2.toc("run time is");
   
    //std::cout << "The run time is: " << (double)(endTime1 - startTime1) / CLOCKS_PER_SEC << "s" << std::endl;
    auto startTime2 = clock();
    std::vector<std::vector<Detection>> result = this->postprocessing_batch(resizedShape,
        image,
        outputTensors,
        confThreshold, iouThreshold);


    auto endTime2 = clock();
 



    return result;
}