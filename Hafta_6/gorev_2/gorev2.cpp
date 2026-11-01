/*==================================================
       CUSTOM HOUGH TRANSFORM HESAPLAMA RAPORU    
==================================================
Akümülatör Matris Boyutu : 857 x 180
En Yüksek Oy Sayısı      : 259
Seçilen Eşik (Threshold) : 100
Tespit Edilen Doğru Sayısı: 25*/

%%bash
cat << 'EOF' > task_custom_hough.cpp
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

int main() {
    // 1. Görseli Oku (sudoku.jpg)
    cv::Mat src = cv::imread("/content/sudoku.jpg", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/sudoku.jpg okunamadi!" << std::endl;
        return 1;
    }

    // 2. Grayscale ve Canny Kenar Tespiti
    cv::Mat gray, blurred, edges;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 0);
    cv::Canny(blurred, edges, 20, 60);

    int width = edges.cols;
    int height = edges.rows;

    // Maximum olası Rho mesafesi (Köşegen uzunluğu)
    int max_rho = static_cast<int>(std::ceil(std::sqrt(width * width + height * height)));
    int rho_bins = 2 * max_rho + 1; // Rho [-max_rho, +max_rho] aralığında
    int num_thetas = 180;          // Theta [0, 179] derece aralığında

    // 3. SIN & COS LOOK-UP TABLE (LUT) OLUŞTURMA
    // Trigonometrik hesaplamaları bellekte tutarak performansı artırıyoruz
    std::vector<double> cos_lut(num_thetas);
    std::vector<double> sin_lut(num_thetas);

    for (int theta_deg = 0; theta_deg < num_thetas; ++theta_deg) {
        double rad = theta_deg * M_PI / 180.0;
        cos_lut[theta_deg] = std::cos(rad);
        sin_lut[theta_deg] = std::sin(rad);
    }

    // 4. OYLAMA MATRİSİ (ACCUMULATOR MATRIX) OLUŞTURMA
    // Satır: Rho indisleri, Sütun: Theta (açı) dereceleri
    cv::Mat accumulator = cv::Mat::zeros(rho_bins, num_thetas, CV_32SC1);

    // 5. OYLAMA İŞLEMİ (ACCUMULATION PROCESS)
    for (int y = 0; y < height; ++y) {
        const uchar* edge_row = edges.ptr<uchar>(y);
        for (int x = 0; x < width; ++x) {
            // Eğer piksel bir kenar pikseli ise (255)
            if (edge_row[x] > 0) {
                for (int theta_deg = 0; theta_deg < num_thetas; ++theta_deg) {
                    // rho = x * cos(theta) + y * sin(theta)
                    double rho_val = x * cos_lut[theta_deg] + y * sin_lut[theta_deg];
                    int rho_idx = static_cast<int>(std::round(rho_val)) + max_rho;

                    if (rho_idx >= 0 && rho_idx < rho_bins) {
                        accumulator.at<int>(rho_idx, theta_deg)++; // Oy ver
                    }
                }
            }
        }
    }

    // 6. OYLAMA MATRİSİNİ RESİM OLARAK KAYDETME
    cv::Mat accum_visual;
    double max_val;
    cv::minMaxLoc(accumulator, nullptr, &max_val);
    
    // Oyları 0-255 aralığına normalize etme ve renk haritası uygulama
    accumulator.convertTo(accum_visual, CV_8UC1, 255.0 / (max_val > 0 ? max_val : 1.0));
    cv::Mat accum_colored;
    cv::applyColorMap(accum_visual, accum_colored, cv::COLORMAP_JET);
    cv::imwrite("/content/hough_accumulator.png", accum_colored);

    // 7. EŞİK DEĞERİ (THRESHOLD) VE TEPE NOKTASI (PEAK) TESPİTİ
    int threshold = 100; // Bir çizgi kabul edilmesi için gereken minimum oy sayısı
    cv::Mat result_img = src.clone();
    int detected_lines_count = 0;

    for (int r = 0; r < rho_bins; ++r) {
        for (int t = 0; t < num_thetas; ++t) {
            int votes = accumulator.at<int>(r, t);

            if (votes >= threshold) {
                // Basit Non-Maximum Suppression (Yerel maksimum kontrolü)
                bool is_peak = true;
                for (int dr = -1; dr <= 1 && is_peak; ++dr) {
                    for (int dt = -1; dt <= 1 && is_peak; ++dt) {
                        if (dr == 0 && dt == 0) continue;
                        int nr = r + dr;
                        int nt = t + dt;
                        if (nr >= 0 && nr < rho_bins && nt >= 0 && nt < num_thetas) {
                            if (accumulator.at<int>(nr, nt) > votes) {
                                is_peak = false;
                            }
                        }
                    }
                }

                if (!is_peak) continue;

                // Koordinat dönüşümü: (rho_idx, theta_deg) -> Gerçek (rho, theta_rad)
                double rho = r - max_rho;
                double theta_rad = t * M_PI / 180.0;

                double a = cos_lut[t];
                double b = sin_lut[t];
                double x0 = a * rho;
                double y0 = b * rho;

                // Çizgiyi tuval dışına doğru uzatacak 2 uç nokta hesabı
                cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
                cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));

                cv::line(result_img, pt1, pt2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
                detected_lines_count++;
            }
        }
    }

    // 8. ÇIKTILARI BİRLEŞTİRİP KAYDETME (Panel: Kenarlar | Akümülatör | Bulunan Doğrular)
    cv::Mat edges_bgr;
    cv::cvtColor(edges, edges_bgr, cv::COLOR_GRAY2BGR);

    // Akümülatör görselini girdi resminin boyutuna yeniden boyutlandır
    cv::Mat accum_resized;
    cv::resize(accum_colored, accum_resized, cv::Size(width, height));

    cv::Mat panel(height, width * 3, CV_8UC3);
    edges_bgr.copyTo(panel(cv::Rect(0, 0, width, height)));
    accum_resized.copyTo(panel(cv::Rect(width, 0, width, height)));
    result_img.copyTo(panel(cv::Rect(width * 2, 0, width, height)));

    cv::putText(panel, "1. Canny Kenarlari", cv::Point(20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "2. Hough Akumulatore (Sinogram)", cv::Point(width + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
    cv::putText(panel, "3. Custom Hough Dogrulari", cv::Point(width * 2 + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

    cv::imwrite("/content/custom_hough_results.png", panel);

    std::cout << "==================================================" << std::endl;
    std::cout << "       CUSTOM HOUGH TRANSFORM HESAPLAMA RAPORU    " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Akümülatör Matris Boyutu : " << rho_bins << " x " << num_thetas << std::endl;
    std::cout << "En Yüksek Oy Sayısı      : " << max_val << std::endl;
    std::cout << "Seçilen Eşik (Threshold) : " << threshold << std::endl;
    std::cout << "Tespit Edilen Doğru Sayısı: " << detected_lines_count << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "[OK] Oylama Görseli Kaydedildi -> /content/hough_accumulator.png" << std::endl;
    std::cout << "[OK] Karşılaştırma Paneli Kaydedildi -> /content/custom_hough_results.png" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_custom_hough.cpp -o task_custom_hough \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_custom_hough