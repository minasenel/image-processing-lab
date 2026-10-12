%%bash
cat << 'EOF' > task3_gauss_sobel.cpp
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// 2D Konvolüsyon Fonksiyonu
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
    // 1. Görseli tek kanallı (grayscale) olarak oku
    cv::Mat src = cv::imread("/content/image3.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image3.jpg okunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl;

    // 2. ADIM: Gauss Yumuşatma Filtresi (3x3 Kernel, Toplam = 1.0)
    const float gaussian_kernel[3][3] = {
        {1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f},
        {2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f},
        {1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f}
    };
    cv::Mat gaussian_img = applyConvolution(src, gaussian_kernel, false);
    cv::imwrite("/content/image3_gaussian_sobel.png", gaussian_img);
    std::cout << "[OK] 1. Adim: Gauss yumusatma tamamlandi -> /content/image3_gaussian_sobel.png" << std::endl;

    // 3. ADIM: Dikey Sobel Kerneli (Gx) - Yumuşatılmış Görüntü Üzerinde
    // [-1  0  1]
    // [-2  0  2]
    // [-1  0  1]
    const float sobel_v_kernel[3][3] = {
        {-1.0f, 0.0f, 1.0f},
        {-2.0f, 0.0f, 2.0f},
        {-1.0f, 0.0f, 1.0f}
    };
    cv::Mat sobel_v = applyConvolution(gaussian_img, sobel_v_kernel, true);
    cv::imwrite("/content/image3_sobel_v_gauss.png", sobel_v);
    std::cout << "[OK] 2. Adim: Dikey Sobel (Gx) tamamlandi -> /content/image3_sobel_v_gauss.png" << std::endl;

    // 4. ADIM: Yatay Sobel Kerneli (Gy) - Yumuşatılmış Görüntü Üzerinde
    // [-1 -2 -1]
    // [ 0  0  0]
    // [ 1  2  1]
    const float sobel_h_kernel[3][3] = {
        {-1.0f, -2.0f, -1.0f},
        { 0.0f,  0.0f,  0.0f},
        { 1.0f,  2.0f,  1.0f}
    };
    cv::Mat sobel_h = applyConvolution(gaussian_img, sobel_h_kernel, true);
    cv::imwrite("/content/image3_sobel_h_gauss.png", sobel_h);
    std::cout << "[OK] 3. Adim: Yatay Sobel (Gy) tamamlandi -> /content/image3_sobel_h_gauss.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task3_gauss_sobel.cpp -o task3_gauss_sobel \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task3_gauss_sobel