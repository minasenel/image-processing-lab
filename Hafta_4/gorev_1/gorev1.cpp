%%bash
cat << 'EOF' > task1_gaussian_prewitt.cpp
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// Genel Konvolüsyon Fonksiyonu
cv::Mat applyConvolution(const cv::Mat& src, const float kernel[3][3], bool take_abs = false) {
    int rows = src.rows;
    int cols = src.cols;
    cv::Mat dst = cv::Mat::zeros(rows, cols, CV_8UC1);

    for (int r = 1; r < rows - 1; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);

        const uchar* prev_row = src.ptr<uchar>(r - 1);
        const uchar* curr_row = src.ptr<uchar>(r);
        const uchar* next_row = src.ptr<uchar>(r + 1);

        for (int c = 1; c < cols - 1; ++c) {
            float sum = 0.0f;

            sum += prev_row[c - 1] * kernel[0][0] + prev_row[c] * kernel[0][1] + prev_row[c + 1] * kernel[0][2];
            sum += curr_row[c - 1] * kernel[1][0] + curr_row[c] * kernel[1][1] + curr_row[c + 1] * kernel[1][2];
            sum += next_row[c - 1] * kernel[2][0] + next_row[c] * kernel[2][1] + next_row[c + 1] * kernel[2][2];

            if (take_abs) {
                sum = std::abs(sum);
            }

            if (sum < 0.0f) sum = 0.0f;
            if (sum > 255.0f) sum = 255.0f;

            dst_row[c] = static_cast<uchar>(sum + 0.5f);
        }
    }
    return dst;
}

int main() {
    cv::Mat src = cv::imread("/content/image3.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image3.jpg bulunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl;

    // 1. ADIM: 3x3 Gauss Yumuşatma Kerneli (Toplam = 1.0)
    const float gaussian_kernel[3][3] = {
        {1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f},
        {2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f},
        {1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f}
    };
    cv::Mat gaussian_img = applyConvolution(src, gaussian_kernel, false);
    cv::imwrite("/content/image3_gaussian.png", gaussian_img);
    std::cout << "[OK] 1. Adim: Gauss yumusatma uygulandi -> /content/image3_gaussian.png" << std::endl;

    // 2. ADIM: Dikey Prewitt Kenar Algılama Kerneli
    const float prewitt_v_kernel[3][3] = {
        {-1.0f, 0.0f, 1.0f},
        {-1.0f, 0.0f, 1.0f},
        {-1.0f, 0.0f, 1.0f}
    };
    
    // Doğrudan Orijinal Görsele Prewitt (Karşılaştırma için)
    cv::Mat direct_prewitt = applyConvolution(src, prewitt_v_kernel, true);
    cv::imwrite("/content/image3_direct_prewitt.png", direct_prewitt);

    // Gauss Uygulanmış Görsele Prewitt
    cv::Mat gauss_prewitt = applyConvolution(gaussian_img, prewitt_v_kernel, true);
    cv::imwrite("/content/image3_gauss_prewitt.png", gauss_prewitt);
    std::cout << "[OK] 2. Adim: Gauss + Prewitt tamamlandi -> /content/image3_gauss_prewitt.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task1_gaussian_prewitt.cpp -o task1_gaussian_prewitt \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task1_gaussian_prewitt