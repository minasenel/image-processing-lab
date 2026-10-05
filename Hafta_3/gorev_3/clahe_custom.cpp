
// Bu kod, OpenCV kullanarak manuel olarak CLAHE (Contrast Limited Adaptive Histogram Equalization) algoritmasını uygular.
//[BILGI] Manuel CLAHE Suresi: 16875 mikrosaniye
//[OK] Manuel CLAHE kaydedildi: /content/image2_clahe_custom.png

%%bash
cat << 'EOF' > clahe_custom.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
    cv::Mat src = cv::imread("/content/image2.png", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image2.png bulunamadi!" << std::endl;
        return 1;
    }

    const int width = src.cols;
    const int height = src.rows;
    const int grid_x = 8;
    const int grid_y = 8;
    const double clip_limit_ratio = 2.0;

    auto t_start = std::chrono::high_resolution_clock::now();

    // 1. Grid Boyutlarını Belirle
    std::vector<int> x_starts(grid_x), x_ends(grid_x);
    std::vector<int> y_starts(grid_y), y_ends(grid_y);
    std::vector<double> x_centers(grid_x), y_centers(grid_y);

    for (int i = 0; i < grid_x; ++i) {
        x_starts[i] = i * width / grid_x;
        x_ends[i] = (i + 1) * width / grid_x;
        x_centers[i] = (x_starts[i] + x_ends[i] - 1) / 2.0;
    }
    for (int j = 0; j < grid_y; ++j) {
        y_starts[j] = j * height / grid_y;
        y_ends[j] = (j + 1) * height / grid_y;
        y_centers[j] = (y_starts[j] + y_ends[j] - 1) / 2.0;
    }

    // 2. Her Blok İçin Kırpılmış Histogram ve Kümülatif LUT Hesapla
    std::vector<std::vector<std::vector<uchar>>> luts(grid_y, std::vector<std::vector<uchar>>(grid_x, std::vector<uchar>(256)));

    for (int gy = 0; gy < grid_y; ++gy) {
        for (int gx = 0; gx < grid_x; ++gx) {
            int block_w = x_ends[gx] - x_starts[gx];
            int block_h = y_ends[gy] - y_starts[gy];
            int total_p = block_w * block_h;

            // Blok Histogramı
            std::vector<int> hist(256, 0);
            for (int r = y_starts[gy]; r < y_ends[gy]; ++r) {
                const uchar* row = src.ptr<uchar>(r);
                for (int c = x_starts[gx]; c < x_ends[gx]; ++c) {
                    hist[row[c]]++;
                }
            }

            // Kırpma Eşiği (Clip Limit) ve Fazlalığı Dağıtma
            int clip_val = std::max(1, static_cast<int>(clip_limit_ratio * (total_p / 256.0)));
            int excess = 0;
            for (int i = 0; i < 256; ++i) {
                if (hist[i] > clip_val) {
                    excess += (hist[i] - clip_val);
                    hist[i] = clip_val;
                }
            }

            int bonus = excess / 256;
            int remainder = excess % 256;
            for (int i = 0; i < 256; ++i) {
                hist[i] += bonus;
                if (i < remainder) hist[i]++;
            }

            // CDF ve Dönüşüm Tablosu (LUT)
            int sum = 0;
            for (int i = 0; i < 256; ++i) {
                sum += hist[i];
                luts[gy][gx][i] = static_cast<uchar>(std::round((static_cast<float>(sum) / total_p) * 255.0f));
            }
        }
    }

    // 3. Bilinear Enterpolasyon ile Pikselleri Haritalama
    cv::Mat dst(height, width, CV_8UC1);

    for (int r = 0; r < height; ++r) {
        uchar* dst_row = dst.ptr<uchar>(r);
        const uchar* src_row = src.ptr<uchar>(r);

        // Y Ekseninde İlgili İki Grid Merkezi
        int y1 = 0, y2 = 0;
        double wy = 0.0;
        if (r <= y_centers[0]) {
            y1 = y2 = 0;
            wy = 0.0;
        } else if (r >= y_centers[grid_y - 1]) {
            y1 = y2 = grid_y - 1;
            wy = 0.0;
        } else {
            for (int j = 0; j < grid_y - 1; ++j) {
                if (r >= y_centers[j] && r < y_centers[j + 1]) {
                    y1 = j;
                    y2 = j + 1;
                    wy = (r - y_centers[j]) / (y_centers[j + 1] - y_centers[j]);
                    break;
                }
            }
        }

        for (int c = 0; c < width; ++c) {
            uchar val = src_row[c];

            // X Ekseninde İlgili İki Grid Merkezi
            int x1 = 0, x2 = 0;
            double wx = 0.0;
            if (c <= x_centers[0]) {
                x1 = x2 = 0;
                wx = 0.0;
            } else if (c >= x_centers[grid_x - 1]) {
                x1 = x2 = grid_x - 1;
                wx = 0.0;
            } else {
                for (int i = 0; i < grid_x - 1; ++i) {
                    if (c >= x_centers[i] && c < x_centers[i + 1]) {
                        x1 = i;
                        x2 = i + 1;
                        wx = (c - x_centers[i]) / (x_centers[i + 1] - x_centers[i]);
                        break;
                    }
                }
            }

            // Dört Komşu Bloktan Bilinear Enterpolasyon
            double v11 = luts[y1][x1][val];
            double v12 = luts[y1][x2][val];
            double v21 = luts[y2][x1][val];
            double v22 = luts[y2][x2][val];

            double top = (1.0 - wx) * v11 + wx * v12;
            double bottom = (1.0 - wx) * v21 + wx * v22;
            double final_val = (1.0 - wy) * top + wy * bottom;

            dst_row[c] = static_cast<uchar>(std::round(final_val));
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[BILGI] Manuel CLAHE Suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/image2_clahe_custom.png", dst);
    std::cout << "[OK] Manuel CLAHE kaydedildi: /content/image2_clahe_custom.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 clahe_custom.cpp -o clahe_custom \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./clahe_custom