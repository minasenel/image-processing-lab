/*

==================================================
         PİKSEL KARŞILAŞTIRMA VE HATA RAPORU      
==================================================
Görüntü Boyutu              : 500 x 480
Toplam Piksel Sayısı        : 240000
Mutlak Farkların Toplamı (SAD): 0
Ortalama Mutlak Hata (MAE)  : 0 piksel/piksel
Maksimum Piksel Farkı       : 0 piksel
==================================================

*/

%%bash
cat << 'EOF' > cell3_calculate_error.cpp
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
    // 1. İki Resim Çıktısını Oku
    cv::Mat img1 = cv::imread("/content/baboon_opencv_gray.png", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("/content/baboon_custom_gray.png", cv::IMREAD_GRAYSCALE);

    if (img1.empty() || img2.empty()) {
        std::cerr << "[HATA] Karşılaştırılacak resimlerden biri bulunamadı!" << std::endl;
        return 1;
    }

    if (img1.rows != img2.rows || img1.cols != img2.cols) {
        std::cerr << "[HATA] Resim boyutları eşleşmiyor!" << std::endl;
        return 1;
    }

    // 2. Mutlak Farkların Toplamı (Sum of Absolute Differences - SAD) Hesabı
    unsigned long long total_absolute_difference = 0;
    int total_pixels = img1.rows * img1.cols;
    int max_diff = 0;

    for (int r = 0; r < img1.rows; ++r) {
        const uchar* row1 = img1.ptr<uchar>(r);
        const uchar* row2 = img2.ptr<uchar>(r);

        for (int c = 0; c < img1.cols; ++c) {
            int diff = std::abs(static_cast<int>(row1[c]) - static_cast<int>(row2[c]));
            total_absolute_difference += diff;

            if (diff > max_diff) {
                max_diff = diff;
            }
        }
    }

    double mean_absolute_error = static_cast<double>(total_absolute_difference) / total_pixels;

    // 3. Raporlama
    std::cout << "==================================================" << std::endl;
    std::cout << "         PİKSEL KARŞILAŞTIRMA VE HATA RAPORU      " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Görüntü Boyutu              : " << img1.cols << " x " << img1.rows << std::endl;
    std::cout << "Toplam Piksel Sayısı        : " << total_pixels << std::endl;
    std::cout << "Mutlak Farkların Toplamı (SAD): " << total_absolute_difference << std::endl;
    std::cout << "Ortalama Mutlak Hata (MAE)  : " << mean_absolute_error << " piksel/piksel" << std::endl;
    std::cout << "Maksimum Piksel Farkı       : " << max_diff << " piksel" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 cell3_calculate_error.cpp -o cell3_calculate_error \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./cell3_calculate_error