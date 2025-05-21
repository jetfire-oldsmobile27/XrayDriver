#pragma once
#include "recognition_provider.h"

class PowerRegulator {
public:
    PowerRegulator(RecognitionProvider& recognizer, float Kp = 0.5f);
    float adjust_power(const cv::Mat& image, float current_power);
    
private:
    RecognitionProvider& recognizer;
    const float Kp; // Пропорциональный коэффициент
};