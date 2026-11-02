%%bash
cat << 'EOF' > task_week7_noise_function.cpp
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

// 1. SİNÜS DALGASI ÜRETME FONKSİYONU (2'nin kuvveti boyutta)
void generateSinusoid(std::vector<double>& vec, double amplitude, double frequency, double phase, double sampling_rate = 1000.0) {
    size_t N = vec.size();
    for (size_t i = 0; i < N; ++i) {
        double t = static_cast<double>(i) / sampling_rate;
        vec[i] = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
    }
}

// 2. ISTENEN FONKSİYON: GENLİĞE VE GÜRÜLTÜ ORANINA [0, 1] GÖRE GÜRÜLTÜ EKLEME
// Input  : input_vec (Orjinal sinyal), noise_ratio (0.0 - 1.0 aralığında gürültü oranı)
// Output : Gürültü eklenmiş yeni vektör
std::vector<double> addNoiseToVector(const std::vector<double>& input_vec, double noise_ratio) {
    // Gürültü oranını [0.0, 1.0] aralığına sınırla
    noise_ratio = std::clamp(noise_ratio, 0.0, 1.0);

    // Vektördeki maksimum genlik değerimizi (Peak Amplitude) bulma
    double max_amplitude = 0.0;
    for (double val : input_vec) {
        if (std::abs(val) > max_amplitude) {
            max_amplitude = std::abs(val);
        }
    }
    if (max_amplitude == 0.0) max_amplitude = 1.0; // Sıfır bölme koruması

    // Gürültü standart sapmasını genliğe orantılı hesapla
    double stddev = max_amplitude * noise_ratio;

    // Rastgele Gaussian Gürültü Üretici (Sıfır merkezli, stddev sapmalı)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, stddev);

    std::vector<double> noisy_vec(input_vec.size());
    for (size_t i = 0; i < input_vec.size(); ++i) {
        // Her bir elemana genlikle orantılı rastgele gürültü ekleme
        noisy_vec[i] = input_vec[i] + dist(gen);
    }

    return noisy_vec;
}

// 3. GRAFİK OLARAK GÖRSELLEŞTİRME VE KAYDETME FONKSİYONU
void plotSignals(const std::vector<double>& clean_vec, const std::vector<double>& noisy_vec, double noise_ratio, const std::string& output_path) {
    int width = 900;
    int height = 500;
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255)); // Beyaz tuval

    int margin = 50;
    int plot_w = width - 2 * margin;
    int plot_h = (height - 3 * margin) / 2;

    auto draw_panel = [&](const std::vector<double>& vec, int top_y, const std::string& title, cv::Scalar line_color) {
        int mid_y = top_y + plot_h / 2;

        // Eksen Çizimleri
        cv::line(canvas, cv::Point(margin, mid_y), cv::Point(width - margin, mid_y), cv::Scalar(200, 200, 200), 1);
        cv::line(canvas, cv::Point(margin, top_y), cv::Point(margin, top_y + plot_h), cv::Scalar(200, 200, 200), 1);

        // Dinamik Ölçekleme
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

        cv::putText(canvas, title, cv::Point(margin, top_y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
    };

    // Temiz Sinyal Çizimi (Üst Panel)
    draw_panel(clean_vec, margin, "1. Orijinal Sinyal Vektoru (N=" + std::to_string(clean_vec.size()) + ")", cv::Scalar(255, 0, 0));

    // Gürültülü Sinyal Çizimi (Alt Panel)
    std::string noisy_title = "2. Gurultulu Sinyal Vektoru (Gurultu Orani: " + std::to_string(noise_ratio) + ")";
    draw_panel(noisy_vec, margin * 2 + plot_h, noisy_title, cv::Scalar(0, 0, 255));

    cv::imwrite(output_path, canvas);
    std::cout << "[OK] Grafik kaydedildi -> " << output_path << std::endl;
}

int main() {
    size_t N = 512; // 2^9 elemanlı vektör
    std::vector<double> signal_vector(N);

    // Temiz Sinüs Dalgası Üret
    double amplitude = 10.0;
    double frequency = 8.0;
    double phase = 0.0;
    generateSinusoid(signal_vector, amplitude, frequency, phase);

    // Test için Gürültü Oranı seçimi [0.0 - 1.0]
    double noise_ratio = 0.25; // %25 oranında gürültü ekle

    // FONKSİYON ÇAĞRISI
    std::vector<double> noisy_signal_vector = addNoiseToVector(signal_vector, noise_ratio);

    // Grafiği Oluştur ve Kaydet
    plotSignals(signal_vector, noisy_signal_vector, noise_ratio, "/content/week7_noise_function_plot.png");

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_noise_function.cpp -o task_week7_noise_function \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_noise_function