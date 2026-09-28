%%bash
cat << 'EOF' > app3_roi_threads.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// Her bir CPU çekirdeğinin (thread) çalıştıracağı fonksiyon
// src ve dst matrislerinin ilgili ROI dilimini işler
void quantize_roi(const cv::Mat& src, cv::Mat& dst, cv::Rect roi, int thread_id) {
    // Matris üzerinde kopyalama yapmadan o bölgeye doğrudan referans (pencere) açılır
    cv::Mat src_roi = src(roi);
    cv::Mat dst_roi = dst(roi);

    int rows = src_roi.rows;
    int cols = src_roi.cols;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            uchar val = src_roi.at<uchar>(r, c);

            // 4 seviyeli kuantalama: 0, 64, 128, 192
            uchar level = val / 64;
            dst_roi.at<uchar>(r, c) = level * 64;
        }
    }
    std::cout << "[Cekirdek " << thread_id << "] ROI basariyla islendi -> Y: " 
              << roi.y << " | Yukseklik: " << roi.height << " | Genislik: " << roi.width << std::endl;
}

int main() {
    // 1. Kadın resmini gri seviyeli olarak oku
    cv::Mat img = cv::imread("human_face.jpg", cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "Resim yuklenemedi! Lutfen human_face.jpg dosyasini kontrol edin." << std::endl;
        return 1;
    }

    int total_rows = img.rows;
    int total_cols = img.cols;

    // Çıktı için aynı boyutlarda tek kanallı matris
    cv::Mat result = cv::Mat::zeros(total_rows, total_cols, CV_8UC1);

    // 2. Parça / Çekirdek sayısı belirleme (4 parçaya bölüyoruz)
    const int num_parts = 4;
    std::vector<std::thread> workers;

    int slice_height = total_rows / num_parts;

    std::cout << "Resim Boyutu: " << total_cols << "x" << total_rows << std::endl;
    std::cout << "Resim " << num_parts << " farkli ROI parcasina bolunup " 
              << num_parts << " ayri CPU cekirdegine gonderiliyor...\n" << std::endl;

    // 3. Resmi N parçaya (ROI) böl ve her birini ayrı bir thread'e ata
    for (int i = 0; i < num_parts; ++i) {
        int start_y = i * slice_height;
        // Son parçanın tam sığması için kalan satırları ekle (yuvarlama farkları için)
        int h = (i == num_parts - 1) ? (total_rows - start_y) : slice_height;

        // cv::Rect(x, y, genislik, yukseklik) ile ROI tanımlanır
        cv::Rect roi(0, start_y, total_cols, h);

        // Yeni thread başlat
        workers.emplace_back(quantize_roi, std::cref(img), std::ref(result), roi, i);
    }

    // 4. Tüm çekirdeklerin işlerini bitirmesini bekle (join)
    for (auto& th : workers) {
        if (th.joinable()) {
            th.join();
        }
    }

    // 5. Birleşik nihai kuantalanmış görüntüyü kaydet
    cv::imwrite("woman_roi_multithread_quantized.png", result);

    std::cout << "\nTum parcalar birlestirildi ve kaydedildi: woman_roi_multithread_quantized.png" << std::endl;
    return 0;
}
EOF

# Derle ve Çalıştır (-pthread desteğiyle)
g++ -std=c++17 -pthread app3_roi_threads.cpp -o app3_roi_threads \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./app3_roi_threads

# Sonucu zip yap
zip -j -o app3_roi_result.zip woman_roi_multithread_quantized.png
