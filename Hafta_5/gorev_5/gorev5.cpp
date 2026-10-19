%%bash
cat << 'EOF' > task_xray_pseudocolor_test.cpp
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// 1. SIFIRDAN GRAYSCALE DÖNÜŞTÜRME (Luminance BT.601)
cv::Mat convertToGrayCustom(const cv::Mat& src) {
    cv::Mat gray(src.rows, src.cols, CV_8UC1);
    for (int r = 0; r < src.rows; ++r) {
        const cv::Vec3b* src_row = src.ptr<cv::Vec3b>(r);
        uchar* dst_row = gray.ptr<uchar>(r);
        for (int c = 0; c < src.cols; ++c) {
            float b = src_row[c][0];
            float g = src_row[c][1];
            float r_val = src_row[c][2];
            float y = 0.114f * b + 0.587f * g + 0.299f * r_val;
            dst_row[c] = static_cast<uchar>(std::round(y));
        }
    }
    return gray;
}

int main() {
    // test1.jpg - test5.jpg girdi listesi
    std::vector<std::string> input_files = {
        "/content/test1.jpg",
        "/content/test2.jpg",
        "/content/test3.jpg",
        "/content/test4.jpg",
        "/content/test5.jpg"
    };

    for (size_t i = 0; i < input_files.size(); ++i) {
        cv::Mat src = cv::imread(input_files[i], cv::IMREAD_COLOR);
        if (src.empty()) {
            std::cerr << "[HATA] " << input_files[i] << " okunamadi! Atlaniyor." << std::endl;
            continue;
        }

        // A) Custom Gray Dönüşümü
        cv::Mat gray = convertToGrayCustom(src);

        // B) Güvenlik Taraması İçin Pseudocolor Renklendirme (JET Paleti)
        cv::Mat pseudo_color;
        cv::applyColorMap(gray, pseudo_color, cv::COLORMAP_JET);

        // C) Sol: Orijinal | Sağ: Renklendirilmiş (Yan Yana Birleştirme)
        int w = src.cols, h = src.rows;
        cv::Mat result_panel(h, w * 2, CV_8UC3);

        src.copyTo(result_panel(cv::Rect(0, 0, w, h)));
        pseudo_color.copyTo(result_panel(cv::Rect(w, 0, w, h)));

        // Etiketler
        cv::putText(result_panel, "Orijinal Resim", cv::Point(20, 35), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        cv::putText(result_panel, "Güvenlik Renklendirmesi (JET)", cv::Point(w + 20, 35), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

        // İstenen formatta kaydetme (test1_results.png ... test5_results.png)
        std::string out_filename = "/content/test" + std::to_string(i + 1) + "_results.png";
        cv::imwrite(out_filename, result_panel);
        std::cout << "[OK] Kaydedildi: " << out_filename << std::endl;
    }

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_xray_pseudocolor_test.cpp -o task_xray_pseudocolor_test \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_xray_pseudocolor_test