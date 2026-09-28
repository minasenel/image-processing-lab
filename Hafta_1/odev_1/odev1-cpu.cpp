#include <iostream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace fs = std::filesystem;

int main() {
    std::string input_dir = "input_images";
    std::string output_dir = "output_images_cpu";

    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") {
            files.push_back(entry.path().string());
        }
    }

    if (files.empty()) {
        std::cout << "input_images klasorunde gorsel bulunamadi!" << std::endl;
        return 1;
    }

    cv::Size target_size(1024, 768);
    cv::Mat dst;

    std::cout << "\n================ [YEREL CPU RESIZE TESTI] ================\n";
    std::cout << std::left << std::setw(20) << "Dosya" 
              << std::setw(18) << "Islem Suresi (ms)" << "\n";
    std::cout << "-----------------------------------------------------------\n";

    double total_time = 0.0;

    for (const auto& path : files) {
        cv::Mat src = cv::imread(path);
        if (src.empty()) continue;

        auto t1 = std::chrono::high_resolution_clock::now();
        cv::resize(src, dst, target_size);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        total_time += ms;

        fs::path p(path);
        cv::imwrite(output_dir + "/" + p.filename().string(), dst);

        std::cout << std::left << std::setw(20) << p.filename().string()
                  << std::setw(18) << std::fixed << std::setprecision(2) << ms << "\n";
    }

    std::cout << "-----------------------------------------------------------\n";
    std::cout << "Ortalama CPU Suresi: " << (total_time / files.size()) << " ms\n\n";

    return 0;
}