#include "driver/recognition_provider.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// 1. Конструктор / Деструктор
////////////////////////////////////////////////////////////////////////////////

RecognitionProvider::RecognitionProvider() {
    cap.open(0);
    if (!cap.isOpened()) {
        throw std::runtime_error("RecognitionProvider: не удалось открыть камеру 0");
    }
}

RecognitionProvider::~RecognitionProvider() {
    cap.release();
}

////////////////////////////////////////////////////////////////////////////////
// 2. capture_frame: захват одного кадра (CV_8UC3 BGR)
////////////////////////////////////////////////////////////////////////////////

cv::Mat RecognitionProvider::capture_frame() {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        throw std::runtime_error("RecognitionProvider: получен пустой кадр с камеры");
    }
    return frame;
}

////////////////////////////////////////////////////////////////////////////////
// 3. encodeMatToPNG: кодирование CV_8U Mat → PNG (std::vector<unsigned char>)
////////////////////////////////////////////////////////////////////////////////

std::vector<unsigned char> RecognitionProvider::encodeMatToPNG(const cv::Mat& mat) {
    std::vector<unsigned char> buffer;
    std::vector<int> params = {cv::IMWRITE_PNG_COMPRESSION, 3};
    if (!cv::imencode(".png", mat, buffer, params)) {
        throw std::runtime_error("encodeMatToPNG: ошибка кодирования в PNG");
    }
    return buffer;
}

////////////////////////////////////////////////////////////////////////////////
// 4. preprocess_and_get_width:
//     Original → Gray → Blur → Thresh+Contours → рисует brect → возвращает width
////////////////////////////////////////////////////////////////////////////////

int RecognitionProvider::preprocess_and_get_width(const cv::Mat& frame, const std::string& window_prefix) {
    imagesBuffer.clear();

    // 1) Original (CV_8UC3)
    imagesBuffer.push_back(encodeMatToPNG(frame));
    cv::imshow(window_prefix + "_Original", frame);

    // 2) Gray (CV_8UC1)
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    imagesBuffer.push_back(encodeMatToPNG(gray));
    cv::imshow(window_prefix + "_Gray", gray);

    // 3) GaussianBlur (CV_8UC1)
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0);
    imagesBuffer.push_back(encodeMatToPNG(blur));
    cv::imshow(window_prefix + "_Blur", blur);

    // 4) threshold → Thresh (CV_8UC1)
    cv::Mat thresh;
    cv::threshold(blur, thresh, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    // Найдём контуры
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Выберем самый большой контур по площади
    size_t best_idx = 0;
    double max_area = 0.0;
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area > max_area) {
            max_area = area;
            best_idx = i;
        }
    }

    // bounding rectangle (CV_8UC3 для рисования)
    cv::Mat drawn = frame.clone();
    int width = 0;
    if (!contours.empty()) {
        cv::Rect brect = cv::boundingRect(contours[best_idx]);
        width = brect.width;
        cv::rectangle(drawn, brect, cv::Scalar(0, 255, 0), 2);
    }
    imagesBuffer.push_back(encodeMatToPNG(drawn));
    cv::imshow(window_prefix + "_Thresh", drawn);

    cv::waitKey(1);
    return width;
}

////////////////////////////////////////////////////////////////////////////////
// 5. estimate_body_build: вычисляем boundingRect.width и height, ratio → SMALL/MEDIUM/LARGE
////////////////////////////////////////////////////////////////////////////////

BodyBuild RecognitionProvider::estimate_body_build() {
    // Захват кадра
    cv::Mat frame = capture_frame();

    // Выполним те же 4 этапа, но нам нужно ПОЛУЧИТЬ width и height:
    // Степень свободы: перепишем preprocess, чтобы вернуть оба (ширину и высоту).

    // Здесь «хитрость»: используем preprocess_and_get_width только для ширины,
    // но для телосложения надо знать и высоту. Поэтому сделаем чуть-чуть иначе:
    //
    // Дублируем код preprocess, но получим и width, и height:

    imagesBuffer.clear();
    cv::imshow("estimate_body_build_Original", frame);
    imagesBuffer.push_back(encodeMatToPNG(frame));

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    imagesBuffer.push_back(encodeMatToPNG(gray));
    cv::imshow("estimate_body_build_Gray", gray);

    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0);
    imagesBuffer.push_back(encodeMatToPNG(blur));
    cv::imshow("estimate_body_build_Blur", blur);

    cv::Mat thresh;
    cv::threshold(blur, thresh, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Найдём самый большой контур
    size_t idx = 0;
    double max_area = 0.0;
    for (size_t i = 0; i < contours.size(); ++i) {
        double a = cv::contourArea(contours[i]);
        if (a > max_area) {
            max_area = a;
            idx = i;
        }
    }

    cv::Mat drawn = frame.clone();
    BodyBuild result = BodyBuild::MEDIUM;
    if (!contours.empty()) {
        cv::Rect brect = cv::boundingRect(contours[idx]);
        int w = brect.width;
        int h = brect.height;
        cv::rectangle(drawn, brect, cv::Scalar(0, 255, 0), 2);

        // ratio = w/h
        if (h > 0) {
            float ratio = static_cast<float>(w) / static_cast<float>(h);
            if (ratio < 0.3f)       result = BodyBuild::SMALL;
            else if (ratio < 0.4f)  result = BodyBuild::MEDIUM;
            else                    result = BodyBuild::LARGE;
        }
    }
    imagesBuffer.push_back(encodeMatToPNG(drawn));
    cv::imshow("estimate_body_build_Thresh", drawn);
    cv::waitKey(1);

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// 6. estimate_body_thickness: просто возвращаем ширину boundingRect
////////////////////////////////////////////////////////////////////////////////

float RecognitionProvider::estimate_body_thickness() {
    cv::Mat frame = capture_frame();
    int width = preprocess_and_get_width(frame, "estimate_body_thickness");
    return static_cast<float>(width);
}

////////////////////////////////////////////////////////////////////////////////
// 7. GetImages: возвращает buffer из 4 PNG
////////////////////////////////////////////////////////////////////////////////

std::vector<std::vector<unsigned char>> RecognitionProvider::GetImages() const {
    return imagesBuffer;
}

////////////////////////////////////////////////////////////////////////////////
// 8. GetVideo: в цикле показываем Original→Gray→Blur→Thresh (для каждого кадра)
////////////////////////////////////////////////////////////////////////////////

void RecognitionProvider::GetVideo() {
    while (true) {
        cv::Mat frame = capture_frame();
        preprocess_and_get_width(frame, "LiveFrame");

        int key = cv::waitKey(1);
        if (key == 27) break; // Esc
    }
    cv::destroyAllWindows();
}
