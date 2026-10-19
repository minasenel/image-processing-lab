%%bash
cat << 'EOF' > task1_rgb_histogram.cpp
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // 1. baboon.jpg Görselini Renkli (3 kanallı BGR) Olarak Oku
    cv::Mat src = cv::imread("/content/baboon.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/baboon.jpg bulunamadi! Lutfen gorselin /content/ dizininde oldugundan emin olun." << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows 
              << " | Kanal Sayisi: " << src.channels() << std::endl;

    // 2. Kanallari Ayir (OpenCV siralamasi: BGR)
    std::vector<cv::Mat> bgr_planes;
    cv::split(src, bgr_planes);

    // 3. Histogram Parametreleri
    int histSize = 256; // 0-255 piksel degerleri
    float range[] = {0, 256};
    const float* histRange = {range};
    bool uniform = true, accumulate = false;

    cv::Mat b_hist, g_hist, r_hist;

    // 4. Her bir kanal icin histogram hesabi
    cv::calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    cv::calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    cv::calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

    // 5. Histogram Çizimi İcin Canvas
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double)hist_w / histSize);
    cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0)); // Siyah arka plan

    // Histogram Degerlerini Normalize Etme
    cv::normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
    cv::normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
    cv::normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

    // 6. Mavi, Yesil, Kirmizi Cizgileri Cizdirme
    for (int i = 1; i < histSize; i++) {
        // Mavi Kanal (B)
        cv::line(histImage, 
                 cv::Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
                 cv::Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))),
                 cv::Scalar(255, 0, 0), 2, 8, 0);

        // Yesil Kanal (G)
        cv::line(histImage, 
                 cv::Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
                 cv::Point(bin_w * (i), hist_h - cvRound(g_hist.at<float>(i))),
                 cv::Scalar(0, 255, 0), 2, 8, 0);

        // Kirmizi Kanal (R)
        cv::line(histImage, 
                 cv::Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
                 cv::Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
                 cv::Scalar(0, 0, 255), 2, 8, 0);
    }

    // 7. Çıktıyı Kaydet
    cv::imwrite("/content/baboon_rgb_histogram.png", histImage);
    std::cout << "[OK] Baboon RGB histogrami çizildi -> /content/baboon_rgb_histogram.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task1_rgb_histogram.cpp -o task1_rgb_histogram \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task1_rgb_histogram