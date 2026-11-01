/*
==================================================
     CUSTOM 3D HOUGH CIRCLE TRANSFORM RAPORU      
==================================================
3D Akümülatör Boyutu     : 1850 x 1858 x 66
Test Edilen Yarıçaplar   : [15 - 80] piksel
Seçilen Eşik (Threshold) : 80 oy
Tespit Edilen Çember    : 0 adet
--------------------------------------------------
[OK] Sonuc kaydedildi -> /content/custom_hough_circles_results.png
==================================================
*/

%%bash
cat << 'EOF' > task_custom_hough_circles.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Bulunan çember yapısı
struct Circle {
    int a;      // Merkez X
    int b;      // Merkez Y
    int r;      // Yarıçap
    int votes;  // Aldığı oy sayısı
};

int main() {
    // 1. Görseli Oku (circles.png)
    cv::Mat src = cv::imread("/content/circles.png", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/circles.png okunamadi!" << std::endl;
        return 1;
    }

    // 2. Grayscale ve Canny Kenar Tespiti
    cv::Mat gray, blurred, edges;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::Canny(blurred, edges, 50, 150);

    int width = edges.cols;
    int height = edges.rows;

    // Aranacak Yarıçap Aralığı (Örnek: 15 ile 80 piksel arası çemberler)
    int min_r = 15;
    int max_r = 80;
    int num_radii = max_r - min_r + 1;
    int num_thetas = 360;

    // 3. TRIGONOMETRİK LOOK-UP TABLE (LUT) OLUŞTURMA
    std::vector<double> cos_lut(num_thetas);
    std::vector<double> sin_lut(num_thetas);
    for (int t = 0; t < num_thetas; ++t) {
        double rad = t * M_PI / 180.0;
        cos_lut[t] = std::cos(rad);
        sin_lut[t] = std::sin(rad);
    }

    // 4. 3D OYLAMA MATRİSİ (3D ACCUMULATOR) OLUŞTURMA
    // Boyutlar: Width x Height x Num_Radii
    // 1D dinamik dizi olarak allocate edip indexleme yapacağız: index = a + b*W + r_idx*W*H
    std::vector<int> accum_3d(width * height * num_radii, 0);

    auto get_accum = [&](int a, int b, int r_idx) -> int& {
        return accum_3d[a + b * width + r_idx * width * height];
    };

    // 5. OYLAMA İŞLEMİ (ACCUMULATION PROCESS)
    std::cout << "[BİLGİ] 3D Hough Oylama Matrisi hesaplaniyor..." << std::endl;
    for (int y = 0; y < height; ++y) {
        const uchar* edge_row = edges.ptr<uchar>(y);
        for (int x = 0; x < width; ++x) {
            if (edge_row[x] > 0) { // Kenar pikseli ise
                for (int r_idx = 0; r_idx < num_radii; ++r_idx) {
                    int r = min_r + r_idx;
                    for (int t = 0; t < num_thetas; t += 2) { // 2 derece adımlarla oylama
                        int a = static_cast<int>(std::round(x - r * cos_lut[t]));
                        int b = static_cast<int>(std::round(y - r * sin_lut[t]));

                        // Görsel sınırları içerisinde ise oyu artır
                        if (a >= 0 && a < width && b >= 0 && b < height) {
                            get_accum(a, b, r_idx)++;
                        }
                    }
                }
            }
        }
    }

    // 6. EŞİKLEME VE NON-MAXIMUM SUPPRESSION (NMS)
    int threshold = 80; // Minimum kabul oyu
    std::vector<Circle> detected_circles;

    for (int r_idx = 0; r_idx < num_radii; ++r_idx) {
        int r = min_r + r_idx;
        for (int b = 1; b < height - 1; ++b) {
            for (int a = 1; a < width - 1; ++a) {
                int votes = get_accum(a, b, r_idx);

                if (votes >= threshold) {
                    // 3D Komşulukta Yerel Maksimum Kontrolü (3x3x3 NMS)
                    bool is_max = true;
                    for (int dr = -1; dr <= 1 && is_max; ++dr) {
                        for (int db = -1; db <= 1 && is_max; ++db) {
                            for (int da = -1; da <= 1 && is_max; ++da) {
                                if (da == 0 && db == 0 && dr == 0) continue;
                                int na = a + da;
                                int nb = b + db;
                                int nr_idx = r_idx + dr;

                                if (na >= 0 && na < width && nb >= 0 && nb < height && nr_idx >= 0 && nr_idx < num_radii) {
                                    if (get_accum(na, nb, nr_idx) > votes) {
                                        is_max = false;
                                    }
                                }
                            }
                        }
                    }

                    if (is_max) {
                        detected_circles.push_back({a, b, r, votes});
                    }
                }
            }
        }
    }

    // Yakın çemberleri filtreleme (Overlap Suppression)
    std::sort(detected_circles.begin(), detected_circles.end(), [](const Circle& c1, const Circle& c2) {
        return c1.votes > c2.votes;
    });

    std::vector<Circle> final_circles;
    for (const auto& c : detected_circles) {
        bool too_close = false;
        for (const auto& fc : final_circles) {
            double dist = std::sqrt((c.a - fc.a)*(c.a - fc.a) + (c.b - fc.b)*(c.b - fc.b));
            if (dist < 20) { // Merkezleri 20 pikselden yakınsa bastır
                too_close = true;
                break;
            }
        }
        if (!too_close) {
            final_circles.push_back(c);
        }
    }

    // 7. SONUÇLARI ÇİZME
    cv::Mat result_img = src.clone();
    for (const auto& c : final_circles) {
        // Çember Dış Hattı (Mavi)
        cv::circle(result_img, cv::Point(c.a, c.b), c.r, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
        // Çember Merkezi (Kırmızı)
        cv::circle(result_img, cv::Point(c.a, c.b), 3, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
    }

    // 8. ÇIKTILARI BİRLEŞTİRİP KAYDETME (2'li Panel: Canny Kenarları | Custom Hough Circles)
    int w = src.cols, h = src.rows;
    cv::Mat panel(h, w * 2, CV_8UC3);

    cv::Mat edges_bgr;
    cv::cvtColor(edges, edges_bgr, cv::COLOR_GRAY2BGR);

    edges_bgr.copyTo(panel(cv::Rect(0, 0, w, h)));
    result_img.copyTo(panel(cv::Rect(w, 0, w, h)));

    cv::putText(panel, "1. Canny Kenarlari", cv::Point(20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "2. Custom 3D Hough Circles", cv::Point(w + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    cv::imwrite("/content/custom_hough_circles_results.png", panel);

    std::cout << "==================================================" << std::endl;
    std::cout << "     CUSTOM 3D HOUGH CIRCLE TRANSFORM RAPORU      " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "3D Akümülatör Boyutu     : " << width << " x " << height << " x " << num_radii << std::endl;
    std::cout << "Test Edilen Yarıçaplar   : [" << min_r << " - " << max_r << "] piksel" << std::endl;
    std::cout << "Seçilen Eşik (Threshold) : " << threshold << " oy" << std::endl;
    std::cout << "Tespit Edilen Çember    : " << final_circles.size() << " adet" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "[OK] Sonuc kaydedildi -> /content/custom_hough_circles_results.png" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_custom_hough_circles.cpp -o task_custom_hough_circles \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_custom_hough_circles