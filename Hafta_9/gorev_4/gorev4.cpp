/*
Goruntu acildi: 500x446, 3 kanal
HSV kaydedildi: /content/parrot_HSV.bmp
HSI kaydedildi: /content/parrot_HSI.bmp
YUV kaydedildi: /content/parrot_YUV.bmp
CIE Lab kaydedildi: /content/parrot_Lab.bmp
Tum donusumler tamamlandi.
*/

%%bash
cat << 'EOF' > week9_task4_color_spaces.cpp
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// OpenCV'de hazir HSI donusumu yok, bu yuzden manuel hesapliyoruz.
// H: Hue (0-180 araligina olceklendi, 8-bit BMP'ye sigmasi icin)
// S: Saturation (0-255)
// I: Intensity = (R+G+B)/3 (0-255)
cv::Mat convertToHSI(const cv::Mat& bgr) {
    cv::Mat hsi(bgr.rows, bgr.cols, CV_8UC3);

    for (int y = 0; y < bgr.rows; y++) {
        for (int x = 0; x < bgr.cols; x++) {
            const cv::Vec3b& px = bgr.at<cv::Vec3b>(y, x);
            double B = px[0] / 255.0;
            double G = px[1] / 255.0;
            double R = px[2] / 255.0;

            double intensity = (R + G + B) / 3.0;

            double minRGB = std::min({R, G, B});
            double saturation = (intensity == 0.0) ? 0.0 : 1.0 - (minRGB / intensity);

            double hue = 0.0;
            double num = 0.5 * ((R - G) + (R - B));
            double den = std::sqrt((R - G) * (R - G) + (R - B) * (G - B));
            if (den != 0.0) {
                double theta = std::acos(std::max(-1.0, std::min(1.0, num / den)));
                hue = (B <= G) ? theta : (2 * CV_PI - theta);
            }
            hue = hue * 180.0 / CV_PI; // radyandan dereceye (0-360)

            cv::Vec3b out;
            out[0] = cv::saturate_cast<uchar>(hue / 2.0);        // H -> 0-180
            out[1] = cv::saturate_cast<uchar>(saturation * 255); // S -> 0-255
            out[2] = cv::saturate_cast<uchar>(intensity * 255);  // I -> 0-255
            hsi.at<cv::Vec3b>(y, x) = out;
        }
    }
    return hsi;
}

int main() {
    // 1. RGB renk uzayinda, BMP formatinda renkli goruntuyu ac
    std::string input_path = "/content/parrot.bmp";
    cv::Mat bgr = cv::imread(input_path, cv::IMREAD_COLOR);

    if (bgr.empty()) {
        std::cerr << "[HATA] " << input_path << " bulunamadi!" << std::endl;
        return -1;
    }
    std::cout << "Goruntu acildi: " << bgr.cols << "x" << bgr.rows
              << ", " << bgr.channels() << " kanal" << std::endl;

    // 2. HSV'ye donustur ve kaydet
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    cv::imwrite("/content/parrot_HSV.bmp", hsv);
    std::cout << "HSV kaydedildi: /content/parrot_HSV.bmp" << std::endl;

    // 3. HSI'ye donustur ve kaydet (OpenCV'de native yok, manuel hesaplandi)
    cv::Mat hsi = convertToHSI(bgr);
    cv::imwrite("/content/parrot_HSI.bmp", hsi);
    std::cout << "HSI kaydedildi: /content/parrot_HSI.bmp" << std::endl;

    // 4. YUV'ye donustur ve kaydet
    cv::Mat yuv;
    cv::cvtColor(bgr, yuv, cv::COLOR_BGR2YUV);
    cv::imwrite("/content/parrot_YUV.bmp", yuv);
    std::cout << "YUV kaydedildi: /content/parrot_YUV.bmp" << std::endl;

    // 5. CIE Lab'a donustur ve kaydet
    cv::Mat lab;
    cv::cvtColor(bgr, lab, cv::COLOR_BGR2Lab);
    cv::imwrite("/content/parrot_Lab.bmp", lab);
    std::cout << "CIE Lab kaydedildi: /content/parrot_Lab.bmp" << std::endl;

    std::cout << "Tum donusumler tamamlandi." << std::endl;
    return 0;
}
EOF

g++ -std=c++17 -O3 week9_task4_color_spaces.cpp -o week9_task4_color_spaces \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task4_color_spaces
