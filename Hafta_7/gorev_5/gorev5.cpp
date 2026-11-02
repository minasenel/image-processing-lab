%%bash
cat << 'EOF' > task_week7_peak_dft.cpp
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

struct PeakInfo {
    int index;          // Frekans Bin İndeksi (k)
    double frequency;   // Karşılık gelen frekans (Hz)
    double amplitude;   // Genlik (A)
};

// 1. SİNÜSOİDAL SİNYAL ÜRETİMİ
void generateSinusoid(std::vector<double>& vec, double amplitude, double frequency, double phase = 0.0, double sampling_rate = 512.0) {
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

// 3. DISCRETE FOURIER TRANSFORM (DFT)
std::vector<double> computeDFT(const std::vector<double>& time_signal) {
    size_t N = time_signal.size();
    std::vector<double> harmonics(N / 2);
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t k = 0; k < N / 2; ++k) {
        double real_part = 0.0;
        double imag_part = 0.0;

        for (size_t n = 0; n < N; ++n) {
            double angle = angle_factor * k * n;
            real_part += time_signal[n] * std::cos(angle);
            imag_part -= time_signal[n] * std::sin(angle);
        }

        double magnitude = std::sqrt(real_part * real_part + imag_part * imag_part);

        if (k == 0) {
            harmonics[k] = magnitude / static_cast<double>(N);
        } else {
            harmonics[k] = (2.0 * magnitude) / static_cast<double>(N);
        }
    }

    return harmonics;
}

// 4. SPEKTRUM ÜZERİNDEKİ PİKLERİ (LOCAL MAXIMA) TESPİT ETME
std::vector<PeakInfo> findSpectrumPeaks(const std::vector<double>& harmonics, double sampling_rate, size_t N, double threshold_ratio = 0.25) {
    std::vector<PeakInfo> peaks;
    double max_val = *std::max_element(harmonics.begin(), harmonics.end());
    double threshold = max_val * threshold_ratio; // Maksimum genliğin %25'inin üzerindeki tepe noktaları

    double delta_f = sampling_rate / static_cast<double>(N); // Frekans adım çözünürlüğü

    for (size_t k = 1; k < harmonics.size() - 1; ++k) {
        // Yerel maksimum kontrolü (Solundaki ve sağındaki komşudan büyük ve eşik üstünde olma şartı)
        if (harmonics[k] > harmonics[k - 1] && harmonics[k] > harmonics[k + 1] && harmonics[k] >= threshold) {
            double freq = k * delta_f;
            peaks.push_back({static_cast<int>(k), freq, harmonics[k]});
        }
    }

    return peaks;
}

// 5. GÖRSELLEŞTİRME VE PİK İNDEKSLERİNİ EKRANA/GÖRSELE YAZDIRMA
void plotCompositeWithPeaks(const std::vector<double>& sum_signal, 
                           const std::vector<double>& sum_harmonics, 
                           const std::vector<PeakInfo>& peaks,
                           const std::string& output_path) {
    int width = 1000;
    int height = 650;
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255));

    int margin = 50;
    int plot_w = width - 2 * margin;
    int plot_h = (height - 3 * margin) / 2;

    // A) Birleşik Sinyal (Zaman)
    int mid_y1 = margin + plot_h / 2;
    cv::line(canvas, cv::Point(margin, mid_y1), cv::Point(width - margin, mid_y1), cv::Scalar(220, 220, 220), 1);
    cv::line(canvas, cv::Point(margin, margin), cv::Point(margin, margin + plot_h), cv::Scalar(220, 220, 220), 1);

    double max_sig = 0.0;
    for (double v : sum_signal) if (std::abs(v) > max_sig) max_sig = std::abs(v);
    if (max_sig == 0.0) max_sig = 1.0;

    size_t N = sum_signal.size();
    for (size_t i = 0; i < N - 1; ++i) {
        int x1 = margin + static_cast<int>((static_cast<double>(i) / N) * plot_w);
        int y1 = mid_y1 - static_cast<int>((sum_signal[i] / (max_sig * 1.2)) * (plot_h / 2));

        int x2 = margin + static_cast<int>((static_cast<double>(i + 1) / N) * plot_w);
        int y2 = mid_y1 - static_cast<int>((sum_signal[i + 1] / (max_sig * 1.2)) * (plot_h / 2));

        cv::line(canvas, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 100, 0), 2, cv::LINE_AA);
    }
    cv::putText(canvas, "1. Birlesik Zaman Sinyali (30 Hz + 50 Hz + AWGN)", cv::Point(margin, margin - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);

    // B) DFT Spektrumu ve Pik Etiketleri
    int top_y2 = margin * 2 + plot_h;
    int base_y2 = top_y2 + plot_h;

    cv::line(canvas, cv::Point(margin, base_y2), cv::Point(width - margin, base_y2), cv::Scalar(220, 220, 220), 1);
    cv::line(canvas, cv::Point(margin, top_y2), cv::Point(margin, base_y2), cv::Scalar(220, 220, 220), 1);

    double max_harm = 0.0;
    for (double v : sum_harmonics) if (v > max_harm) max_harm = v;
    if (max_harm == 0.0) max_harm = 1.0;

    size_t K = sum_harmonics.size();
    for (size_t k = 0; k < K; ++k) {
        int x = margin + static_cast<int>((static_cast<double>(k) / K) * plot_w);
        int bar_h = static_cast<int>((sum_harmonics[k] / (max_harm * 1.2)) * plot_h);
        int y = base_y2 - bar_h;

        cv::line(canvas, cv::Point(x, base_y2), cv::Point(x, y), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // PİKLERİ GRAFİK ÜZERİNE İŞLEME (Kırmızı Nokta ve Metin)
    for (const auto& p : peaks) {
        int px = margin + static_cast<int>((static_cast<double>(p.index) / K) * plot_w);
        int bar_h = static_cast<int>((p.amplitude / (max_harm * 1.2)) * plot_h);
        int py = base_y2 - bar_h;

        // Pik Noktasına Kırmızı Daire
        cv::circle(canvas, cv::Point(px, py), 5, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);

        // Pik Bilgisi Metni (İndex, Frekans, Genlik)
        std::string label = "k=" + std::to_string(p.index) + " (" + std::to_string(static_cast<int>(p.frequency)) + "Hz)";
        cv::putText(canvas, label, cv::Point(px - 25, py - 12), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 100, 0), 2);
    }

    cv::putText(canvas, "2. DFT Harmonik Spektrumu ve Tespit Edilen Pik Indeksleri", cv::Point(margin, top_y2 - 10), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);

    cv::imwrite(output_path, canvas);
    std::cout << "[OK] Pik etiketli grafik kaydedildi -> " << output_path << std::endl;
}

int main() {
    size_t N = 256;
    double sampling_rate = 512.0;

    // Sinyal 1: f = 30 Hz, A = 5
    std::vector<double> signal1(N);
    generateSinusoid(signal1, 5.0, 30.0, 0.0, sampling_rate);
    std::vector<double> noisy1 = addNoiseToVector(signal1, 0.20);

    // Sinyal 2: f = 50 Hz, A = 12
    std::vector<double> signal2(N);
    generateSinusoid(signal2, 12.0, 50.0, 0.0, sampling_rate);
    std::vector<double> noisy2 = addNoiseToVector(signal2, 0.20);

    // İki Sinyalin Toplamı
    std::vector<double> sum_signal(N);
    for (size_t i = 0; i < N; ++i) {
        sum_signal[i] = noisy1[i] + noisy2[i];
    }

    // Birleşik Sinyal DFT Analizi
    std::vector<double> sum_harmonics = computeDFT(sum_signal);

    // SPEKTRUM PİKLERİNİ HESAPLAMA
    std::vector<PeakInfo> peaks = findSpectrumPeaks(sum_harmonics, sampling_rate, N);

    // KONSOL ÇIKTISI
    std::cout << "==================================================" << std::endl;
    std::cout << "     DFT HARMONİK SPEKTRUMU PİK RAPORU            " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Örnek Sayısı (N)           : " << N << std::endl;
    std::cout << "Örnekleme Frekansı (Fs)    : " << sampling_rate << " Hz" << std::endl;
    std::cout << "Frekans Adımı (Delta f)    : " << (sampling_rate / N) << " Hz/bin" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Tespit Edilen Pik Sayısı  : " << peaks.size() << std::endl;
    for (size_t i = 0; i < peaks.size(); ++i) {
        std::cout << i + 1 << ". Pik -> Bin Indeksi (k): " << peaks[i].index 
                  << " | Frekans: " << peaks[i].frequency << " Hz"
                  << " | Genlik (A): " << peaks[i].amplitude << std::endl;
    }
    std::cout << "==================================================" << std::endl;

    // GRAFİK ÇİZİMİ
    plotCompositeWithPeaks(sum_signal, sum_harmonics, peaks, "/content/week7_dft_peaks_plot.png");

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_peak_dft.cpp -o task_week7_peak_dft \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_peak_dft