/*
amaç: sanki hiç gürültü eklememişiz gibi eski görütntüyü geri kazanacağız. 
computeIDFT() (Ters Ayrık Fourier Dönüşümü) fonksiyonu yazıldı. Frekans ortamından zaman ortamına geri dönülerek gürültüsüz sinyal elde edildi.
applySpectralThreshold() fonksiyonu yazıldı. Belirlenen eşiğin (threshold = 2.5) altındaki tüm gürültü frekansları ($X[k] = 0$) olarak sıfırlandı.

[OK] Kaydedildi -> /content/week7_task6_signal1_idft.png
[OK] Kaydedildi -> /content/week7_task6_signal2_idft.png
[OK] Kaydedildi -> /content/week7_task6_composite_idft.png
==================================================
  GÖREV 6: ESİKLEME & IDFT İLE SİNYAL REKONSTRÜKSİYONU  
==================================================
Belirlenen Eşik Değeri (Threshold) : 2.5
[1/3] Sinyal 1 (30 Hz) temizlendi ve IDFT yapildi.
[2/3] Sinyal 2 (50 Hz) temizlendi ve IDFT yapildi.
[3/3] Birleşik Sinyal (30+50 Hz) temizlendi ve IDFT yapildi.

*/


%%bash
cat << 'EOF' > task_week7_idft_threshold.cpp
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

// Karmaşık Sayı Yapısı (Complex Number)
struct Complex {
    double real;
    double imag;

    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
    Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    Complex operator*(const Complex& other) const {
        return Complex(real * other.real - imag * other.imag, real * other.imag + imag * other.real);
    }
    double magnitude() const {
        return std::sqrt(real * real + imag * imag);
    }
};

// 1. SİNÜSOİDAL SİNYAL ÜRETİMİ
void generateSinusoid(std::vector<double>& vec, double amplitude, double frequency, double phase = 0.0, double sampling_rate = 512.0) {
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

// 3. FULL DISCRETE FOURIER TRANSFORM (DFT) - KARMAŞIK SONUÇ
std::vector<Complex> computeFullDFT(const std::vector<double>& time_signal) {
    size_t N = time_signal.size();
    std::vector<Complex> X(N);
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t k = 0; k < N; ++k) {
        double real_part = 0.0;
        double imag_part = 0.0;

        for (size_t n = 0; n < N; ++n) {
            double angle = angle_factor * k * n;
            real_part += time_signal[n] * std::cos(angle);
            imag_part -= time_signal[n] * std::sin(angle);
        }
        X[k] = Complex(real_part, imag_part);
    }
    return X;
}

// 4. INVERSE DISCRETE FOURIER TRANSFORM (IDFT) - FREKANSTAN ZAMANA GERİ DÖNÜŞ
std::vector<double> computeIDFT(const std::vector<Complex>& freq_signal) {
    size_t N = freq_signal.size();
    std::vector<double> reconstructed_time_signal(N);
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t n = 0; n < N; ++n) {
        double real_part = 0.0;
        for (size_t k = 0; k < N; ++k) {
            double angle = angle_factor * k * n;
            // IDFT formülü: e^(+j * 2*pi*k*n / N) -> Cosine + j*Sine
            real_part += freq_signal[k].real * std::cos(angle) - freq_signal[k].imag * std::sin(angle);
        }
        reconstructed_time_signal[n] = real_part / static_cast<double>(N); // N'e bölme normalizasyonu
    }
    return reconstructed_time_signal;
}

// 5. FREKANS UZAYINDA EŞİKLEME İLE GÜRÜLTÜ TEMİZLEME (SPECTRAL THRESHOLDING)
std::vector<Complex> applySpectralThreshold(const std::vector<Complex>& X, double threshold_val) {
    size_t N = X.size();
    std::vector<Complex> X_filtered = X;

    for (size_t k = 0; k < N; ++k) {
        // Genlik (Magnitude) hesabı
        double mag = (2.0 * X[k].magnitude()) / static_cast<double>(N);
        if (k == 0) mag /= 2.0;

        // Eşik değerinin (örneğin 2.5) altındaki frekans bileşenlerini sıfırla
        if (mag < threshold_val) {
            X_filtered[k] = Complex(0.0, 0.0);
        }
    }
    return X_filtered;
}

// HARMONİK GENLİK SPEKTRUMU HESAPLAMA (GÖRSELLEŞTİRME İÇİN)
std::vector<double> getHarmonicsFromComplex(const std::vector<Complex>& X) {
    size_t N = X.size();
    std::vector<double> harmonics(N / 2);
    for (size_t k = 0; k < N / 2; ++k) {
        double mag = X[k].magnitude();
        if (k == 0) harmonics[k] = mag / static_cast<double>(N);
        else harmonics[k] = (2.0 * mag) / static_cast<double>(N);
    }
    return harmonics;
}

// 6. GÖRSELLEŞTİRME
void plotTask6Panel(const std::vector<double>& noisy_sig,
                    const std::vector<double>& noisy_harm,
                    const std::vector<double>& filtered_harm,
                    const std::vector<double>& reconstructed_sig,
                    double threshold,
                    const std::string& title_prefix,
                    const std::string& output_path) {
    int width = 1000;
    int height = 800;
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(255, 255, 255));

    int margin = 40;
    int plot_w = width - 2 * margin;
    int plot_h = (height - 5 * margin) / 4;

    auto draw_signal = [&](const std::vector<double>& vec, int top_y, const std::string& title, cv::Scalar color) {
        int mid_y = top_y + plot_h / 2;
        cv::line(canvas, cv::Point(margin, mid_y), cv::Point(width - margin, mid_y), cv::Scalar(220, 220, 220), 1);
        cv::line(canvas, cv::Point(margin, top_y), cv::Point(margin, top_y + plot_h), cv::Scalar(220, 220, 220), 1);

        double max_val = 0.0;
        for (double v : vec) if (std::abs(v) > max_val) max_val = std::abs(v);
        if (max_val == 0.0) max_val = 1.0;

        size_t N = vec.size();
        for (size_t i = 0; i < N - 1; ++i) {
            int x1 = margin + static_cast<int>((static_cast<double>(i) / N) * plot_w);
            int y1 = mid_y - static_cast<int>((vec[i] / (max_val * 1.2)) * (plot_h / 2));

            int x2 = margin + static_cast<int>((static_cast<double>(i + 1) / N) * plot_w);
            int y2 = mid_y - static_cast<int>((vec[i + 1] / (max_val * 1.2)) * (plot_h / 2));

            cv::line(canvas, cv::Point(x1, y1), cv::Point(x2, y2), color, 2, cv::LINE_AA);
        }
        cv::putText(canvas, title, cv::Point(margin, top_y - 8), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 0, 0), 2);
    };

    auto draw_spectrum = [&](const std::vector<double>& harm, int top_y, const std::string& title, cv::Scalar color) {
        int base_y = top_y + plot_h;
        cv::line(canvas, cv::Point(margin, base_y), cv::Point(width - margin, base_y), cv::Scalar(220, 220, 220), 1);
        cv::line(canvas, cv::Point(margin, top_y), cv::Point(margin, base_y), cv::Scalar(220, 220, 220), 1);

        double max_val = 0.0;
        for (double v : harm) if (v > max_val) max_val = v;
        if (max_val == 0.0) max_val = 1.0;

        size_t K = harm.size();
        for (size_t k = 0; k < K; ++k) {
            int x = margin + static_cast<int>((static_cast<double>(k) / K) * plot_w);
            int bar_h = static_cast<int>((harm[k] / (max_val * 1.2)) * plot_h);
            int y = base_y - bar_h;

            cv::line(canvas, cv::Point(x, base_y), cv::Point(x, y), color, 2, cv::LINE_AA);
        }

        // Eşik Çizgisini Kırmızı Kesikli Çizgi Olarak Çiz
        int thresh_y = base_y - static_cast<int>((threshold / (max_val * 1.2)) * plot_h);
        cv::line(canvas, cv::Point(margin, thresh_y), cv::Point(width - margin, thresh_y), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
        cv::putText(canvas, "Esik (" + std::to_string(threshold).substr(0,3) + ")", cv::Point(width - margin - 80, thresh_y - 5), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 255), 1);

        cv::putText(canvas, title, cv::Point(margin, top_y - 8), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 0, 0), 2);
    };

    draw_signal(noisy_sig, margin, "1. " + title_prefix + " - Gurultulu Zaman Sinyali", cv::Scalar(255, 100, 0));
    draw_spectrum(noisy_harm, margin * 2 + plot_h, "2. " + title_prefix + " - DFT Harmonik Spektrumu (Esik Alti Gurultulu)", cv::Scalar(0, 0, 255));
    draw_spectrum(filtered_harm, margin * 3 + plot_h * 2, "3. " + title_prefix + " - Esiklenmis Harmonik Spektrumu (Gurultuden Arindirilmis)", cv::Scalar(0, 150, 0));
    draw_signal(reconstructed_sig, margin * 4 + plot_h * 3, "4. " + title_prefix + " - IDFT ile Geri Elde Edilen Temiz Zaman Sinyali", cv::Scalar(255, 0, 0));

    cv::imwrite(output_path, canvas);
    std::cout << "[OK] Kaydedildi -> " << output_path << std::endl;
}

int main() {
    size_t N = 256;
    double sampling_rate = 512.0;
    double threshold_val = 2.5; // Eşik değer (2-3 arası istenmişti)

    // A) 1. SİNYAL (30 Hz, A=5)
    std::vector<double> s1(N);
    generateSinusoid(s1, 5.0, 30.0, 0.0, sampling_rate);
    std::vector<double> noisy_s1 = addNoiseToVector(s1, 0.25);

    std::vector<Complex> X1 = computeFullDFT(noisy_s1);
    std::vector<Complex> X1_filtered = applySpectralThreshold(X1, threshold_val);
    std::vector<double> rec_s1 = computeIDFT(X1_filtered);

    plotTask6Panel(noisy_s1, getHarmonicsFromComplex(X1), getHarmonicsFromComplex(X1_filtered), rec_s1, threshold_val, "Sinyal 1 (30Hz)", "/content/week7_task6_signal1_idft.png");

    // B) 2. SİNYAL (50 Hz, A=12)
    std::vector<double> s2(N);
    generateSinusoid(s2, 12.0, 50.0, 0.0, sampling_rate);
    std::vector<double> noisy_s2 = addNoiseToVector(s2, 0.25);

    std::vector<Complex> X2 = computeFullDFT(noisy_s2);
    std::vector<Complex> X2_filtered = applySpectralThreshold(X2, threshold_val);
    std::vector<double> rec_s2 = computeIDFT(X2_filtered);

    plotTask6Panel(noisy_s2, getHarmonicsFromComplex(X2), getHarmonicsFromComplex(X2_filtered), rec_s2, threshold_val, "Sinyal 2 (50Hz)", "/content/week7_task6_signal2_idft.png");

    // C) BİNDİRİLMİŞ (TOPLAM) SİNYAL (30Hz + 50Hz)
    std::vector<double> sum_sig(N);
    for (size_t i = 0; i < N; ++i) sum_sig[i] = noisy_s1[i] + noisy_s2[i];

    std::vector<Complex> X_sum = computeFullDFT(sum_sig);
    std::vector<Complex> X_sum_filtered = applySpectralThreshold(X_sum, threshold_val);
    std::vector<double> rec_sum = computeIDFT(X_sum_filtered);

    plotTask6Panel(sum_sig, getHarmonicsFromComplex(X_sum), getHarmonicsFromComplex(X_sum_filtered), rec_sum, threshold_val, "Birlesik Sinyal (30Hz+50Hz)", "/content/week7_task6_composite_idft.png");

    std::cout << "==================================================" << std::endl;
    std::cout << "  GÖREV 6: ESİKLEME & IDFT İLE SİNYAL REKONSTRÜKSİYONU  " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Belirlenen Eşik Değeri (Threshold) : " << threshold_val << std::endl;
    std::cout << "[1/3] Sinyal 1 (30 Hz) temizlendi ve IDFT yapildi." << std::endl;
    std::cout << "[2/3] Sinyal 2 (50 Hz) temizlendi ve IDFT yapildi." << std::endl;
    std::cout << "[3/3] Birleşik Sinyal (30+50 Hz) temizlendi ve IDFT yapildi." << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_idft_threshold.cpp -o task_week7_idft_threshold \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_idft_threshold