%%bash
cat << 'EOF' > task_week7_filter_comparison.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 1. SİNÜS DALGASI ÜRETME
void generateSinusoid(std::vector<double>& vec, double amplitude, double frequency, double phase, double sampling_rate = 1000.0) {
    size_t N = vec.size();
    for (size_t i = 0; i < N; ++i) {
        double t = static_cast<double>(i) / sampling_rate;
        vec[i] = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
    }
}

// 2. GÜRÜLTÜ EKLEME
std::vector<double> addNoiseToVector(const std::vector<double>& input_vec, double noise_ratio) {
    noise_ratio = std::clamp(noise_ratio, 0.0, 1.0);
    double max_amplitude = 0.0;
    for (double val : input_vec) {
        if (std::abs(val) > max_amplitude) max_amplitude = std::abs(val);
    }
    if (max_amplitude == 0.0) max_amplitude = 1.0;

    double stddev = max_amplitude * noise_ratio;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, stddev);

    std::vector<double> noisy_vec(input_vec.size());
    for (size_t i = 0; i < input_vec.size(); ++i) {
        noisy_vec[i] = input_vec[i] + dist(gen);
    }
    return noisy_vec;
}

// 3. 1D GAUSS FİLTRESİ
std::vector<double> applyGaussianFilter1D(const std::vector<double>& input_vec, int kernel_size = 5, double sigma = 1.0) {
    size_t N = input_vec.size();
    std::vector<double> filtered(N);
    int half_k = kernel_size / 2;

    // 1D Gauss Çekirdeği (Kernel) Oluşturma
    std::vector<double> kernel(kernel_size);
    double sum = 0.0;
    for (int i = -half_k; i <= half_k; ++i) {
        double val = std::exp(-(i * i) / (2.0 * sigma * sigma));
        kernel[i + half_k] = val;
        sum += val;
    }
    for (int i = 0; i < kernel_size; ++i) kernel[i] /= sum; // Normalize et

    // Konvolüsyon İşlemi
    for (size_t i = 0; i < N; ++i) {
        double current_sum = 0.0;
        for (int k = -half_k; k <= half_k; ++k) {
            int idx = std::clamp(static_cast<int>(i) + k, 0, static_cast<int>(N) - 1); // Sınır koruması
            current_sum += input_vec[idx] * kernel[k + half_k];
        }
        filtered[i] = current_sum;
    }
    return filtered;
}

// 4. 1D MEDYAN FİLTRESİ
std::vector<double> applyMedianFilter1D(const std::vector<double>& input_vec, int window_size = 5) {
    size_t N = input_vec.size();
    std::vector<double> filtered(N);
    int half_w = window_size / 2;

    for (size_t i = 0; i < N; ++i) {
        std::vector<double> window;
        for (int w = -half_w; w <= half_w; ++w) {
            int idx = std::clamp(static_cast<int>(i) + w, 0, static_cast<int>(N) - 1);
            window.push_back(input_vec[idx]);
        }
        std::sort(window.begin(), window.end());
        filtered[i] = window[window.size() / 2]; // Ortanca değeri al
    }
    return filtered;
}

// 5. 4 PANELLİ KARŞILAŞTIRMA GRAFİĞİ GÖRSELLEŞTİRME
void plotFilterComparison(const std::vector<double>& clean, 
                         const std::vector<double>& noisy, 
                         const std::vector<double>& gauss_filtered, 
                         const std::vector<double>& median_filtered, 
                         const std::string& output_path) {
    int width = 1000;
    int height = 800;
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255));

    int margin = 40;
    int plot_w = width - 2 * margin;
    int plot_h = (height - 5 * margin) / 4;

    auto draw_panel = [&](const std::vector<double>& vec, int top_y, const std::string& title, cv::Scalar line_color) {
        int mid_y = top_y + plot_h / 2;

        cv::line(canvas, cv::Point(margin, mid_y), cv::Point(width - margin, mid_y), cv::Scalar(220, 220, 220), 1);
        cv::line(canvas, cv::Point(margin, top_y), cv::Point(margin, top_y + plot_h), cv::Scalar(220, 220, 220), 1);

        double max_val = 0.0;
        for (double v : vec) {
            if (std::abs(v) > max_val) max_val = std::abs(v);
        }
        if (max_val == 0.0) max_val = 1.0;

        size_t N = vec.size();
        for (size_t i = 0; i < N - 1; ++i) {
            int x1 = margin + static_cast<int>((static_cast<double>(i) / N) * plot_w);
            int y1 = mid_y - static_cast<int>((vec[i] / (max_val * 1.2)) * (plot_h / 2));

            int x2 = margin + static_cast<int>((static_cast<double>(i + 1) / N) * plot_w);
            int y2 = mid_y - static_cast<int>((vec[i + 1] / (max_val * 1.2)) * (plot_h / 2));

            cv::line(canvas, cv::Point(x1, y1), cv::Point(x2, y2), line_color, 2, cv::LINE_AA);
        }

        cv::putText(canvas, title, cv::Point(margin, top_y - 8), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 0, 0), 2);
    };

    draw_panel(clean, margin, "1. Orijinal Temiz Sinyal (A=10, f=8Hz)", cv::Scalar(255, 0, 0));
    draw_panel(noisy, margin * 2 + plot_h, "2. Gurultulu Sinyal (Gurultu Orani: 0.30)", cv::Scalar(0, 0, 255));
    draw_panel(gauss_filtered, margin * 3 + plot_h * 2, "3. Gauss Filtresi Uygulanmis Sinyal (Kernel Size=5, Sigma=1.0)", cv::Scalar(0, 150, 0));
    draw_panel(median_filtered, margin * 4 + plot_h * 3, "4. Medyan Filtresi Uygulanmis Sinyal (Window Size=5)", cv::Scalar(255, 100, 0));

    cv::imwrite(output_path, canvas);
    std::cout << "[OK] Filtre karsilastirma grafigi kaydedildi -> " << output_path << std::endl;
}

int main() {
    size_t N = 512; // 2^9 elemanlı vektör
    std::vector<double> clean_signal(N);

    generateSinusoid(clean_signal, 10.0, 8.0, 0.0);

    // Gürültülü Sinyal Oluştur
    double noise_ratio = 0.30;
    std::vector<double> noisy_signal = addNoiseToVector(clean_signal, noise_ratio);

    // Filtreleri Sırasıyla Uygula
    std::vector<double> gauss_result = applyGaussianFilter1D(noisy_signal, 5, 1.0);
    std::vector<double> median_result = applyMedianFilter1D(noisy_signal, 5);

    // Görselleştir
    plotFilterComparison(clean_signal, noisy_signal, gauss_result, median_result, "/content/week7_filter_comparison_plot.png");

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_filter_comparison.cpp -o task_week7_filter_comparison \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_filter_comparison