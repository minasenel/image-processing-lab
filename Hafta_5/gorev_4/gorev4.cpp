/*
==================================================
        BGR -> HSV SAYISAL HATA RAPORU            
==================================================
Toplam Mutlak Fark (SAD) : 154563
Ortalama Mutlak Hata (MAE): 0.214671
Maksimum Kanal Farkı      : 180 piksel
==================================================

[OK] Birleştirilmiş görsel oluşturuldu -> /content/baboon_hsv_comparison.png
*/
%%bash
cat << 'EOF' > task_rgb_to_hsv_visual.cpp
#include <iostream>
#include <cmath>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// 1. Fonksiyon Göstericisi (Function Pointer) Tanımı
using ColorConverterFunc = void(*)(const cv::Mat&, cv::Mat&);

// SIFIRDAN BGR -> HSV DÖNÜŞTÜRME FONKSİYONU
void customBGR2HSV(const cv::Mat& src, cv::Mat& dst) {
    int rows = src.rows;
    int cols = src.cols;
    dst = cv::Mat(rows, cols, CV_8UC3);

    for (int r = 0; r < rows; ++r) {
        const cv::Vec3b* src_row = src.ptr<cv::Vec3b>(r);
        cv::Vec3b* dst_row = dst.ptr<cv::Vec3b>(r);

        for (int c = 0; c < cols; ++c) {
            float b = src_row[c][0] / 255.0f;
            float g = src_row[c][1] / 255.0f;
            float r_val = src_row[c][2] / 255.0f;

            float max_v = std::max({b, g, r_val});
            float min_v = std::min({b, g, r_val});
            float delta = max_v - min_v;

            float v = max_v * 255.0f;
            float s = (max_v > 0.0f) ? (delta / max_v) * 255.0f : 0.0f;

            float h = 0.0f;
            if (delta > 0.0f) {
                if (max_v == r_val) {
                    h = 60.0f * (g - b) / delta;
                } else if (max_v == g) {
                    h = 60.0f * ((b - r_val) / delta + 2.0f);
                } else {
                    h = 60.0f * ((r_val - g) / delta + 4.0f);
                }
                if (h < 0.0f) h += 360.0f;
            }

            uchar h_final = static_cast<uchar>(std::round(h / 2.0f));
            uchar s_final = static_cast<uchar>(std::round(s));
            uchar v_final = static_cast<uchar>(std::round(v));

            dst_row[c] = cv::Vec3b(h_final, s_final, v_final);
        }
    }
}

int main() {
    cv::Mat src = cv::imread("/content/baboon.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/baboon.jpg okunamadi!" << std::endl;
        return 1;
    }

    // 2. Fonksiyon Göstericisi (Function Pointer) ile Dönüştürme
    ColorConverterFunc converter = customBGR2HSV;
    cv::Mat custom_hsv;
    converter(src, custom_hsv);

    // 3. OpenCV Hazır Fonksiyonu
    cv::Mat opencv_hsv;
    cv::cvtColor(src, opencv_hsv, cv::COLOR_BGR2HSV);

    // 4. Sayısal Karşılaştırma ve Hata Raporu
    cv::Mat diff_map;
    cv::absdiff(opencv_hsv, custom_hsv, diff_map);

    unsigned long long total_sad = 0;
    int total_elements = src.rows * src.cols * 3;
    int max_diff = 0;

    for (int r = 0; r < diff_map.rows; ++r) {
        const cv::Vec3b* row = diff_map.ptr<cv::Vec3b>(r);
        for (int c = 0; c < diff_map.cols; ++c) {
            for (int ch = 0; ch < 3; ++ch) {
                int err = row[c][ch];
                total_sad += err;
                if (err > max_diff) max_diff = err;
            }
        }
    }

    double mae = static_cast<double>(total_sad) / total_elements;

    std::cout << "==================================================" << std::endl;
    std::cout << "        BGR -> HSV SAYISAL HATA RAPORU            " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Toplam Mutlak Fark (SAD) : " << total_sad << std::endl;
    std::cout << "Ortalama Mutlak Hata (MAE): " << mae << std::endl;
    std::cout << "Maksimum Kanal Farkı      : " << max_diff << " piksel" << std::endl;
    std::cout << "==================================================" << std::endl;

    // 5. TEK RESİMDE YAN YANA GÖRSEL BİRLEŞTİRME (2x2 Panel)
    int w = src.cols;
    int h = src.rows;
    cv::Mat grid(h * 2, w * 2, CV_8UC3, cv::Scalar(0, 0, 0));

    // Sol Üst: Orijinal BGR
    src.copyTo(grid(cv::Rect(0, 0, w, h)));
    cv::putText(grid, "1. Orijinal BGR", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

    // Sağ Üst: OpenCV HSV
    opencv_hsv.copyTo(grid(cv::Rect(w, 0, w, h)));
    cv::putText(grid, "2. OpenCV HSV", cv::Point(w + 20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

    // Sol Alt: Custom HSV
    custom_hsv.copyTo(grid(cv::Rect(0, h, w, h)));
    cv::putText(grid, "3. Custom HSV", cv::Point(20, h + 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

    // Sağ Alt: Fark Haritası (Mutlak Fark)
    diff_map.copyTo(grid(cv::Rect(w, h, w, h)));
    cv::putText(grid, "4. Fark (OpenCV - Custom)", cv::Point(w + 20, h + 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    cv::imwrite("/content/baboon_hsv_comparison.png", grid);
    std::cout << "\n[OK] Birleştirilmiş görsel oluşturuldu -> /content/baboon_hsv_comparison.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_rgb_to_hsv_visual.cpp -o task_rgb_to_hsv_visual \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_rgb_to_hsv_visual