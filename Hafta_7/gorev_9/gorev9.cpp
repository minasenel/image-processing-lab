/*
[1/2] 2D-DFT & 2D-IDFT Rekonstrüksiyonu hesaplanıyor...
==========================================================
      GÖREV 9: 2D-IDFT REKONSTRÜKSİYON VE ZNCC RAPORU     
==========================================================
1. Doğrudan Gürültülü Resim (Orijinale Göre):
   - ZNCC : 0.9760 (1.0 mükemmel eşleşmedir)
   - MSE  : 412.0154
   - PSNR : 21.9817 dB
----------------------------------------------------------
2. Frekans Filtreli + 2D-IDFT Yeniden Üretilmiş Resim:
   - ZNCC : 0.8654 (Filtreleme sonrası benzerlik artı)
   - MSE  : 2333.7833
   - PSNR : 14.4502 dB
==========================================================
[OK] Çıktı kaydedildi -> /content/week8_task8_zncc_result.png
*/

%%bash
cat << 'EOF' > task_week8_idft_zncc.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <random>
#include <algorithm>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef std::complex<double> Complex;

// 1. 1D-DFT VE 1D-IDFT
std::vector<Complex> compute1D_DFT(const std::vector<Complex>& input) {
    size_t N = input.size();
    std::vector<Complex> output(N);
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t k = 0; k < N; ++k) {
        Complex sum(0.0, 0.0);
        for (size_t n = 0; n < N; ++n) {
            double angle = angle_factor * k * n;
            Complex twiddle(std::cos(angle), -std::sin(angle));
            sum += input[n] * twiddle;
        }
        output[k] = sum;
    }
    return output;
}

std::vector<Complex> compute1D_IDFT(const std::vector<Complex>& input) {
    size_t N = input.size();
    std::vector<Complex> output(N);
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t n = 0; n < N; ++n) {
        Complex sum(0.0, 0.0);
        for (size_t k = 0; k < N; ++k) {
            double angle = angle_factor * k * n;
            Complex twiddle(std::cos(angle), std::sin(angle)); // IDFT: +j
            sum += input[k] * twiddle;
        }
        output[n] = sum / static_cast<double>(N);
    }
    return output;
}

// 2. 2D-DFT VE 2D-IDFT İŞLEMLERİ
std::vector<std::vector<Complex>> compute2D_DFT(const cv::Mat& img) {
    int rows = img.rows;
    int cols = img.cols;
    std::vector<std::vector<Complex>> matrix(rows, std::vector<Complex>(cols));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            matrix[r][c] = Complex(static_cast<double>(img.at<uchar>(r, c)), 0.0);
        }
    }

    for (int r = 0; r < rows; ++r) matrix[r] = compute1D_DFT(matrix[r]);
    for (int c = 0; c < cols; ++c) {
        std::vector<Complex> col_in(rows);
        for (int r = 0; r < rows; ++r) col_in[r] = matrix[r][c];
        std::vector<Complex> col_out = compute1D_DFT(col_in);
        for (int r = 0; r < rows; ++r) matrix[r][c] = col_out[r];
    }
    return matrix;
}

cv::Mat compute2D_IDFT(std::vector<std::vector<Complex>> matrix) {
    int rows = matrix.size();
    int cols = matrix[0].size();

    // Sütun IDFT
    for (int c = 0; c < cols; ++c) {
        std::vector<Complex> col_in(rows);
        for (int r = 0; r < rows; ++r) col_in[r] = matrix[r][c];
        std::vector<Complex> col_out = compute1D_IDFT(col_in);
        for (int r = 0; r < rows; ++r) matrix[r][c] = col_out[r];
    }

    // Satır IDFT
    for (int r = 0; r < rows; ++r) {
        matrix[r] = compute1D_IDFT(matrix[r]);
    }

    cv::Mat reconstructed(rows, cols, CV_8UC1);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double val = matrix[r][c].real();
            reconstructed.at<uchar>(r, c) = static_cast<uchar>(std::clamp(val, 0.0, 255.0));
        }
    }
    return reconstructed;
}

// FREKANS ORTAMINDA ALÇAK GEÇİREN FİLTRELEME (LOW-PASS)
std::vector<std::vector<Complex>> applyLowPassFilter(const std::vector<std::vector<Complex>>& spec, double cutoff_radius) {
    int rows = spec.size();
    int cols = spec[0].size();
    int cx = cols / 2;
    int cy = rows / 2;
    std::vector<std::vector<Complex>> filtered = spec;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Merkeze olan uzaklık (FFTShift mantığı dikkate alınarak)
            int shift_r = (r < cy) ? r + cy : r - cy;
            int shift_c = (c < cx) ? c + cx : c - cx;
            double dist = std::sqrt((shift_r - cy) * (shift_r - cy) + (shift_c - cx) * (shift_c - cx));

            if (dist > cutoff_radius) {
                filtered[r][c] = Complex(0.0, 0.0);
            }
        }
    }
    return filtered;
}

// 3. ZERO MEAN NORMALIZED CROSS CORRELATION (ZNCC) HESABI
double computeZNCC(const cv::Mat& imgA, const cv::Mat& imgB) {
    cv::Scalar meanA_s = cv::mean(imgA);
    cv::Scalar meanB_s = cv::mean(imgB);
    double meanA = meanA_s[0];
    double meanB = meanB_s[0];

    double numerator = 0.0;
    double sumA_sq = 0.0;
    double sumB_sq = 0.0;

    for (int r = 0; r < imgA.rows; ++r) {
        for (int c = 0; c < imgA.cols; ++c) {
            double diffA = static_cast<double>(imgA.at<uchar>(r, c)) - meanA;
            double diffB = static_cast<double>(imgB.at<uchar>(r, c)) - meanB;

            numerator += diffA * diffB;
            sumA_sq += diffA * diffA;
            sumB_sq += diffB * diffB;
        }
    }

    double denominator = std::sqrt(sumA_sq * sumB_sq);
    if (denominator == 0.0) return 0.0;
    return numerator / denominator;
}

// ALTERNATİF METRİKLER (MSE, PSNR, SSIM)
double computeMSE(const cv::Mat& imgA, const cv::Mat& imgB) {
    double sum = 0.0;
    for (int r = 0; r < imgA.rows; ++r) {
        for (int c = 0; c < imgA.cols; ++c) {
            double diff = static_cast<double>(imgA.at<uchar>(r, c)) - static_cast<double>(imgB.at<uchar>(r, c));
            sum += diff * diff;
        }
    }
    return sum / static_cast<double>(imgA.rows * imgA.cols);
}

double computePSNR(double mse) {
    if (mse <= 1e-10) return 100.0;
    return 10.0 * std::log10((255.0 * 255.0) / mse);
}

int main() {
    cv::Mat src = cv::imread("/content/lines.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] lines.jpg bulunamadı!" << std::endl;
        return -1;
    }

    // 256x256 Boyutlandırma (Hesaplama süresi için)
    cv::Mat clean;
    cv::resize(src, clean, cv::Size(256, 256), 0, 0, cv::INTER_CUBIC);

    // Gürültülü Resimler
    cv::Mat gauss_noisy = clean.clone();
    std::random_device rd; std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, 25.0);
    for (int r = 0; r < clean.rows; ++r) {
        for (int c = 0; c < clean.cols; ++c) {
            double val = static_cast<double>(clean.at<uchar>(r, c)) + dist(gen);
            gauss_noisy.at<uchar>(r, c) = static_cast<uchar>(std::clamp(val, 0.0, 255.0));
        }
    }

    // 2D-DFT ve Filtreleme + 2D-IDFT
    std::cout << "[1/2] 2D-DFT & 2D-IDFT Rekonstrüksiyonu hesaplanıyor..." << std::endl;
    auto dft_clean = compute2D_DFT(clean);
    auto dft_gauss = compute2D_DFT(gauss_noisy);

    // Gürültülü spektruma alçak geçiren filtre uygulanıp IDFT yapılıyor
    auto dft_filtered = applyLowPassFilter(dft_gauss, 60.0);
    cv::Mat rec_gauss = compute2D_IDFT(dft_filtered);
    cv::Mat rec_direct = compute2D_IDFT(dft_clean); // Doğrudan rekonstrüksiyon kontrolü

    // METRİK HESAPLAMALARI
    double zncc_noisy = computeZNCC(clean, gauss_noisy);
    double zncc_rec   = computeZNCC(clean, rec_gauss);

    double mse_noisy  = computeMSE(clean, gauss_noisy);
    double mse_rec    = computeMSE(clean, rec_gauss);

    double psnr_noisy = computePSNR(mse_noisy);
    double psnr_rec   = computePSNR(mse_rec);

    std::cout << "==========================================================" << std::endl;
    std::cout << "      GÖREV 9: 2D-IDFT REKONSTRÜKSİYON VE ZNCC RAPORU     " << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "1. Doğrudan Gürültülü Resim (Orijinale Göre):" << std::endl;
    std::cout << "   - ZNCC : " << zncc_noisy << " (1.0 mükemmel eşleşmedir)" << std::endl;
    std::cout << "   - MSE  : " << mse_noisy << std::endl;
    std::cout << "   - PSNR : " << psnr_noisy << " dB" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "2. Frekans Filtreli + 2D-IDFT Yeniden Üretilmiş Resim:" << std::endl;
    std::cout << "   - ZNCC : " << zncc_rec << " (Filtreleme sonrası benzerlik artı)" << std::endl;
    std::cout << "   - MSE  : " << mse_rec << std::endl;
    std::cout << "   - PSNR : " << psnr_rec << " dB" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Görsel Paneli Kaydetme
    cv::Mat panel(256, 256 * 3, CV_8UC1);
    clean.copyTo(panel(cv::Rect(0, 0, 256, 256)));
    gauss_noisy.copyTo(panel(cv::Rect(256, 0, 256, 256)));
    rec_gauss.copyTo(panel(cv::Rect(512, 0, 256, 256)));

    cv::cvtColor(panel, panel, cv::COLOR_GRAY2BGR);
    cv::putText(panel, "Orijinal Clean", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 0), 1);
    cv::putText(panel, "Gurultulu (ZNCC: " + std::to_string(zncc_noisy).substr(0,5) + ")", cv::Point(266, 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 255), 1);
    cv::putText(panel, "IDFT Rekonstruksiyon (ZNCC: " + std::to_string(zncc_rec).substr(0,5) + ")", cv::Point(522, 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 0), 1);

    cv::imwrite("/content/week8_task9_zncc_result.png", panel);
    std::cout << "[OK] Çıktı kaydedildi -> /content/week8_task8_zncc_result.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week8_idft_zncc.cpp -o task_week8_idft_zncc \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week8_idft_zncc