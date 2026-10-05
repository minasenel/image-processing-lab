// Gorev 4: Manuel 2D Konvolüsyon Uygulaması
// noise.jpg görselini tek kanallı olarak okuyup hem 1/9 kutu (ortalama) filtresini hem de merkez ağırlıklı ($0.5$) filtreyi uygulayan C++ Colab hücresi:
// [BILGI] Girdi Boyutu: 514x514
// [OK] 1/9 Ortalama filtre sonucu: /content/noise_box_filtered.png
// [OK] Agirlikli filtre sonucu: /content/noise_weighted_filtered.png

%%bash
cat << 'EOF' > task4_run.cpp
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// Manuel 2D Konvolüsyon Fonksiyonu (Hazır fonksiyon yok)
cv::Mat applyConvolution(const cv::Mat& src, const float kernel[3][3]) {
    int rows = src.rows;
    int cols = src.cols;
    
    // Kenar piksellerini korumak için orijinalden klonluyoruz
    cv::Mat dst = src.clone();

    // 3x3 kernel için 1. satır ve sütundan başlayıp sınırda duruyoruz (taşma önleme)
    for (int r = 1; r < rows - 1; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);
        
        const uchar* prev_row = src.ptr<uchar>(r - 1);
        const uchar* curr_row = src.ptr<uchar>(r);
        const uchar* next_row = src.ptr<uchar>(r + 1);

        for (int c = 1; c < cols - 1; ++c) {
            float sum = 0.0f;

            // 3x3 Komşuluk Ağırlıklı Toplamı
            sum += prev_row[c - 1] * kernel[0][0];
            sum += prev_row[c]     * kernel[0][1];
            sum += prev_row[c + 1] * kernel[0][2];

            sum += curr_row[c - 1] * kernel[1][0];
            sum += curr_row[c]     * kernel[1][1];
            sum += curr_row[c + 1] * kernel[1][2];

            sum += next_row[c - 1] * kernel[2][0];
            sum += next_row[c]     * kernel[2][1];
            sum += next_row[c + 1] * kernel[2][2];

            // 0-255 Aralığına Kırpma (Clamping)
            if (sum < 0.0f) sum = 0.0f;
            if (sum > 255.0f) sum = 255.0f;

            dst_row[c] = static_cast<uchar>(sum + 0.5f);
        }
    }
    return dst;
}

int main() {
    // 1. /content/noise.jpg dosyasını tek kanallı (grayscale) oku
    cv::Mat src = cv::imread("/content/noise.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/noise.jpg okunamadi! Dosyanin yuklendiginden emin olun." << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl;

    // 2. Filtre 1: 3x3 Ortalama Filtresi (1/9 Box Filter)
    float box_kernel[3][3] = {
        {1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
        {1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f},
        {1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f}
    };
    cv::Mat box_filtered = applyConvolution(src, box_kernel);
    cv::imwrite("/content/noise_box_filtered.png", box_filtered);
    std::cout << "[OK] 1/9 Ortalama filtre sonucu: /content/noise_box_filtered.png" << std::endl;

    // 3. Filtre 2: Kendi Değerini Koruyan Ağırlıklı Kernel (Toplam = 1.0)
    // Merkez: 0.5, Eksenler: 0.1, Köşeler: 0.025
    float weighted_kernel[3][3] = {
        {0.025f, 0.10f, 0.025f},
        {0.10f,  0.50f, 0.10f},
        {0.025f, 0.10f, 0.025f}
    };
    cv::Mat weighted_filtered = applyConvolution(src, weighted_kernel);
    cv::imwrite("/content/noise_weighted_filtered.png", weighted_filtered);
    std::cout << "[OK] Agirlikli filtre sonucu: /content/noise_weighted_filtered.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task4_run.cpp -o task4_run \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task4_run