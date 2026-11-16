/*
Goruntu acildi: 2364x1576
H, S, V kanallari tek kanalli BMP olarak kaydedildi.
Sonuc kaydedildi: /content/people_skin_red_hsv.bmp
*/

%%bash
cat << 'EOF' > week9_task5_skin_detection_hsv.cpp
#include <iostream>
#include <vector>
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

    // 2. HSV'ye donustur
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    // 3. HSV'yi 3 kanala ayir: her kanal artik TEK KANALLI (gri tonlu) bir resim gibi
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    cv::Mat H = channels[0]; // Hue    : 0-180  (OpenCV olceginde)
    cv::Mat S = channels[1]; // Saturation : 0-255
    cv::Mat V = channels[2]; // Value  : 0-255

    // Kanallari tek tek gri tonlu BMP olarak kaydet (gorsellestirme/kontrol icin)
    cv::imwrite("/content/people_H.bmp", H);
    cv::imwrite("/content/people_S.bmp", S);
    cv::imwrite("/content/people_V.bmp", V);
    std::cout << "H, S, V kanallari tek kanalli BMP olarak kaydedildi." << std::endl;

    // 4. Ten rengi tespiti icin HER KANALI TEK TEK esikle (cv::inRange degil, tek kanal threshold)
    // Ten tonu OpenCV'nin HSV olceginde (H: 0-180) genellikle 0-25 araliginda yogunlasir.
    cv::Mat maskH, maskS, maskV;
    cv::threshold(H, maskH, 25, 255, cv::THRESH_BINARY_INV); // H <= 25 -> beyaz
    cv::threshold(S, maskS, 40,  255, cv::THRESH_BINARY);     // S >= 40 -> beyaz (cok soluk/beyaz alanlari ele)
    cv::threshold(V, maskV, 50,  255, cv::THRESH_BINARY);     // V >= 50 -> beyaz (cok karanlik alanlari ele)

    // 5. Tek kanal maskeleri birlestir (AND)
    cv::Mat skinMask;
    cv::bitwise_and(maskH, maskS, skinMask);
    cv::bitwise_and(skinMask, maskV, skinMask);

    // Gurultu/delikleri temizle
    cv::morphologyEx(skinMask, skinMask, cv::MORPH_OPEN,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::morphologyEx(skinMask, skinMask, cv::MORPH_CLOSE,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

    cv::imwrite("/content/people_skin_mask_hsv.bmp", skinMask);

    // 6. Ten rengi tespit edilen pikselleri kirmiziya boya (BGR: 0,0,255)
    cv::Mat result = bgr.clone();
    result.setTo(cv::Scalar(0, 0, 255), skinMask);

    std::string out_path = "/content/people_skin_red_hsv.bmp";
    cv::imwrite(out_path, result);
    std::cout << "Sonuc kaydedildi: " << out_path << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week9_task5_skin_detection_hsv.cpp -o week9_task5_skin_detection_hsv \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task5_skin_detection_hsv
