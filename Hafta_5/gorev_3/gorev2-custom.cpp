%%bash
cat << 'EOF' > cell2_custom_gray.cpp
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// SIFIRDAN (CUSTOM) GRAYSCALE DÖNÜŞTÜRME
cv::Mat customConvertToGray(const cv::Mat& src) {
    int rows = src.rows;
    int cols = src.cols;
    cv::Mat dst(rows, cols, CV_8UC1);

    for (int r = 0; r < rows; ++r) {
        const cv::Vec3b* src_row = src.ptr<cv::Vec3b>(r);
        uchar* dst_row = dst.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c) {
            float b = src_row[c][0];
            float g = src_row[c][1];
            float r_val = src_row[c][2];

            // Ağırlıklı ortalama hesabı ve yuvarlama (round)
            float gray = 0.114f * b + 0.587f * g + 0.299f * r_val;
            dst_row[c] = static_cast<uchar>(std::round(gray));
        }
    }
    return dst;
}

int main() {
    // 1. Görseli Oku
    cv::Mat src = cv::imread("/content/baboon.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/baboon.jpg okunamadi!" << std::endl;
        return 1;
    }

    // 2. Özel (Custom) Fonksiyon ile Dönüştürme
    cv::Mat custom_gray = customConvertToGray(src);

    // 3. Sonucu Kaydet
    cv::imwrite("/content/baboon_custom_gray.png", custom_gray);
    std::cout << "[OK] 2. Hücre Tamamlandı: Custom Gray kaydedildi -> /content/baboon_custom_gray.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 cell2_custom_gray.cpp -o cell2_custom_gray \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./cell2_custom_gray