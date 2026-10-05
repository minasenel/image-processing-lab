//girdi resmi : saltpepper.png
// custom
%%bash
cat << 'EOF' > median_custom.cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
    cv::Mat src = cv::imread("/content/saltpepper.png", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/saltpepper.png okunamadi!" << std::endl;
        return 1;
    }

    int rows = src.rows;
    int cols = src.cols;
    cv::Mat dst = src.clone(); // Kenarları orijinalden korumak için klonluyoruz

    auto t_start = std::chrono::high_resolution_clock::now();

    // 5x5 pencere için pencere yarıçapı = 2 (2 piksel kenar boşluğu)
    const int radius = 2;
    std::vector<uchar> window(25);

    for (int r = radius; r < rows - radius; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);

        for (int c = radius; c < cols - radius; ++c) {
            int idx = 0;

            // 5x5 komşuluk piksellerini topla (25 piksel)
            for (int wr = -radius; wr <= radius; ++wr) {
                const uchar* src_row = src.ptr<uchar>(r + wr);
                for (int wc = -radius; wc <= radius; ++wc) {
                    window[idx++] = src_row[c + wc];
                }
            }

            // 25 elemanın ortanca (12. indeks, 0-tabanlı) değerini bul
            // nth_element, tam sıralama yapmadan O(N) karmaşıklıkla ortancayı bulur
            std::nth_element(window.begin(), window.begin() + 12, window.end());
            
            dst_row[c] = window[12];
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[BILGI] Manuel 5x5 Medyan suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/saltpepper_custom_median.png", dst);
    std::cout << "[OK] Manuel filtre kaydedildi: /content/saltpepper_custom_median.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 median_custom.cpp -o median_custom \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./median_custom