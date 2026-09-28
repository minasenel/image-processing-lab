%%bash
cat << 'EOF' > task4_histogram.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// Her thread kendi ROI alanındaki pikselleri sayar
void compute_sub_histogram(const cv::Mat& src, cv::Rect roi, std::vector<int>& local_hist, int thread_id) {
    cv::Mat roi_mat = src(roi);
    int rows = roi_mat.rows;
    int cols = roi_mat.cols;

    for (int r = 0; r < rows; ++r) {
        const uchar* row_ptr = roi_mat.ptr<uchar>(r);
        for (int c = 0; c < cols; ++c) {
            uchar intensity = row_ptr[c];
            local_hist[intensity]++;
        }
    }

    int local_sum = std::accumulate(local_hist.begin(), local_hist.end(), 0);
    std::cout << "[Cekirdek " << thread_id << "] ROI islendi. Bu parcadaki piksel sayisi: " 
              << local_sum << " (Beklenen: " << (rows * cols) << ")\n";
}

int main() {
    // 1. Kadın resmini gri seviyeli oku (0-255 arası tek kanal)
    cv::Mat img = cv::imread("human_face.jpg", cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "Resim yuklenemedi!" << std::endl;
        return 1;
    }

    int total_rows = img.rows;
    int total_cols = img.cols;
    int total_pixels = total_rows * total_cols;

    std::cout << "Gorsel Boyutu : " << total_cols << "x" << total_rows 
              << " (" << total_pixels << " piksel)\n\n";

    const int num_parts = 4;
    int slice_height = total_rows / num_parts;

    // 4 cekirdek icin ayri ayri 256 elemanli histogram havuzlari
    std::vector<std::vector<int>> sub_histograms(num_parts, std::vector<int>(256, 0));
    std::vector<std::thread> workers;
    workers.reserve(num_parts);

    // 2. Resmi 4 ROI parcasina bol ve thread'leri baslat
    for (int i = 0; i < num_parts; ++i) {
        int start_y = i * slice_height;
        int h = (i == num_parts - 1) ? (total_rows - start_y) : slice_height;
        cv::Rect roi(0, start_y, total_cols, h);

        workers.emplace_back(compute_sub_histogram, std::cref(img), roi, std::ref(sub_histograms[i]), i);
    }

    // 3. Thread'lerin bitmesini bekle
    for (auto& th : workers) {
        if (th.joinable()) th.join();
    }

    // 4. Alt histogramlari tek bir ana histogramda birlestir (Reduction)
    std::vector<int> global_histogram(256, 0);
    for (int i = 0; i < num_parts; ++i) {
        for (int bin = 0; bin < 256; ++bin) {
            global_histogram[bin] += sub_histograms[i][bin];
        }
    }

    // 5. Dogrulama Hesabi (Tum parlaklik frekanslarinin toplami)
    long long verified_total = 0;
    for (int bin = 0; bin < 256; ++bin) {
        verified_total += global_histogram[bin];
    }

    std::cout << "\n======================================================\n";
    std::cout << "                 HISTOGRAM SONUCLARI                  \n";
    std::cout << "======================================================\n";
    std::cout << "Ornek bazi parlaklik degerleri ve piksel adetleri:\n";
    std::cout << "- Parlaklik 0   (Tam Siyah) : " << global_histogram[0]   << " adet piksel\n";
    std::cout << "- Parlaklik 64  (Koyu Gri)  : " << global_histogram[64]  << " adet piksel\n";
    std::cout << "- Parlaklik 128 (Orta Gri)  : " << global_histogram[128] << " adet piksel\n";
    std::cout << "- Parlaklik 192 (Acik Gri)  : " << global_histogram[192] << " adet piksel\n";
    std::cout << "- Parlaklik 255 (Tam Beyaz) : " << global_histogram[255] << " adet piksel\n";
    std::cout << "------------------------------------------------------\n";
    std::cout << "Histogram Toplam Piksel Sayisi : " << verified_total << "\n";
    std::cout << "Resmin Gercek Piksel Sayisi   : " << total_pixels << "\n";

    if (verified_total == total_pixels) {
        std::cout << "\n[DOGRULAMA BASARILI]: Histogram toplami resmin toplam piksel sayisina tam esittir!\n";
    } else {
        std::cout << "\n[HATA]: Sayim eksik veya hatali!\n";
    }
    std::cout << "======================================================\n";

    return 0;
}
EOF

# Derle ve Calistir
g++ -std=c++17 -O3 -pthread task4_histogram.cpp -o task4_histogram \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task4_histogram
```
