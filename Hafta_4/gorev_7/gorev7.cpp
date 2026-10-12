%%bash
cat << 'EOF' > task7_canny_benchmark.cpp
#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using Clock = std::chrono::high_resolution_clock;

// 1. Prewitt Dikey Konvolüsyonu
cv::Mat runPrewitt(const cv::Mat& src) {
    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
    const float kernel[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
    for (int r = 1; r < src.rows - 1; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);
        for (int c = 1; c < src.cols - 1; ++c) {
            float sum = 0;
            for (int kr = -1; kr <= 1; ++kr) {
                const uchar* src_row = src.ptr<uchar>(r + kr);
                for (int kc = -1; kc <= 1; ++kc) {
                    sum += src_row[c + kc] * kernel[kr + 1][kc + 1];
                }
            }
            dst_row[c] = static_cast<uchar>(std::min(255.0f, std::abs(sum)));
        }
    }
    return dst;
}

// 2. Sobel Dikey Konvolüsyonu
cv::Mat runSobel(const cv::Mat& src) {
    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
    const float kernel[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    for (int r = 1; r < src.rows - 1; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);
        for (int c = 1; c < src.cols - 1; ++c) {
            float sum = 0;
            for (int kr = -1; kr <= 1; ++kr) {
                const uchar* src_row = src.ptr<uchar>(r + kr);
                for (int kc = -1; kc <= 1; ++kc) {
                    sum += src_row[c + kc] * kernel[kr + 1][kc + 1];
                }
            }
            dst_row[c] = static_cast<uchar>(std::min(255.0f, std::abs(sum)));
        }
    }
    return dst;
}

// 3. LoG (3x3 Gauss + 3x3 Laplacian)
cv::Mat runLoG(const cv::Mat& src) {
    cv::Mat blur, dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
    cv::GaussianBlur(src, blur, cv::Size(3, 3), 0);
    const float kernel[3][3] = {{1, 1, 1}, {1, -8, 1}, {1, 1, 1}};
    for (int r = 1; r < src.rows - 1; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);
        for (int c = 1; c < src.cols - 1; ++c) {
            float sum = 0;
            for (int kr = -1; kr <= 1; ++kr) {
                const uchar* src_row = blur.ptr<uchar>(r + kr);
                for (int kc = -1; kc <= 1; ++kc) {
                    sum += src_row[c + kc] * kernel[kr + 1][kc + 1];
                }
            }
            dst_row[c] = static_cast<uchar>(std::min(255.0f, std::abs(sum)));
        }
    }
    return dst;
}

// 4. 7x7 Mexican Hat (LoG)
cv::Mat runMexicanHat(const cv::Mat& src) {
    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
    const float kernel[7][7] = {
        { 0,  0, -1, -1, -1,  0,  0},
        { 0, -1, -3, -3, -3, -1,  0},
        {-1, -3,  0,  7,  0, -3, -1},
        {-1, -3,  7, 24,  7, -3, -1},
        {-1, -3,  0,  7,  0, -3, -1},
        { 0, -1, -3, -3, -3, -1,  0},
        { 0,  0, -1, -1, -1,  0,  0}
    };
    for (int r = 3; r < src.rows - 3; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);
        for (int c = 3; c < src.cols - 3; ++c) {
            float sum = 0;
            for (int kr = -3; kr <= 3; ++kr) {
                const uchar* src_row = src.ptr<uchar>(r + kr);
                for (int kc = -3; kc <= 3; ++kc) {
                    sum += src_row[c + kc] * kernel[kr + 3][kc + 3];
                }
            }
            dst_row[c] = static_cast<uchar>(std::min(255.0f, std::abs(sum)));
        }
    }
    return dst;
}

// 5. Canny Kenar Algılayıcı (OpenCV)
cv::Mat runCanny(const cv::Mat& src) {
    cv::Mat dst;
    // Düşük eşik = 50, Yüksek eşik = 150 (1:3 oranı)
    cv::Canny(src, dst, 50, 150);
    return dst;
}

int main() {
    cv::Mat src = cv::imread("/content/image3.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image3.jpg okunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl << std::endl;

    // Isınma (Warmup) ve Çalışma Zamanı Ölçümü
    const int N = 20; // 20 kez çalıştırıp ortalama alma
    auto measure = [&](const std::string& name, auto fn, cv::Mat& out) {
        out = fn(src); // İlk çalıştırma
        auto t0 = Clock::now();
        for (int i = 0; i < N; ++i) {
            out = fn(src);
        }
        auto t1 = Clock::now();
        double avg_ms = std::chrono::duration<double, std::milli>(t1 - t0).count() / N;
        std::cout << std::left << std::setw(25) << name << " : " 
                  << std::fixed << std::setprecision(3) << avg_ms << " ms" << std::endl;
    };

    cv::Mat res_p, res_s, res_log, res_mh, res_canny;

    std::cout << "--- BENCHMARK SONUÇLARI (Ortalama Süre) ---" << std::endl;
    measure("1. Prewitt (3x3 Manuel)", runPrewitt, res_p);
    measure("2. Sobel (3x3 Manuel)", runSobel, res_s);
    measure("3. LoG (3x3 Manuel)", runLoG, res_log);
    measure("4. Mexican Hat (7x7 Manuel)", runMexicanHat, res_mh);
    measure("5. Canny (OpenCV)", runCanny, res_canny);

    // Görselleri kaydet
    cv::imwrite("/content/image3_canny.png", res_canny);
    std::cout << "\n[OK] Canny çıktısı kaydedildi -> /content/image3_canny.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task7_canny_benchmark.cpp -o task7_canny_benchmark \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task7_canny_benchmark