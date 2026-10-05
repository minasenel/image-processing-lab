%%bash
cat << 'EOF' > run_image2.cpp
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>

int main() {
    // 1. /content/image2.png dosyasını tek kanallı oku
    cv::Mat h_src = cv::imread("/content/image2.png", cv::IMREAD_GRAYSCALE);
    if (h_src.empty()) {
        std::cerr << "[HATA] /content/image2.png bulunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << h_src.cols << "x" << h_src.rows << std::endl;

    // GPU'ya Aktar
    cv::cuda::GpuMat d_src, d_stretched;
    d_src.upload(h_src);

    auto t_start = std::chrono::high_resolution_clock::now();

    // 2. Min ve Max Değerleri GPU üzerinde bul
    double min_val = 0.0, max_val = 0.0;
    cv::cuda::minMaxLoc(d_src, &min_val, &max_val, nullptr, nullptr);

    std::cout << "[ORIJINAL] Min: " << min_val << " | Max: " << max_val << std::endl;

    if (max_val <= min_val) {
        std::cerr << "[UYARI] Gorsel tekduze (duz renk), germe yapilamaz." << std::endl;
        return 0;
    }

    // 3. g = (f - min) * (255.0 / (max - min))
    cv::cuda::GpuMat d_float, d_sub;
    d_src.convertTo(d_float, CV_32FC1);
    cv::cuda::subtract(d_float, cv::Scalar(min_val), d_sub);

    float scale = 255.0f / static_cast<float>(max_val - min_val);
    cv::cuda::GpuMat d_scaled;
    cv::cuda::multiply(d_sub, cv::Scalar(scale), d_scaled);

    d_scaled.convertTo(d_stretched, CV_8UC1);

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    // 4. Host'a Çek ve Doğrula
    cv::Mat h_stretched;
    d_stretched.download(h_stretched);

    double new_min = 0.0, new_max = 0.0;
    cv::cuda::minMaxLoc(d_stretched, &new_min, &new_max, nullptr, nullptr);

    std::cout << "[GERILMIS] Min: " << new_min << " | Max: " << new_max << std::endl;
    std::cout << "[OK] GPU Islem Suresi: " << elapsed << " mikrosaniye" << std::endl;

    cv::imwrite("/content/image2_stretched.png", h_stretched);
    std::cout << "[OK] Sonuc kaydedildi: /content/image2_stretched.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 run_image2.cpp -o run_image2 \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_cudaimgproc -lopencv_cudaarithm \
    -Wl,-rpath,/content/opencv_install/lib

./run_image2
