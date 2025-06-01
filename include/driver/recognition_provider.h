#pragma once
#include <opencv2/core.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/dnn.hpp>

class RecognitionProvider {
public:
    explicit RecognitionProvider(const std::string& modelPath);
    float estimate_body_thickness(const cv::Mat& image);
    
private:
    cv::dnn::Net net;
    const cv::Size inputSize{224, 224};
};