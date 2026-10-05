
// girdi resmi : saltpepper.png
%%bash
cat << 'EOF' > median_builtin.cpp
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    cv::Mat src = cv::imread("/content/saltpepper.png", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/saltpepper.png okunamadi! Dosyanin yuklendiginden emin olun." << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl;

    cv::Mat dst;
    auto t_start = std::chrono::high_resolution_clock::now();

    // OpenCV Hazır Medyan Filtresi (5x5)
    cv::medianBlur(src, dst, 5);

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[BILGI] Hazir cv::medianBlur suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/saltpepper_builtin_median.png", dst);
    std::cout << "[OK] Hazir filtre kaydedildi: /content/saltpepper_builtin_median.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 median_builtin.cpp -o median_builtin \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./median_builtin