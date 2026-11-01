/*==================================================
Standard HoughLines (cv::HoughLines)   : 7 dogru bulundu.
Probabilistic Hough (cv::HoughLinesP)  : 9 dogru parcasi bulundu.
Sonuc paneli kaydedildi -> /content/sudoku_hough_results.png
==================================================*/

%%bash
cat << 'EOF' > task_hough_sudoku.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // 1. Görseli Oku (sudoku.jpg)
    cv::Mat src = cv::imread("/content/sudoku.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/sudoku.jpg bulunamadi!" << std::endl;
        return 1;
    }

    // 2. Grayscale Dönüşümü
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    // 3. Adım 1: Canny ile Kenar Tespiti
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    // 4. Adım 2: Kenar Resmi Üzerine Median Filtre Uygulanması
    cv::Mat filtered_edges;
    cv::medianBlur(edges, filtered_edges, 3); // 3x3 çekirdek boyutu

    // Çizimler için görsel kopyaları
    cv::Mat img_standard = src.clone();
    cv::Mat img_probabilistic = src.clone();

    // 5. FONKSİYON 1: Standard Hough Transform (cv::HoughLines)
    std::vector<cv::Vec2f> lines_standard;
    // Sudoku çizgilerini rahat yakalamak için threshold = 100
    cv::HoughLines(filtered_edges, lines_standard, 1, CV_PI / 180, 100);

    // Standard Hough doğrularını ÇİZME (MAVİ Renk - Scalar(255, 0, 0))
    for (size_t i = 0; i < lines_standard.size(); i++) {
        float rho = lines_standard[i][0];
        float theta = lines_standard[i][1];

        double a = std::cos(theta);
        double b = std::sin(theta);
        double x0 = a * rho;
        double y0 = b * rho;

        cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));

        cv::line(img_standard, pt1, pt2, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
    }

    // 6. FONKSİYON 2: Probabilistic Hough Transform (cv::HoughLinesP)
    std::vector<cv::Vec4i> lines_probabilistic;
    // Sudoku hücre çizgileri için: threshold=50, minLineLength=30, maxLineGap=10
    cv::HoughLinesP(filtered_edges, lines_probabilistic, 1, CV_PI / 180, 50, 30, 10);

    // Olasılıksal Hough doğru parçalarını ÇİZME (KIRMIZI Renk - Scalar(0, 0, 255))
    for (size_t i = 0; i < lines_probabilistic.size(); i++) {
        cv::Vec4i l = lines_probabilistic[i];
        cv::line(img_probabilistic, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), 
                 cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // 7. Çıktıları Yan Yana Birleştirme (3'lü Panel)
    int w = src.cols, h = src.rows;
    cv::Mat panel(h, w * 3, CV_8UC3);

    cv::Mat edges_bgr;
    cv::cvtColor(filtered_edges, edges_bgr, cv::COLOR_GRAY2BGR);

    edges_bgr.copyTo(panel(cv::Rect(0, 0, w, h)));
    img_standard.copyTo(panel(cv::Rect(w, 0, w, h)));
    img_probabilistic.copyTo(panel(cv::Rect(w * 2, 0, w, h)));

    // Etiketler
    cv::putText(panel, "1. Canny + Median Filter", cv::Point(20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "2. Standard HoughLines (Mavi)", cv::Point(w + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "3. Probabilistic HoughLinesP (Kirmizi)", cv::Point(w * 2 + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    // Kaydet
    cv::imwrite("/content/sudoku_hough_results.png", panel);

    std::cout << "==================================================" << std::endl;
    std::cout << "Standard HoughLines (cv::HoughLines)   : " << lines_standard.size() << " dogru bulundu." << std::endl;
    std::cout << "Probabilistic Hough (cv::HoughLinesP)  : " << lines_probabilistic.size() << " dogru parcasi bulundu." << std::endl;
    std::cout << "Sonuc paneli kaydedildi -> /content/sudoku_hough_results.png" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_hough_sudoku.cpp -o task_hough_sudoku \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_hough_sudoku