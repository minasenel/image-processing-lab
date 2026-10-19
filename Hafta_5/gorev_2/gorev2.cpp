//opencv'nin hazır calcHist fonksiyonunu kullanmak yerine, sıfırdan manuel olarak histogram hesaplaması yapacak bir C++ programı yazıyoruz. Bu program, baboon.jpg görselini okuyacak, her bir renk kanalı için histogramı hesaplayacak ve ardından bu histogramları bir tuval üzerine çizecek. Sonuç olarak, RGB histogramını gösteren bir görsel kaydedilecek.

%%bash
cat << 'EOF' > task1_custom_rgb_histogram.cpp
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp> // cv::line ve cv::Point için gerekli başlık dosyası

int main() {
    // 1. baboon.jpg Görselini Renkli (3 kanallı BGR) Olarak Oku
    cv::Mat src = cv::imread("/content/baboon.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/baboon.jpg bulunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows 
              << " | Kanal Sayisi: " << src.channels() << std::endl;

    // 2. Her bir kanal için 256 elemanlı frekans sayaç dizileri (0 ile başlatılır)
    std::vector<int> b_hist(256, 0);
    std::vector<int> g_hist(256, 0);
    std::vector<int> r_hist(256, 0);

    // 3. SIFIRDAN HISTOGRAM HESABI (Piksel piksel gezerek frekans sayma)
    for (int r = 0; r < src.rows; ++r) {
        const cv::Vec3b* row_ptr = src.ptr<cv::Vec3b>(r);
        for (int c = 0; c < src.cols; ++c) {
            uchar b_val = row_ptr[c][0]; // Mavi kanal
            uchar g_val = row_ptr[c][1]; // Yeşil kanal
            uchar r_val = row_ptr[c][2]; // Kırmızı kanal

            b_hist[b_val]++;
            g_hist[g_val]++;
            r_hist[r_val]++;
        }
    }

    // 4. Histogram Çizimi İçin Tuvale Sığdırma (Manuel Min-Max Normalizasyonu)
    int max_val = 0;
    for (int i = 0; i < 256; ++i) {
        if (b_hist[i] > max_val) max_val = b_hist[i];
        if (g_hist[i] > max_val) max_val = g_hist[i];
        if (r_hist[i] > max_val) max_val = r_hist[i];
    }

    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double)hist_w / 256.0);
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0)); // Siyah tuval

    // 5. Kanalları Tuval Üzerine Çizdirme
    for (int i = 1; i < 256; ++i) {
        // Frekans değerlerini tuval yüksekliğine (hist_h) oranlama
        int b_prev = hist_h - cvRound((double)b_hist[i - 1] / max_val * (hist_h - 20));
        int b_curr = hist_h - cvRound((double)b_hist[i] / max_val * (hist_h - 20));

        int g_prev = hist_h - cvRound((double)g_hist[i - 1] / max_val * (hist_h - 20));
        int g_curr = hist_h - cvRound((double)g_hist[i] / max_val * (hist_h - 20));

        int r_prev = hist_h - cvRound((double)r_hist[i - 1] / max_val * (hist_h - 20));
        int r_curr = hist_h - cvRound((double)r_hist[i] / max_val * (hist_h - 20));

        // Mavi (B)
        cv::line(histImage, cv::Point(bin_w * (i - 1), b_prev),
                            cv::Point(bin_w * i, b_curr),
                            cv::Scalar(255, 0, 0), 2);
        // Yeşil (G)
        cv::line(histImage, cv::Point(bin_w * (i - 1), g_prev),
                            cv::Point(bin_w * i, g_curr),
                            cv::Scalar(0, 255, 0), 2);
        // Kırmızı (R)
        cv::line(histImage, cv::Point(bin_w * (i - 1), r_prev),
                            cv::Point(bin_w * i, r_curr),
                            cv::Scalar(0, 0, 255), 2);
    }

    // 6. Çıktıyı Kaydet
    cv::imwrite("/content/baboon_custom_rgb_histogram.png", histImage);
    std::cout << "[OK] Sıfırdan hesaplanan RGB histogramı kaydedildi -> /content/baboon_custom_rgb_histogram.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır (imgproc kütüphanesi eklendi)
g++ -std=c++17 -O3 task1_custom_rgb_histogram.cpp -o task1_custom_rgb_histogram \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task1_custom_rgb_histogram