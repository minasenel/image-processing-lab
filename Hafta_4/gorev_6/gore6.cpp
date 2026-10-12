%%bash
cat << 'EOF' > task7_mexican_hat.cpp
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// 7x7 Mexican Hat Konvolüsyon Fonksiyonu
cv::Mat applyMexicanHat(const cv::Mat& src) {
    int rows = src.rows;
    int cols = src.cols;
    cv::Mat dst = cv::Mat::zeros(rows, cols, CV_8UC1);

    // 7x7 Mexican Hat (LoG) Çekirdek Matrisi
    const float kernel[7][7] = {
        { 0.0f,  0.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f},
        { 0.0f, -1.0f, -3.0f, -3.0f, -3.0f, -1.0f,  0.0f},
        {-1.0f, -3.0f,  0.0f,  7.0f,  0.0f, -3.0f, -1.0f},
        {-1.0f, -3.0f,  7.0f, 24.0f,  7.0f, -3.0f, -1.0f},
        {-1.0f, -3.0f,  0.0f,  7.0f,  0.0f, -3.0f, -1.0f},
        { 0.0f, -1.0f, -3.0f, -3.0f, -3.0f, -1.0f,  0.0f},
        { 0.0f,  0.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f}
    };

    // 7x7 yarıçapı r = 3 olduğundan sınır kontrolü 3. satır/sütundan başlar
    const int radius = 3;

    for (int r = radius; r < rows - radius; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);

        for (int c = radius; c < cols - radius; ++c) {
            float sum = 0.0f;

            // 7x7 Komşuluk Alanı Konvolüsyonu
            for (int kr = -radius; kr <= radius; ++kr) {
                const uchar* src_row = src.ptr<uchar>(r + kr);
                for (int kc = -radius; kc <= radius; ++kc) {
                    sum += src_row[c + kc] * kernel[kr + radius][kc + radius];
                }
            }

            // İkinci türev sonucu işaret değiştirdiği için mutlak değerini alıyoruz
            float abs_val = std::abs(sum);
            if (abs_val > 255.0f) abs_val = 255.0f;

            dst_row[c] = static_cast<uchar>(abs_val);
        }
    }
    return dst;
}

int main() {
    cv::Mat src = cv::imread("/content/image3.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image3.jpg okunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl;

    cv::Mat result = applyMexicanHat(src);

    cv::imwrite("/content/image3_mexican_hat.png", result);
    std::cout << "[OK] 7x7 Mexican Hat filtresi uygulandi -> /content/image3_mexican_hat.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task7_mexican_hat.cpp -o task7_mexican_hat \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task7_mexican_hat