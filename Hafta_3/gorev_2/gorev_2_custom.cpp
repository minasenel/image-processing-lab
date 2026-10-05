/*
//GÖREV 2: KENDİ YAZDIĞIMIZ FONKİSYON İLE

./step2_custom
[BILGI] Manuel algoritma suresi: 2085 mikrosaniye
[OK] Manuel fonksiyon ciktisi kaydedildi -> /content/image2_custom_eq.png
 */


%%bash
cat << 'EOF' > step2_custom.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
    cv::Mat src = cv::imread("/content/image2.png", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image2.png okunamadi!" << std::endl;
        return 1;
    }

    int rows = src.rows;
    int cols = src.cols;
    int total_pixels = rows * cols;

    auto t_start = std::chrono::high_resolution_clock::now();

    // 1. Histogram Çıkartma (Hazır fonksiyon yok, doğrudan piksel frekansı sayımı)
    std::vector<int> hist(256, 0);
    for (int r = 0; r < rows; ++r) {
        const uchar* row = src.ptr<uchar>(r);
        for (int c = 0; c < cols; ++c) {
            hist[row[c]]++;
        }
    }

    // 2. Kümülatif Dağılım Fonksiyonu (CDF)
    std::vector<int> cdf(256, 0);
    cdf[0] = hist[0];
    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // Sıfır olmayan ilk kümülatif değeri (cdf_min) bul
    int cdf_min = 0;
    for (int i = 0; i < 256; ++i) {
        if (cdf[i] > 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    // 3. Eşitleme / Dönüşüm Tablosu (LUT)
    // Formül: round(((cdf[v] - cdf_min) / (total_pixels - cdf_min)) * 255)
    std::vector<uchar> lut(256, 0);
    int denominator = total_pixels - cdf_min;

    for (int i = 0; i < 256; ++i) {
        if (cdf[i] < cdf_min || denominator <= 0) {
            lut[i] = 0;
        } else {
            float val = (static_cast<float>(cdf[i] - cdf_min) / denominator) * 255.0f;
            lut[i] = static_cast<uchar>(std::round(val));
        }
    }

    // 4. Çıktı Matrisini Oluştur ve Pikselleri Güncelle
    cv::Mat dst(rows, cols, CV_8UC1);
    for (int r = 0; r < rows; ++r) {
        const uchar* src_row = src.ptr<uchar>(r);
        uchar* dst_row = dst.ptr<uchar>(r);
        for (int c = 0; c < cols; ++c) {
            dst_row[c] = lut[src_row[c]];
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[BILGI] Manuel algoritma suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/image2_custom_eq.png", dst);
    std::cout << "[OK] Manuel fonksiyon ciktisi kaydedildi -> /content/image2_custom_eq.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 step2_custom.cpp -o step2_custom \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./step2_custom