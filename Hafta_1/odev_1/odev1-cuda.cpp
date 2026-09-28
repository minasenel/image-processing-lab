%%bash
# Gerekli klasorleri hazirla ve 3 adet test gorseli indir
mkdir -p input_images output_images_gpu
curl -s -o input_images/img1.jpg "https://upload.wikimedia.org/wikipedia/commons/4/47/PNG_transparency_demonstration_1.png"
curl -s -o input_images/img2.jpg "https://images.unsplash.com/photo-1579783902614-a3fb3927b675?w=1600"
curl -s -o input_images/img3.jpg "https://images.unsplash.com/photo-1541701494587-cb58502866ab?w=1600"

cat << 'EOF' > resize_cuda.cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>

namespace fs = std::filesystem;

int main() {
    std::string input_dir = "input_images";
    std::string output_dir = "output_images_gpu";

    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") {
            files.push_back(entry.path().string());
        }
    }

    if (files.empty()) {
        std::cout << "Gorsel bulunamadi!" << std::endl;
        return 1;
    }

    cv::Size target_size(1024, 768);
    cv::cuda::GpuMat d_src, d_dst;
    cv::Mat h_dst;

    // GPU Isinma (Warm-up - ilk CUDA cagrisinin ek yukunu eler)
    cv::cuda::GpuMat dummy(100, 100, CV_8UC3);
    cv::cuda::GpuMat dummy_out;
    cv::cuda::resize(dummy, dummy_out, target_size);

    std::cout << "\n================ [COLAB CUDA RESIZE TESTI] ================\n";
    std::cout << std::left << std::setw(20) << "Dosya"
              << std::setw(15) << "Aktarim (ms)"
              << std::setw(15) << "Kernel (ms)"
              << std::setw(15) << "Toplam GPU (ms)" << "\n";
    std::cout << "-----------------------------------------------------------\n";

    double total_kernel_time = 0.0;
    double total_gpu_time = 0.0;

    for (const auto& path : files) {
        cv::Mat h_src = cv::imread(path);
        if (h_src.empty()) continue;

        auto t_start = std::chrono::high_resolution_clock::now();

        // 1. Host -> Device (VRAM)
        d_src.upload(h_src);

        // 2. CUDA Resize Kernel
        auto t_k1 = std::chrono::high_resolution_clock::now();
        cv::cuda::resize(d_src, d_dst, target_size);
        auto t_k2 = std::chrono::high_resolution_clock::now();

        // 3. Device -> Host (RAM)
        d_dst.download(h_dst);

        auto t_end = std::chrono::high_resolution_clock::now();

        double kernel_ms = std::chrono::duration<double, std::milli>(t_k2 - t_k1).count();
        double full_gpu_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        double transfer_ms = full_gpu_ms - kernel_ms;

        total_kernel_time += kernel_ms;
        total_gpu_time += full_gpu_ms;

        fs::path p(path);
        cv::imwrite(output_dir + "/" + p.filename().string(), h_dst);

        std::cout << std::left << std::setw(20) << p.filename().string()
                  << std::setw(15) << std::fixed << std::setprecision(2) << transfer_ms
                  << std::setw(15) << kernel_ms
                  << std::setw(15) << full_gpu_ms << "\n";
    }

    std::cout << "-----------------------------------------------------------\n";
    std::cout << "Ortalama Saf Kernel Suresi : " << (total_kernel_time / files.size()) << " ms\n";
    std::cout << "Ortalama Uctan Uca Sure    : " << (total_gpu_time / files.size()) << " ms\n\n";

    return 0;
}
EOF

# Derle ve Calistir
g++ -std=c++17 resize_cuda.cpp -o resize_cuda \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_cudawarping -lopencv_cudaimgproc \
    -Wl,-rpath,/content/opencv_install/lib

./resize_cuda