
// OPENCV BUILT IN FONKİSYON ile histogram eşitleme işlemi
//[BILGI] Hazir equalizeHist suresi: 6020 mikrosaniye
//[OK] Hazir fonksiyon ciktisi kaydedildi -> /content/image2_builtin_eq.png

%%bash
cat << 'EOF' > step1_builtin.cpp
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    cv::Mat src = cv::imread("/content/image2.png", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image2.png okunamadi!" << std::endl;
        return 1;
    }

    cv::Mat dst;
    auto t_start = std::chrono::high_resolution_clock::now();

    // OpenCV yerleşik hazır fonksiyonu
    cv::equalizeHist(src, dst);

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[BILGI] Hazir equalizeHist suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/image2_builtin_eq.png", dst);
    std::cout << "[OK] Hazir fonksiyon ciktisi kaydedildi -> /content/image2_builtin_eq.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 step1_builtin.cpp -o step1_builtin \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./step1_builtin