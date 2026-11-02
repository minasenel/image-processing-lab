%%bash
cat << 'EOF' > task_week7_noisy_sinusoid.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 1. TEMİZ SİNÜSOİDAL DALGA ÜRETME FONKSİYONU
void generateSinusoid(std::vector<double>& vec, double amplitude, double frequency, double phase, double sampling_rate = 1000.0) {
    size_t N = vec.size();
    for (size_t i = 0; i < N; ++i) {
        double t = static_cast<double>(i) / sampling_rate;
        vec[i] = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
    }
}

// 2. SİNYALE GAUSSIAN GÜRÜLTÜ EKLEME FONKSİYONU
// noise_stddev: Gürültünün standart sapması (Gürültü seviyesi arttıkça dalga bozulur)
void addGaussianNoise(std::vector<double>& vec, double noise_stddev = 1.5) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, noise_stddev); // Ortalama = 0, StdDev = noise_stddev

    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] += dist(gen); // Sinyale rastgele gürültü ekle
    }
}

// 3. İKİ SİNYALİ ÜST ÜSTE VEYA AYRI PANELDE GÖRSELLEŞTİRME
void plotSignals(const std::vector<double>& clean_vec, const std::vector<double>& noisy_vec, const std::string& output_path) {
    int width = 900;
    int height = 500;
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255));

    int margin = 50;
    int plot_w = width - 2 * margin;
    int plot_h = (height - 3 * margin) / 2; // İki ayrı panel için yükseklik

    auto draw_panel = [&](const std::vector<double>& vec, int top_y, const std::string& title, cv::Scalar line_color) {
        int mid_y = top_y + plot_h / 2;

        // Eksen
        cv::line(canvas, cv::Point(margin, mid_y), cv::Point(width - margin, mid_y), cv::Scalar(200, 200, 200), 1);
        cv::line(canvas, cv::Point(margin, top_y), cv::Point(margin, top_y + plot_h), cv::Scalar(200, 200, 200), 1);

        // Ölçeklendirme
        double max_val = 8.0; // Sabit eksen ölçeği
        size_t N = vec.size();

        for (size_t i = 0; i < N - 1; ++i) {
            int x1 = margin + static_cast<int>((static_cast<double>(i) / N) * plot_w);
            int y1 = mid_y - static_cast<int>((vec[i] / max_val) * (plot_h / 2));

            int x2 = margin + static_cast<int>((static_cast<double>(i + 1) / N) * plot_w);
            int y2 = mid_y - static_cast<int>((vec[i + 1] / max_val) * (plot_h / 2));

            cv::line(canvas, cv::Point(x1, y1), cv::Point(x2, y2), line_color, 2, cv::LINE_AA);
        }

        cv::putText(canvas, title, cv::Point(margin, top_y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
    };

    // Temiz Sinyal Çizimi (Üst Panel - Mavi)
    draw_panel(clean_vec, margin, "1. Temiz Sinusoidal Dalga (N=" + std::to_string(clean_vec.size()) + ")", cv::Scalar(255, 0, 0));

    // Gürültülü Sinyal Çizimi (Alt Panel - Kırmızı)
    draw_panel(noisy_vec, margin * 2 + plot_h, "2. Gaussian Gurultulu Sinusoidal Dalga (AWGN)", cv::Scalar(0, 0, 255));

    cv::imwrite(output_path, canvas);
    std::cout << "[OK] Karşılaştırmalı sinyal görseli kaydedildi -> " << output_path << std::endl;
}

int main() {
    size_t N = 512; // 2^9 elemanlı vektör
    std::vector<double> clean_signal(N);

    // Parametreler
    double amplitude = 5.0;            // Genlik (A)
    double frequency = 10.0;           // Frekans (Hz)
    double phase = M_PI / 4.0;         // Faz (45 derece)
    double sampling_rate = 1000.0;     // Örnekleme Frekansı

    // 1. Temiz Sinüs Vektörünü Doldur
    generateSinusoid(clean_signal, amplitude, frequency, phase, sampling_rate);

    // 2. Gürültülü Vektörü Oluştur
    std::vector<double> noisy_signal = clean_signal; // Kopyasını al
    addGaussianNoise(noisy_signal, 1.8);             // 1.8 şiddetinde Gaussian gürültü ekle

    // 3. İki Sinyali Yan Yana/Alt Alta Görselleştir
    plotSignals(clean_signal, noisy_signal, "/content/week7_noisy_sinusoid_plot.png");

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_noisy_sinusoid.cpp -o task_week7_noisy_sinusoid \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_noisy_sinusoid