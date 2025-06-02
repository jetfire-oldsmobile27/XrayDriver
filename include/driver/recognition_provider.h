#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <stdexcept>
#include <string>

/**
 * \brief Три категории телосложения человека
 */
enum class BodyBuild {
    SMALL,
    MEDIUM,
    LARGE
};

/**
 * \brief RecognitionProvider (на базе OpenCV-алгоритмов)
 *
 * Работает исключительно с камерой (index 0), без ONNX/нейросетей.
 * Использует базовую обработку кадра, чтобы:
 *  - Найти силуэт человека (threshold + контуры)
 *  - На основании bounding‐rectangle классифицировать телосложение (SMALL/​MEDIUM/​LARGE)
 *  - Для «толщины» возвращает ширину bounding‐rectangle (в пикселях) как proxy
 *
 * По каждому вызову методов:
 * 1) estimate_body_build() или estimate_body_thickness():
 *    – захватывает кадр
 *    – атакует его алгоритмами: Original → Gray → Blur → Thresh+Contours
 *    – сохраняет 4 промежуточных изображения (PNG-байты) в imagesBuffer и показывает окна
 *    – возвращает соответствующий результат (BodyBuild или float)
 *
 * 2) GetImages(): возвращает последний буфер PNG (4 элемента)
 * 3) GetVideo(): непрерывно в цикле показывает те же 4 окна обработки для каждого кадра
 *
 */
class RecognitionProvider {
public:
    /**
     * \brief Конструктор
     *
     * Открывает камеру index = 0. Если открыть не удалось, бросает std::runtime_error.
     */
    RecognitionProvider();
    ~RecognitionProvider();

    /**
     * \brief Оценивает телосложение (SMALL/​MEDIUM/​LARGE)
     *
     * @return BodyBuild::SMALL / MEDIUM / LARGE
     *  Когда вызывается, imagesBuffer заполняется четырьмя PNG:
     *    [0] — Original (BGR)
     *    [1] — Gray     (CV_8UC1)
     *    [2] — Blur     (CV_8UC1)
     *    [3] — Thresh + Contours (CV_8UC3)
     */
    BodyBuild estimate_body_build();

    /**
     * \brief Оценивает «толщину» (ширину bounding‐rectangle) в пикселях
     *
     * @return float — ширина bounding‐rectangle, найденного по контурному силуэту.
     *  Когда вызывается, imagesBuffer заполняется четырьмя PNG (как в estimate_body_build).
     */
    float estimate_body_thickness();

    /**
     * \brief Возвращает буфер PNG (4 элемента) из последнего распознавания
     */
    std::vector<std::vector<unsigned char>> GetImages() const;

    /**
     * \brief Демонстрация в реальном времени (Original→Gray→Blur→Thresh)
     *
     * В бесконечном цикле захватывает кадр, строит:
     *  – Original (BGR)
     *  – Gray
     *  – Blur
     *  – Thresh+Contours
     * и показывает четыре окна. Выход по Esc.
     */
    void GetVideo();

private:
    cv::VideoCapture cap;
    std::vector<std::vector<unsigned char>> imagesBuffer;

    /**
     * \brief Захватывает один кадр. Бросает std::runtime_error, если пустой.
     */
    cv::Mat capture_frame();

    /**
     * \brief Предобрабатывает frame → Gray → Blur → Thresh+Contours.
     *
     * 1) Конвертация BGR→Gray
     * 2) GaussianBlur
     * 3) threshold (черно‐белый)
     * 4) findContours → самый большой контур → boundingRect
     * 5) Рисует прямоугольник на копии оригинала (для визуализации)
     *
     * Заполняет imagesBuffer четырьмя этапами (PNG):
     *   buffer[0] = Original (BGR)
     *   buffer[1] = Gray     (CV_8UC1)
     *   buffer[2] = Blur     (CV_8UC1)
     *   buffer[3] = Thresh+Contours (CV_8UC3, цветной; прямоугольник обведён)
     *
     * @param frame Исходный CV_8UC3 BGR
     * @param window_prefix Префикс для имен окон
     * @return boundingRect.width (в пикселях)
     */
    int preprocess_and_get_width(const cv::Mat& frame, const std::string& window_prefix);

    /**
     * \brief Кодирует Mat (любого CV_8U) в PNG-байты.
     */
    static std::vector<unsigned char> encodeMatToPNG(const cv::Mat& mat);
};
