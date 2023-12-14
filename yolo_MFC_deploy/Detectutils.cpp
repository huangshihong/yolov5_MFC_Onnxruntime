#include "pch.h"
#include "Detectutils.h"


size_t Detectutils::vectorProduct(const std::vector<int64_t>& vector)
{
    if (vector.empty())
        return 0;

    size_t product = 1;
    for (const auto& element : vector)
        product *= element;

    return product;
}

std::wstring Detectutils::charToWstring(const char* str)
{
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.from_bytes(str);
}

std::vector<std::string> Detectutils::loadNames(const std::string& path)
{
    // load class names
    std::vector<std::string> classNames;
    std::ifstream infile(path);
    if (infile.good())
    {
        std::string line;
        while (getline(infile, line))
        {
            if (line.back() == '\r')
                line.pop_back();
            classNames.emplace_back(line);
        }
        infile.close();
    }
    else
    {
        std::cerr << "ERROR: Failed to access class name path: " << path << std::endl;
    }

    return classNames;
}


void Detectutils::visualizeDetection(cv::Mat& image, std::vector<Detection>& detections,
    const std::vector<std::string>& classNames)
{
    for (const Detection& detection : detections)
    {
        cv::rectangle(image, detection.box, cv::Scalar(255, 255, 114), 2);

        int x = detection.box.x;
        int y = detection.box.y;

        int conf = (int)std::round(detection.conf * 100);
        int classId = detection.classId;
        if (classId < classNames.size())
        {
            std::string label = classNames[classId] + "0." + std::to_string(conf);

            int baseline = 0;
            cv::Size size = cv::getTextSize(label, cv::FONT_ITALIC, 0.5, 2, &baseline);

            //cv::rectangle(image,
            //              cv::Point(x, y - 25), cv::Point(x + size.width, y),
            //              cv::Scalar(229, 160, 21), -1);

            cv::putText(image, label,
                cv::Point(x, y - 3), cv::FONT_ITALIC,
                0.6, cv::Scalar(255, 255, 114), 2);
        }
        else
        {
            std::string label = std::to_string(classId) + " 0." + std::to_string(conf);

            int baseline = 0;
            cv::Size size = cv::getTextSize(label, cv::FONT_ITALIC, 0.5, 2, &baseline);

            //cv::rectangle(image,
            //              cv::Point(x, y - 25), cv::Point(x + size.width, y),
            //              cv::Scalar(229, 160, 21), -1);

            cv::putText(image, label,
                cv::Point(x, y - 3), cv::FONT_ITALIC,
                0.6, cv::Scalar(255, 255, 114), 2);
        }

        

    }
}




void Detectutils::letterbox(const cv::Mat& image, cv::Mat& outImage,
    const cv::Size& newShape = cv::Size(640, 640),
    const cv::Scalar& color = cv::Scalar(114, 114, 114),
    bool auto_ = true,
    bool scaleFill = false,
    bool scaleUp = true,
    int stride = 32)
{
    cv::Size shape = image.size();
    double r = std::min((double)newShape.height / (double)shape.height,
        (double)newShape.width / (double)shape.width);
    if (!scaleUp)
        r = std::min(r, 1.0);

    double ratio[2]{ r, r };
    double newUnpad[2]{ (int)std::round((double)shape.width * r),
                     (int)std::round((double)shape.height * r) };

    double dw = (double)(double(newShape.width) - newUnpad[0]);
    double dh = (double)(double(newShape.height) - newUnpad[1]);


    if (auto_)
    {
        dw = (double)((int)dw % stride);
        dh = (double)((int)dh % stride);

    }
    else if (scaleFill)
    {
        dw = 0.0f;
        dh = 0.0f;
        newUnpad[0] = newShape.width;
        newUnpad[1] = newShape.height;
        ratio[0] = (double)newShape.width / (double)shape.width;
        ratio[1] = (double)newShape.height / (double)shape.height;
    }

    dw /= 2.0f;
    dh /= 2.0f;
    std::cout << "r" << r << std::endl;
    std::cout << dw << std::endl;
    std::cout << dh << std::endl;

    std::cout << newUnpad[0] << std::endl;
    std::cout << newUnpad[1] << std::endl;
    std::cout << shape.width << std::endl;
    std::cout << shape.height << std::endl;



    if (shape.width != newUnpad[0] || shape.height != newUnpad[1])
    {
        cv::resize(image, outImage, cv::Size(newUnpad[0], newUnpad[1]));
        std::cout << " xxxxxxxxxxxxxxxxxx" << std::endl;
       
    }
    else 
    {
        image.copyTo(outImage);
    }

    

    int top = int(std::round(dh - 0.1f));
    int bottom = int(std::round(dh + 0.1f));
    int left = int(std::round(dw - 0.1f));
    int right = int(std::round(dw + 0.1f));
    cv::copyMakeBorder(outImage, outImage, top, bottom, left, right, cv::BORDER_CONSTANT, color);

}

void Detectutils::scaleCoords(const cv::Size& imageShape, cv::Rect& coords, const cv::Size& imageOriginalShape)
{
    float gain = std::min((float)imageShape.height / (float)imageOriginalShape.height,
        (float)imageShape.width / (float)imageOriginalShape.width);

    int pad[2] = { (int)(((float)imageShape.width - (float)imageOriginalShape.width * gain) / 2.0f),
                  (int)(((float)imageShape.height - (float)imageOriginalShape.height * gain) / 2.0f) };

    coords.x = (int)std::round(((float)(coords.x - pad[0]) / gain));
    coords.y = (int)std::round(((float)(coords.y - pad[1]) / gain));

    coords.width = (int)std::round(((float)coords.width / gain));
    coords.height = (int)std::round(((float)coords.height / gain));

    // // clip coords, should be modified for width and height
    // coords.x =Detectutils::clip(coords.x, 0, imageOriginalShape.width);
    // coords.y =Detectutils::clip(coords.y, 0, imageOriginalShape.height);
    // coords.width =Detectutils::clip(coords.width, 0, imageOriginalShape.width);
    // coords.height =Detectutils::clip(coords.height, 0, imageOriginalShape.height);
}

void Detectutils::scaleCoords_f(const cv::Size& imageShape, cv::Rect2f& coords, const cv::Size& imageOriginalShape)
{
    float gain = std::min((float)imageShape.height / (float)imageOriginalShape.height,
        (float)imageShape.width / (float)imageOriginalShape.width);

    int pad[2] = { (int)(((float)imageShape.width - (float)imageOriginalShape.width * gain) / 2.0f),
                  (int)(((float)imageShape.height - (float)imageOriginalShape.height * gain) / 2.0f) };

    coords.x = (float)(coords.x - pad[0]) / gain;
    coords.y = (float)(coords.y - pad[1]) / gain;

    coords.width = (float)coords.width / gain;
    coords.height = (float)coords.height / gain;


}

template <typename T>
T Detectutils::clip(const T& n, const T& lower, const T& upper)
{
    return std::max(lower, std::min(n, upper));
}

Timer_ms::Timer_ms()
    : t1(res::zero())
    , t2(res::zero()) {
    tic();
}

Timer_ms::~Timer_ms() {}

void Timer_ms::tic() {
    t1 = clock::now();
}

int Timer_ms::toc(const char* str) {
    t2 = clock::now();
    std::cout << str << " time: "
        << std::chrono::duration_cast<res>(t2 - t1).count() / 1e3 << "ms." << std::endl;
    return std::chrono::duration_cast<res>(t2 - t1).count() / 1e3;
}
