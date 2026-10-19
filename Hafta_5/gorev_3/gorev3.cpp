// rgb-> iki kanallı dönüşümü. ilk önce opencv nin hazır cvtColor fonksiyonu ile yapılacak: 
// SONUÇ: baboon_opencv_gray.png 
%%bash
cat << 'EOF' > cell1_opencv_gray.cpp
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // 1. Görseli Oku
    cv::Mat src = cv::imread("/content/baboon.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/baboon.jpg okunamadi!" << std::endl;
        return 1;
    }

    // 2. OpenCV Hazır Fonksiyonu ile Gri Seviyeye Dönüştürme
    cv::Mat opencv_gray;
    cv::cvtColor(src, opencv_gray, cv::COLOR_BGR2GRAY);

    // 3. Sonucu Kaydet
    cv::imwrite("/content/baboon_opencv_gray.png", opencv_gray);
    std::cout << "[OK] 1. Hücre Tamamlandı: OpenCV Gray kaydedildi -> /content/baboon_opencv_gray.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 cell1_opencv_gray.cpp -o cell1_opencv_gray \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./cell1_opencv_gray