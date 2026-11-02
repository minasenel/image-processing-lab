%%bash
cat << 'EOF' > task_week8_task10_filters.cpp
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
            Complex twiddle(std::cos(angle), std::sin(angle));
            sum += input[k] * twiddle;
        }
        output[n] = sum / static_cast<double>(N);
    }
    return output;
}

// 2. 2D-DFT VE 2D-IDFT
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

    for (int c = 0; c < cols; ++c) {
        std::vector<Complex> col_in(rows);
        for (int r = 0; r < rows; ++r) col_in[r] = matrix[r][c];
        std::vector<Complex> col_out = compute1D_IDFT(col_in);
        for (int r = 0; r < rows; ++r) matrix[r][c] = col_out[r];
    }

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

// 3. SHIFT / IFFTShift İŞLEMİ
void fftShift2D(std::vector<std::vector<Complex>>& matrix) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    int cx = cols / 2;
    int cy = rows / 2;

    for (int r = 0; r < cy; ++r) {
        for (int c = 0; c < cx; ++c) {
            std::swap(matrix[r][c], matrix[r + cy][c + cx]);
            std::swap(matrix[r + cy][c], matrix[r][c + cx]);
        }
    }
}

// 4. FREKANS FİLTRELERİ (GAUSSIAN TYPE)
enum FilterType { LOW_PASS, HIGH_PASS, BAND_PASS, BAND_REJECT };

std::vector<std::vector<Complex>> apply2DFilter(const std::vector<std::vector<Complex>>& spec, 
                                                FilterType type, 
                                                double cutoff1, 
                                                double cutoff2 = 0.0) {
    int rows = spec.size();
    int cols = spec[0].size();
    int cx = cols / 2;
    int cy = rows / 2;

    std::vector<std::vector<Complex>> shifted = spec;
    fftShift2D(shifted); // Merkeze al

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double dist = std::sqrt((r - cy) * (r - cy) + (c - cx) * (c - cx));
            double H = 1.0;

            if (type == LOW_PASS) {
                H = std::exp(-(dist * dist) / (2.0 * cutoff1 * cutoff1));
            } 
            else if (type == HIGH_PASS) {
                H = 1.0 - std::exp(-(dist * dist) / (2.0 * cutoff1 * cutoff1));
            } 
            else if (type == BAND_PASS) {
                double center = (cutoff1 + cutoff2) / 2.0;
                double width = std::abs(cutoff2 - cutoff1);
                double d_from_center = dist - center;
                H = std::exp(-(d_from_center * d_from_center) / (2.0 * (width / 2.0) * (width / 2.0)));
            } 
            else if (type == BAND_REJECT) {
                double center = (cutoff1 + cutoff2) / 2.0;
                double width = std::abs(cutoff2 - cutoff1);
                double d_from_center = dist - center;
                H = 1.0 - std::exp(-(d_from_center * d_from_center) / (2.0 * (width / 2.0) * (width / 2.0)));
            }

            shifted[r][c] *= H;
        }
    }

    fftShift2D(shifted); // Un-shift yap
    return shifted;
}

int main() {
    std::string img_path = "/content/zoneplate.png";
    cv::Mat src = cv::imread(img_path, cv::IMREAD_GRAYSCALE);

    if (src.empty()) {
        std::cerr << "[HATA] " << img_path << " bulunamadı! Lütfen görseli Colab'a yükleyin." << std::endl;
        return -1;
    }

    // 256x256 Çözünürlüğe Getirme (Hesaplama Hızı İçin)
    cv::Mat clean;
    cv::resize(src, clean, cv::Size(256, 256), 0, 0, cv::INTER_CUBIC);

    // Gauss Gürültüsü Ekleme
    cv::Mat noisy = clean.clone();
    std::random_device rd; std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, 20.0);
    for (int r = 0; r < clean.rows; ++r) {
        for (int c = 0; c < clean.cols; ++c) {
            double val = static_cast<double>(clean.at<uchar>(r, c)) + dist(gen);
            noisy.at<uchar>(r, c) = static_cast<uchar>(std::clamp(val, 0.0, 255.0));
        }
    }

    std::cout << "[1/5] 2D-DFT hesaplanıyor..." << std::endl;
    auto spec = compute2D_DFT(noisy);

    std::cout << "[2/5] Low-Pass Filtresi uygulanıyor..." << std::endl;
    auto spec_lp = apply2DFilter(spec, LOW_PASS, 35.0);
    cv::Mat img_lp = compute2D_IDFT(spec_lp);

    std::cout << "[3/5] High-Pass Filtresi uygulanıyor..." << std::endl;
    auto spec_hp = apply2DFilter(spec, HIGH_PASS, 20.0);
    cv::Mat img_hp = compute2D_IDFT(spec_hp);

    std::cout << "[4/5] Band-Pass Filtresi uygulanıyor..." << std::endl;
    auto spec_bp = apply2DFilter(spec, BAND_PASS, 20.0, 60.0);
    cv::Mat img_bp = compute2D_IDFT(spec_bp);

    std::cout << "[5/5] Band-Reject Filtresi uygulanıyor..." << std::endl;
    auto spec_br = apply2DFilter(spec, BAND_REJECT, 20.0, 60.0);
    cv::Mat img_br = compute2D_IDFT(spec_br);

    // 2 Satır x 3 Sütun Görsel Paneli
    int w = 256, h = 256;
    cv::Mat grid(h * 2, w * 3, CV_8UC1);

    clean.copyTo(grid(cv::Rect(0, 0, w, h)));
    noisy.copyTo(grid(cv::Rect(w, 0, w, h)));
    img_lp.copyTo(grid(cv::Rect(w * 2, 0, w, h)));

    img_hp.copyTo(grid(cv::Rect(0, h, w, h)));
    img_bp.copyTo(grid(cv::Rect(w, h, w, h)));
    img_br.copyTo(grid(cv::Rect(w * 2, h, w, h)));

    cv::cvtColor(grid, grid, cv::COLOR_GRAY2BGR);

    cv::putText(grid, "1. Orijinal zoneplate", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 0), 1);
    cv::putText(grid, "2. Gurultulu Resim", cv::Point(w + 10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 255), 1);
    cv::putText(grid, "3. Low-Pass Filter (LPF)", cv::Point(w * 2 + 10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 255), 1);

    cv::putText(grid, "4. High-Pass Filter (HPF)", cv::Point(10, h + 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 255), 1);
    cv::putText(grid, "5. Band-Pass Filter (BPF)", cv::Point(w + 10, h + 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 255), 1);
    cv::putText(grid, "6. Band-Reject Filter (BRF)", cv::Point(w * 2 + 10, h + 20), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 255), 1);

    cv::imwrite("/content/week8_task10_filter_results.png", grid);

    std::cout << "==========================================================" << std::endl;
    std::cout << "  GÖREV 10: 2D FREKANS FİLTRELERİ UYGULANDI VE KAYDEDİLDİ " << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "[OK] Çıktı Görseli -> /content/week8_task10_filter_results.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week8_task10_filters.cpp -o task_week8_task10_filters \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week8_task10_filters