#include "driver/power_regulator.h"
#include "service/logger.h"

PowerRegulator::PowerRegulator(RecognitionProvider& recognizer, float Kp)
    : recognizer(recognizer), Kp(Kp) {}

float PowerRegulator::adjust_power(const cv::Mat& image, float current_power) {
    try {
        float thickness = 0.0f;
        // float thickness = recognizer.estimate_body_thickness(image);
        return current_power + Kp * thickness;
    } catch (const cv::Exception& e) {
        jetfire27::Engine::Logging::Logger::GetInstance().Error(
            "OpenCV error: {}", e.what());
        return current_power; 
    }
}