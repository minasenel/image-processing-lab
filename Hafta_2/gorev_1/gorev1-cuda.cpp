%%bash
cat << 'EOF' > process_cuda.cpp
#include <iostream>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>

namespace fs = std::filesystem;

int main() {
    std::string input_file = "resim.jpeg"; 
    std::string out_dir = "output_results";

    if (!fs::exists(out_dir)) fs::create_directories(out_dir);

    cv::Mat h_src = cv::imread(input_file);
    if (h_src.empty()) {
        std::cerr << "Hata: '" << input_file << "' yuklenemedi! Dosya adini ve sol paneldeki yerini kontrol edin." << std::endl;
        return 1;
    }

    // MacBook Pro 14" taban olcekli cozunurlugu (1512 x 982)
    int base_w = 1512;
    int base_h = 982;

    cv::Size half_size(base_w / 2, base_h / 2);   // 756 x 491 (0.5x)
    cv::Size double_size(base_w * 2, base_h * 2); // 3024 x 1964 (2.0x)

    // GPU Matrisleri
    cv::cuda::GpuMat d_bgr, d_gray, d_half, d_double;

    // 1. Host -> Device (VRAM)
    d_bgr.upload(h_src);

    // 2. Grayscale Donusumu (CUDA)
    cv::cuda::cvtColor(d_bgr, d_gray, cv::COLOR_BGR2GRAY);

    // 3. Boyutlandirma (CUDA)
    cv::cuda::resize(d_gray, d_half, half_size);
    cv::cuda::resize(d_gray, d_double, double_size);

    // 4. Device -> Host (RAM)
    cv::Mat h_half, h_double;
    d_half.download(h_half);
    d_double.download(h_double);

    // 5. Sonuclari kaydet
    cv::imwrite(out_dir + "/gray_half.jpg", h_half);
    cv::imwrite(out_dir + "/gray_double.jpg", h_double);

    std::cout << "Islem basariyla tamamlandi!" << std::endl;
    std::cout << "- 0.5x boyut: " << half_size.width << "x" << half_size.height << " -> " << out_dir << "/gray_half.jpg" << std::endl;
    std::cout << "- 2.0x boyut: " << double_size.width << "x" << double_size.height << " -> " << out_dir << "/gray_double.jpg" << std::endl;

    return 0;
}
EOF

# Derle ve Calistir
g++ -std=c++17 process_cuda.cpp -o process_cuda \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_cudaimgproc -lopencv_cudawarping \
    -Wl,-rpath,/content/opencv_install/lib

./process_cuda

# Ciktilari zip dosyasinda topla
zip -j -o processed_images.zip output_results/*

