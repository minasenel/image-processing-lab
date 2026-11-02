///Hafta_7/gorev_4/week7_dft_spectrum_plot.png  -> 12 HZ, genlik = 10 -> sızma gözlemledim 
//tam tamsayı katına denk gelmediği için sinyal pencerelenirken periyot tam bölünmez ve komşu frekans kutularına ($bins$) sızma yapar.
//Hafta_7/gorev_4/week7_dft_16hz_spectrum_plot.png  -> 16 hz ile yaptım.FFT/DFT bin adımıyla tam örtüşeceği için sızma (leakage) ortadan kalkar ve spikelar tamamen keskinleşir.


%%bash
cat << 'EOF' > task_week7_dft.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <random>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 1. SİNÜSOİDAL SİNYAL ÜRETİMİ
void generateSinusoid(std::vector<double>& vec, double amplitude, double frequency, double phase, double sampling_rate = 1000.0) {
    size_t N = vec.size();
    for (size_t i = 0; i < N; ++i) {
        double t = static_cast<double>(i) / sampling_rate;
        vec[i] = amplitude * std::sin(2.0 * M_PI * frequency * t + phase);
    }
}

// 2. GÜRÜLTÜ EKLEME FONKSİYONU
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

// 3. İSTENEN FONKSİYON: DISCRETE FOURIER TRANSFORM (DFT) VE HARMONİK VEKTÖRÜ ÜRETİMİ
// Girdi  : time_signal (Zaman uzayındaki N elemanlı sinyal)
// Çıktı : Harmonik (Genlik Spektrumu) Vektörü -> |X[k]|
std::vector<double> computeDFT(const std::vector<double>& time_signal) {
    size_t N = time_signal.size();
    std::vector<double> harmonics(N / 2); // Nyquist teoremi gereği N/2 frekans bileşeni yeterlidir

    // Trigonometrik hesaplamaları hızlandırmak için Euler açısı çarpanı
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t k = 0; k < N / 2; ++k) {
        double real_part = 0.0;
        double imag_part = 0.0;

        for (size_t n = 0; n < N; ++n) {
            double angle = angle_factor * k * n;
            real_part += time_signal[n] * std::cos(angle);
            imag_part -= time_signal[n] * std::sin(angle); // Euler: e^(-j*theta) = cos(theta) - j*sin(theta)
        }

        // Karmaşık sayının genliği (Magnitude / Harmonik Büyüklüğü)
        double magnitude = std::sqrt(real_part * real_part + imag_part * imag_part);

        // Doğru genlik ölçeklemesi (k=0 DC bileşen hariç 2 ile çarpılır)
        if (k == 0) {
            harmonics[k] = magnitude / static_cast<double>(N);
        } else {
            harmonics[k] = (2.0 * magnitude) / static_cast<double>(N);
        }
    }

    return harmonics;
}

// 4. KARŞILAŞTIRMALI SPEKTRUM GÖRSELLEŞTİRME
void plotDFTSpectrums(const std::vector<double>& clean_harmonics, 
                      const std::vector<double>& noisy_harmonics, 
                      const std::string& output_path) {
    int width = 900;
    int height = 500;
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255));

    int margin = 50;
    int plot_w = width - 2 * margin;
    int plot_h = (height - 3 * margin) / 2;

    auto draw_spectrum = [&](const std::vector<double>& harm, int top_y, const std::string& title, cv::Scalar color) {
        int base_y = top_y + plot_h;

        // Eksenler
        cv::line(canvas, cv::Point(margin, base_y), cv::Point(width - margin, base_y), cv::Scalar(200, 200, 200), 1);
        cv::line(canvas, cv::Point(margin, top_y), cv::Point(margin, base_y), cv::Scalar(200, 200, 200), 1);

        double max_val = 0.0;
        for (double v : harm) if (v > max_val) max_val = v;
        if (max_val == 0.0) max_val = 1.0;

        size_t K = harm.size();
        for (size_t k = 0; k < K; ++k) {
            int x = margin + static_cast<int>((static_cast<double>(k) / K) * plot_w);
            int bar_h = static_cast<int>((harm[k] / (max_val * 1.1)) * plot_h);
            int y = base_y - bar_h;

            // Frekans çizgileri (Stem plot formatında)
            cv::line(canvas, cv::Point(x, base_y), cv::Point(x, y), color, 2, cv::LINE_AA);
        }

        cv::putText(canvas, title, cv::Point(margin, top_y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
    };

    // Gürültüsüz Harmonik Spektrumu (Mavi)
    draw_spectrum(clean_harmonics, margin, "1. Gurultusuz Sinyal DFT Harmonik Spektrumu", cv::Scalar(255, 0, 0));

    // Gürültülü Harmonik Spektrumu (Kırmızı)
    draw_spectrum(noisy_harmonics, margin * 2 + plot_h, "2. Gurultulu Sinyal DFT Harmonik Spektrumu (AWGN)", cv::Scalar(0, 0, 255));

    cv::imwrite(output_path, canvas);
    std::cout << "[OK] DFT Spektrum grafigi kaydedildi -> " << output_path << std::endl;
}

int main() {
    size_t N = 256; // DFT O(N^2) karmaşıklığında olduğu için N=256 seçildi
    std::vector<double> clean_signal(N);

    // Parametreler: Genlik=10, Frekans=12 Hz, Örnekleme=500 Hz
    double amplitude = 10.0;
    double frequency = 12.0;
    generateSinusoid(clean_signal, amplitude, frequency, 0.0, 500.0);

    // Gürültülü sinyal oluştur (%30 Gürültü Oranı)
    std::vector<double> noisy_signal = addNoiseToVector(clean_signal, 0.30);

    // 1. Gürültüsüz Sinyali DFT'ye Gönder
    std::vector<double> clean_harmonics = computeDFT(clean_signal);

    // 2. Gürültülü Sinyali DFT'ye Gönder
    std::vector<double> noisy_harmonics = computeDFT(noisy_signal);

    // Grafiği Oluştur
    plotDFTSpectrums(clean_harmonics, noisy_harmonics, "/content/week7_dft_spectrum_plot.png");

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_dft.cpp -o task_week7_dft \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_dft