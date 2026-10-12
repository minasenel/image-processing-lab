%%bash
cat << 'EOF' > task2_direct_horizontal.cpp
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// Doğrudan Konvolüsyon Fonksiyonu
cv::Mat applyHorizontalPrewittDirect(const cv::Mat& src) {
    int rows = src.rows;
    int cols = src.cols;
    cv::Mat dst = cv::Mat::zeros(rows, cols, CV_8UC1);

    // Yatay Prewitt Kerneli:
    // [-1  -1  -1]
    // [ 0   0   0]
    // [ 1   1   1]
    const float kernel[3][3] = {
        {-1.0f, -1.0f, -1.0f},
        { 0.0f,  0.0f,  0.0f},
        { 1.0f,  1.0f,  1.0f}
    };

    // Taşmayı önlemek için (1, 1)'den başlanıp sınırda duruluyor
    for (int r = 1; r < rows - 1; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);

        const uchar* prev_row = src.ptr<uchar>(r - 1);
        const uchar* curr_row = src.ptr<uchar>(r);
        const uchar* next_row = src.ptr<uchar>(r + 1);

        for (int c = 1; c < cols - 1; ++c) {
            float sum = 0.0f;

            // 3x3 Komşuluk Çarpımı
            sum += prev_row[c - 1] * kernel[0][0] + prev_row[c] * kernel[0][1] + prev_row[c + 1] * kernel[0][2];
            sum += curr_row[c - 1] * kernel[1][0] + curr_row[c] * kernel[1][1] + curr_row[c + 1] * kernel[1][2];
            sum += next_row[c - 1] * kernel[2][0] + next_row[c] * kernel[2][1] + next_row[c + 1] * kernel[2][2];

            // Mutlak değer alınıp 0-255 aralığına sınırlandırılıyor
            float abs_val = std::abs(sum);
            if (abs_val > 255.0f) abs_val = 255.0f;

            dst_row[c] = static_cast<uchar>(abs_val);
        }
    }
    return dst;
}

int main() {
    // Görseli doğrudan tek kanallı (grayscale) olarak okuyoruz
    cv::Mat src = cv::imread("/content/image3.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image3.jpg bulunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl;

    cv::Mat horizontal_edges = applyHorizontalPrewittDirect(src);

    cv::imwrite("/content/image3_direct_horizontal.png", horizontal_edges);
    std::cout << "[OK] Doğrudan Yatay Prewitt tamamlandi -> /content/image3_direct_horizontal.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task2_direct_horizontal.cpp -o task2_direct_horizontal \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task2_direct_horizontal