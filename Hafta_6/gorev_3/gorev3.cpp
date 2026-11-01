/*
==================================================
Tespit Edilen Çember Sayısı: 0
Sonuç paneli kaydedildi   -> /content/hough_circles_results.png
*/
%%bash
cat << 'EOF' > task_hough_circles.cpp
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // 1. Görseli Oku (circles.png)
    cv::Mat src = cv::imread("/content/circles.png", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/circles.png bulunamadi! Lutfen resmi yukleyin." << std::endl;
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
    cv::medianBlur(edges, filtered_edges, 5); // 5x5 çekirdek boyutu

    // 5. Adım 3: OpenCV Hough Circles Tespiti
    // cv::HoughCircles doğrudan gri/yumuşatılmış görselle daha kararlı çalıştığı için
    // medianBlur uygulanmış kenar haritasını girdi olarak veriyoruz.
    std::vector<cv::Vec3f> circles;
    
    // Parametreler: (Image, Output, Method, dp, minDist, param1, param2, minRadius, maxRadius)
    // dp=1: Çözünürlük oranı
    // minDist=30: İki çember merkezi arasındaki minimum mesafe (çakışmayı önler)
    // param1=100: Canny yüksek eşik değeri
    // param2=30: Akümülatör eşik değeri (düşerse daha çok çember bulur, yükselirse daha seçici olur)
    // minRadius=10, maxRadius=200: Aranacak çemberlerin piksel cinsinden yarıçap aralığı
    cv::HoughCircles(filtered_edges, circles, cv::HOUGH_GRADIENT, 1, 30, 100, 30, 10, 200);

    // 6. Çemberleri Resim Üzerine Çizme
    cv::Mat result_img = src.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);
        int radius = c[2];

        // Çember Merkezini Çiz (Kırmızı Nokta)
        cv::circle(result_img, center, 3, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
        // Çember Dış Hattını Çiz (Yeşil Çizgi)
        cv::circle(result_img, center, radius, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }

    // 7. Çıktıları Birleştirme (3'lü Panel: Canny Kenarları | Median Filtre | Hough Circles)
    int w = src.cols, h = src.rows;
    cv::Mat panel(h, w * 3, CV_8UC3);

    cv::Mat edges_bgr, filtered_bgr;
    cv::cvtColor(edges, edges_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(filtered_edges, filtered_bgr, cv::COLOR_GRAY2BGR);

    edges_bgr.copyTo(panel(cv::Rect(0, 0, w, h)));
    filtered_bgr.copyTo(panel(cv::Rect(w, 0, w, h)));
    result_img.copyTo(panel(cv::Rect(w * 2, 0, w, h)));

    // Etiketler
    cv::putText(panel, "1. Canny Edges", cv::Point(20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "2. Median Filtered", cv::Point(w + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "3. Hough Circles Result", cv::Point(w * 2 + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

    // Kaydet
    cv::imwrite("/content/hough_circles_results.png", panel);

    std::cout << "==================================================" << std::endl;
    std::cout << "Tespit Edilen Çember Sayısı: " << circles.size() << std::endl;
    std::cout << "Sonuç paneli kaydedildi   -> /content/hough_circles_results.png" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_hough_circles.cpp -o task_hough_circles \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_hough_circles