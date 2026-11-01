/*
EŞ MERKEZLİ HOUGH TRANSFORM İYİLEŞTİRME NOTU: istediğim görüntüyü ilk yaptığımda alamadım
bu sebeple treshold değerini dinamik olarak ayarladım ve NMS penceresini genişlettim.
1. Dinamik Eşikleme (Dynamic Thresholding):
   Sabit eşik değerleri (threshold) farklı çözünürlükteki görsellerde tespit hatasına veya sıfır tespite yol açtığı için, 
   merkez kabul eşiği akümülatördeki maksimum oyun %50'sine (max_center_vote * 0.50) dinamik olarak bağlandı.

2. Genişletilmiş NMS (Non-Maximum Suppression):
   Eş merkezli yapılarda piksel ayrışmasından kaynaklı yalancı/çakışan merkez gürültülerini engellemek için 
   akümülatör üzerindeki komşuluk bastırma penceresi 15x15 boyutuna genişletildi.

3. Kenar Hassasiyeti:
   Canny eşikleri (30, 90) seviyesine çekilip Gauss filtresi (3x3) 
   hafifletilerek dairesel katman kenarlarının kopması önlendi ve gerçek ana merkez tespit edildi.


   sonuçlar:
[1/4] Dartboard uzerinde es merkezli Hough oylamasi yapiliyor...
[BİLGİ] Akümülatörde kaydedilen EN YÜKSEK merkez oyu: 6448
[2/4] Dinamik NMS ile merkezler tespit ediliyor...
[3/4] Ana merkez ve es merkezli halkalar ciziliyor...
==================================================
  DİNAMİK EŞİKLİ DARTBOARD EŞ MERKEZLİ HOUGH    
==================================================
Tespit Edilen Merkez Sayısı : 1
Ana Merkez Koordinatı       : (551, 544)
Merkezin Aldığı Toplam Oy   : 6448
Çizilen Halka Sayısı         : 15
[OK] Kaydedildi -> /content/dartboard_concentric_fixed_results.png
==================================================

*/

%%bash
cat << 'EOF' > task_concentric_dartboard_fixed.cpp
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

struct ConcentricCircleGroup {
    int a;                  // Ortak Merkez X
    int b;                  // Ortak Merkez Y
    int total_center_votes; // Merkezin aldığı toplam oy
    std::vector<int> radii; // Bu merkeze ait halka yarıçapları
};

int main() {
    // 1. GÖRSELİ OKUMA (dartboard.png)
    cv::Mat src = cv::imread("/content/dartboard.png", cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "[HATA] /content/dartboard.png bulunamadi!" << std::endl;
        return 1;
    }

    // 2. GRAYSCALE & HAFİF GAUSSIAN BLUR & CANNY
    cv::Mat gray, blurred, edges;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    // Kenarları korumak için 3x3 hafif Gauss filtresi
    cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 1.0);
    cv::Canny(blurred, edges, 30, 90);

    int width = edges.cols;
    int height = edges.rows;

    int min_r = 15;
    int max_r = std::min(width, height) / 2;
    int num_radii = max_r - min_r + 1;
    int num_thetas = 360;

    // 3. TRIGONOMETRİK LOOK-UP TABLE (LUT)
    std::vector<double> cos_lut(num_thetas);
    std::vector<double> sin_lut(num_thetas);
    for (int t = 0; t < num_thetas; ++t) {
        double rad = t * M_PI / 180.0;
        cos_lut[t] = std::cos(rad);
        sin_lut[t] = std::sin(rad);
    }

    // 4. OYLAMA MATRİSLERİ (3D ve 2D CENTER ACCUMULATOR)
    std::vector<int> accum_3d(width * height * num_radii, 0);
    std::vector<int> accum_center(width * height, 0);

    auto get_accum_3d = [&](int a, int b, int r_idx) -> int& {
        return accum_3d[a + b * width + r_idx * width * height];
    };

    auto get_accum_center = [&](int a, int b) -> int& {
        return accum_center[a + b * width];
    };

    // 5. EŞ MERKEZLİ OYLAMA İŞLEMİ
    std::cout << "[1/4] Dartboard uzerinde es merkezli Hough oylamasi yapiliyor..." << std::endl;
    int max_center_vote_found = 0;

    for (int y = 0; y < height; ++y) {
        const uchar* edge_row = edges.ptr<uchar>(y);
        for (int x = 0; x < width; ++x) {
            if (edge_row[x] > 0) {
                for (int r_idx = 0; r_idx < num_radii; ++r_idx) {
                    int r = min_r + r_idx;
                    for (int t = 0; t < num_thetas; t += 2) {
                        int a = static_cast<int>(std::round(x - r * cos_lut[t]));
                        int b = static_cast<int>(std::round(y - r * sin_lut[t]));

                        if (a >= 0 && a < width && b >= 0 && b < height) {
                            get_accum_3d(a, b, r_idx)++;
                            int current_votes = ++get_accum_center(a, b);
                            if (current_votes > max_center_vote_found) {
                                max_center_vote_found = current_votes;
                            }
                        }
                    }
                }
            }
        }
    }

    std::cout << "[BİLGİ] Akümülatörde kaydedilen EN YÜKSEK merkez oyu: " << max_center_vote_found << std::endl;

    // 6. DİNAMİK EŞİKLEME İLE MERKEZ TESPİTİ
    // Eşik sabit sayı değil, resmin aldığı maks oyun %50'si olarak ayarlandı
    int center_threshold = static_cast<int>(max_center_vote_found * 0.50); 
    int layer_threshold = 25; // Halka tespit eşiği
    int nms_window = 7;      // 15x15 NMS komşuluk penceresi

    std::vector<ConcentricCircleGroup> candidate_groups;

    std::cout << "[2/4] Dinamik NMS ile merkezler tespit ediliyor..." << std::endl;
    for (int b = nms_window; b < height - nms_window; ++b) {
        for (int a = nms_window; a < width - nms_window; ++a) {
            int center_votes = get_accum_center(a, b);

            if (center_votes >= center_threshold) {
                // NMS Kontrolü
                bool is_center_max = true;
                for (int db = -nms_window; db <= nms_window && is_center_max; ++db) {
                    for (int da = -nms_window; da <= nms_window && is_center_max; ++da) {
                        if (da == 0 && db == 0) continue;
                        if (get_accum_center(a + da, b + db) > center_votes) {
                            is_center_max = false;
                        }
                    }
                }

                if (is_center_max) {
                    std::vector<int> valid_radii;
                    for (int r_idx = 0; r_idx < num_radii; ++r_idx) {
                        int r_votes = get_accum_3d(a, b, r_idx);
                        if (r_votes >= layer_threshold) {
                            bool is_r_max = true;
                            if (r_idx > 0 && get_accum_3d(a, b, r_idx - 1) > r_votes) is_r_max = false;
                            if (r_idx < num_radii - 1 && get_accum_3d(a, b, r_idx + 1) > r_votes) is_r_max = false;

                            if (is_r_max) {
                                int current_r = min_r + r_idx;
                                if (valid_radii.empty() || (current_r - valid_radii.back() > 8)) {
                                    valid_radii.push_back(current_r);
                                }
                            }
                        }
                    }

                    if (!valid_radii.empty()) { 
                        candidate_groups.push_back({a, b, center_votes, valid_radii});
                    }
                }
            }
        }
    }

    // En yüksek oya sahip ana merkezi başa getir
    std::sort(candidate_groups.begin(), candidate_groups.end(), [](const ConcentricCircleGroup& g1, const ConcentricCircleGroup& g2) {
        return g1.total_center_votes > g2.total_center_votes;
    });

    std::vector<ConcentricCircleGroup> final_groups;
    if (!candidate_groups.empty()) {
        final_groups.push_back(candidate_groups[0]); // Sadece en güçlü tek ana merkez
    }

    // 7. ÇİZİM İŞLEMİ
    std::cout << "[3/4] Ana merkez ve es merkezli halkalar ciziliyor..." << std::endl;
    cv::Mat result_img = src.clone();

    int total_rings_drawn = 0;
    for (const auto& group : final_groups) {
        // Ortak Merkez (Bullseye - Kırmızı Dolgu)
        cv::circle(result_img, cv::Point(group.a, group.b), 5, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);

        // Katman Çizimleri (Canlı Renkler)
        for (size_t i = 0; i < group.radii.size(); ++i) {
            int r = group.radii[i];
            cv::Scalar color;
            if (i % 4 == 0) color = cv::Scalar(0, 255, 0);       // Yeşil
            else if (i % 4 == 1) color = cv::Scalar(0, 255, 255); // Sarı
            else if (i % 4 == 2) color = cv::Scalar(255, 255, 0); // Cyan
            else color = cv::Scalar(255, 0, 255);                // Magenta

            cv::circle(result_img, cv::Point(group.a, group.b), r, color, 2, cv::LINE_AA);
            total_rings_drawn++;
        }
    }

    // 8. PANEL KAYDETME
    cv::Mat panel(height, width * 2, CV_8UC3);

    cv::Mat edges_bgr;
    cv::cvtColor(edges, edges_bgr, cv::COLOR_GRAY2BGR);

    edges_bgr.copyTo(panel(cv::Rect(0, 0, width, height)));
    result_img.copyTo(panel(cv::Rect(width, 0, width, height)));

    cv::putText(panel, "1. Canny Kenarlari", cv::Point(20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(panel, "2. Es Merkezli Hough Circles", cv::Point(width + 20, 35), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    cv::imwrite("/content/dartboard_concentric_fixed_results.png", panel);

    std::cout << "==================================================" << std::endl;
    std::cout << "  DİNAMİK EŞİKLİ DARTBOARD EŞ MERKEZLİ HOUGH    " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Tespit Edilen Merkez Sayısı : " << final_groups.size() << std::endl;
    if (!final_groups.empty()) {
        std::cout << "Ana Merkez Koordinatı       : (" << final_groups[0].a << ", " << final_groups[0].b << ")" << std::endl;
        std::cout << "Merkezin Aldığı Toplam Oy   : " << final_groups[0].total_center_votes << std::endl;
    }
    std::cout << "Çizilen Halka Sayısı         : " << total_rings_drawn << std::endl;
    std::cout << "[OK] Kaydedildi -> /content/dartboard_concentric_fixed_results.png" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_concentric_dartboard_fixed.cpp -o task_concentric_dartboard_fixed \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_concentric_dartboard_fixed