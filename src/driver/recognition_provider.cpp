#include "driver/recognition_provider.h"

RecognitionProvider::RecognitionProvider(const std::string& modelPath) {
    //net = cv::dnn::readNet(modelPath);
}

float RecognitionProvider::estimate_body_thickness(const cv::Mat& image) {
    
    // cv::Mat processed;
    // if (image.channels() == 1) {
    //     cv::cvtColor(image, processed, cv::COLOR_GRAY2RGB);
    // } else {
    //     cv::cvtColor(image, processed, cv::COLOR_BGR2RGB);
    // }
    // cv::resize(processed, processed, inputSize);
    // cv::Mat blob = cv::dnn::blobFromImage(image, 1.0/127.5, inputSize, 
    //                                     cv::Scalar(127.5, 127.5, 127.5), true);
    // //net.setInput(blob);
    // cv::Mat output = net.forward();
    constexpr float tmp{0};
    return tmp;
    //return output.at<float>(0);
}