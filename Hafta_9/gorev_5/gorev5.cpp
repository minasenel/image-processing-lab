%%bash
cat << 'EOF' > week9_task5_skin_detection.cpp
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // 1. Goruntuyu ac (RGB/BGR renk uzayinda)
    std::string input_path = "/content/people.jpg";
    cv::Mat bgr = cv::imread(input_path, cv::IMREAD_COLOR);

    if (bgr.empty()) {
        std::cerr << "[HATA] " << input_path << " bulunamadi!" << std::endl;
        return -1;
    }
    std::cout << "Goruntu acildi: " << bgr.cols << "x" << bgr.rows << std::endl;

    // 2. Ten rengini tespit etmek icin YCrCb renk uzayina donustur.
    // YCrCb, aydinlatma (Y) ile renk (Cr, Cb) bilgisini ayirdigi icin
    // ten rengi tespitinde RGB/HSV'den daha kararlidir.
    cv::Mat ycrcb;
    cv::cvtColor(bgr, ycrcb, cv::COLOR_BGR2YCrCb);

    // Literaturde yaygin kullanilan ten rengi esik degerleri (YCrCb):
    // 0   <= Y  <= 255
    // 133 <= Cr <= 173
    // 77  <= Cb <= 127
    cv::Scalar lower(0, 133, 77);
    cv::Scalar upper(255, 173, 127);

    cv::Mat skinMask;
    cv::inRange(ycrcb, lower, upper, skinMask);

    // Maskeyi hafifce temizle (gurultu/delikleri azalt)
    cv::morphologyEx(skinMask, skinMask, cv::MORPH_OPEN,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::morphologyEx(skinMask, skinMask, cv::MORPH_CLOSE,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

    // 3. Ten rengi olarak tespit edilen pikselleri kirmiziya boya (BGR: 0,0,255)
    cv::Mat result = bgr.clone();
    result.setTo(cv::Scalar(0, 0, 255), skinMask);

    // 4. Sonucu kaydet
    std::string out_path = "/content/people_skin_red.bmp";
    cv::imwrite(out_path, result);
    std::cout << "Sonuc kaydedildi: " << out_path << std::endl;

    // Karsilastirma icin maskeyi de kaydet
    cv::imwrite("/content/people_skin_mask.bmp", skinMask);
    std::cout << "Maske kaydedildi: /content/people_skin_mask.bmp" << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week9_task5_skin_detection.cpp -o week9_task5_skin_detection \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task5_skin_detection
