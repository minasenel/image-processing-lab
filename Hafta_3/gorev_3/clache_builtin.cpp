/*
[BILGI] Hazir CLAHE Suresi: 15855 mikrosaniye
[OK] Hazir fonksiyon kaydedildi: /content/image2_clahe_builtin.png
*/


%%bash
cat << 'EOF' > clahe_builtin.cpp
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    cv::Mat src = cv::imread("/content/image2.png", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image2.png bulunamadi!" << std::endl;
        return 1;
    }

    cv::Mat dst;
    auto t_start = std::chrono::high_resolution_clock::now();

    // Standart parametreler: clipLimit = 2.0, tileGridSize = 8x8
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(src, dst);

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[BILGI] Hazir CLAHE Suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/image2_clahe_builtin.png", dst);
    std::cout << "[OK] Hazir fonksiyon kaydedildi: /content/image2_clahe_builtin.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 clahe_builtin.cpp -o clahe_builtin \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./clahe_builtin