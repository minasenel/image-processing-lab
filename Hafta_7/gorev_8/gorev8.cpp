%%bash
cat << 'EOF' > task_week8_noise_2d_dft_512.cpp
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

// 1. 1D-DFT FONKSİYONU
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

// 2. RESMİ MERKEZDEN 2'NİN KUVVETİNE KIRPMA (612x612 -> 512x512)
cv::Mat cropToPowerOfTwo(const cv::Mat& src) {
    int w = src.cols;
    int h = src.rows;

    int target_dim = 1;
    while (target_dim * 2 <= std::min(w, h)) { 
        target_dim *= 2; 
    }

    int start_x = (w - target_dim) / 2;
    int start_y = (h - target_dim) / 2;

    cv::Rect roi(start_x, start_y, target_dim, target_dim);
    cv::Mat cropped = src(roi).clone();

    std::cout << "[BOYUT] Orijinal: " << w << "x" << h 
              << " -> Ortadan Kırpılan 2^N: " << target_dim << "x" << target_dim << std::endl;

    return cropped;
}

// 3. GAUSS GÜRÜLTÜSÜ EKLEME (2D)
cv::Mat addGaussianNoise2D(const cv::Mat& src, double mean = 0.0, double stddev = 30.0) {
    cv::Mat noisy = src.clone();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(mean, stddev);

    for (int r = 0; r < noisy.rows; ++r) {
        for (int c = 0; c < noisy.cols; ++c) {
            double val = static_cast<double>(src.at<uchar>(r, c)) + dist(gen);
            noisy.at<uchar>(r, c) = static_cast<uchar>(std::clamp(val, 0.0, 255.0));
        }
    }
    return noisy;
}

// 4. SALT AND PEPPER GÜRÜLTÜSÜ EKLEME (2D)
cv::Mat addSaltAndPepperNoise2D(const cv::Mat& src, double noise_ratio = 0.08) {
    cv::Mat noisy = src.clone();
    int total_pixels = src.rows * src.cols;
    int num_noise_pixels = static_cast<int>(total_pixels * noise_ratio);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> row_dist(0, src.rows - 1);
    std::uniform_int_distribution<int> col_dist(0, src.cols - 1);
    std::uniform_int_distribution<int> type_dist(0, 1);

    for (int i = 0; i < num_noise_pixels; ++i) {
        int r = row_dist(gen);
        int c = col_dist(gen);
        noisy.at<uchar>(r, c) = (type_dist(gen) == 0) ? 0 : 255;
    }
    return noisy;
}

// 5. 2D-DFT HESAPLAMA VE HARMONİK GENLİK MATRİSİ DÖNDÜRME
cv::Mat compute2D_DFT(const cv::Mat& img) {
    int rows = img.rows;
    int cols = img.cols;

    std::vector<std::vector<Complex>> matrix(rows, std::vector<Complex>(cols));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            matrix[r][c] = Complex(static_cast<double>(img.at<uchar>(r, c)), 0.0);
        }
    }

    // Satır ve Sütun 1D-DFT
    for (int r = 0; r < rows; ++r) matrix[r] = compute1D_DFT(matrix[r]);
    for (int c = 0; c < cols; ++c) {
        std::vector<Complex> col_in(rows);
        for (int r = 0; r < rows; ++r) col_in[r] = matrix[r][c];
        std::vector<Complex> col_out = compute1D_DFT(col_in);
        for (int r = 0; r < rows; ++r) matrix[r][c] = col_out[r];
    }

    // Harmonik Genliği (Magnitude Spectrum) + Logaritmik Ölçekleme
    cv::Mat magnitude_img(rows, cols, CV_64FC1);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double mag = std::abs(matrix[r][c]);
            magnitude_img.at<double>(r, c) = std::log(1.0 + mag);
        }
    }

    // FFTShift (Alçak Frekans / DC Bileşenini Merkeze Alma)
    int cx = cols / 2;
    int cy = rows / 2;
    cv::Mat q0(magnitude_img, cv::Rect(0, 0, cx, cy));   
    cv::Mat q1(magnitude_img, cv::Rect(cx, 0, cx, cy));  
    cv::Mat q2(magnitude_img, cv::Rect(0, cy, cx, cy));  
    cv::Mat q3(magnitude_img, cv::Rect(cx, cy, cx, cy)); 

    cv::Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);

    cv::normalize(magnitude_img, magnitude_img, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    return magnitude_img;
}

int main() {
    std::string img_path = "/content/lines.jpg";
    cv::Mat src = cv::imread(img_path, cv::IMREAD_GRAYSCALE);

    if (src.empty()) {
        std::cerr << "[HATA] " << img_path << " bulunamadı! Lütfen /content/lines.jpg dosyasını yükleyin." << std::endl;
        return -1;
    }

    // 1. Resmi 2'nin Kuvvetine (512x512) Kırpma
    cv::Mat img_clean = cropToPowerOfTwo(src);

    // 2. Gürültülü Görselleri Oluşturma
    cv::Mat img_gauss = addGaussianNoise2D(img_clean, 0.0, 30.0);
    cv::Mat img_sp    = addSaltAndPepperNoise2D(img_clean, 0.08);

    // 3. 2D-DFT Harmonik Matrislerinin Hesabı
    std::cout << "[1/3] Temiz Resim 2D-DFT Harmonik Matrisi hesaplanıyor..." << std::endl;
    cv::Mat harm_clean = compute2D_DFT(img_clean);

    std::cout << "[2/3] Gauss Gürültülü Resim 2D-DFT Harmonik Matrisi hesaplanıyor..." << std::endl;
    cv::Mat harm_gauss = compute2D_DFT(img_gauss);

    std::cout << "[3/3] Salt & Pepper Gürültülü Resim 2D-DFT Harmonik Matrisi hesaplanıyor..." << std::endl;
    cv::Mat harm_sp = compute2D_DFT(img_sp);

    // 4. Görselleştirme Paneli Oluşturma (2 Satır x 3 Sütun)
    int w = img_clean.cols, h = img_clean.rows;
    cv::Mat grid(h * 2, w * 3, CV_8UC1);

    // Üst Satır: Zaman / Mekan Uzayı Resimleri
    img_clean.copyTo(grid(cv::Rect(0, 0, w, h)));
    img_gauss.copyTo(grid(cv::Rect(w, 0, w, h)));
    img_sp.copyTo(grid(cv::Rect(w * 2, 0, w, h)));

    // Alt Satır: Frekans Uzayı Harmonik Matrisleri
    harm_clean.copyTo(grid(cv::Rect(0, h, w, h)));
    harm_gauss.copyTo(grid(cv::Rect(w, h, w, h)));
    harm_sp.copyTo(grid(cv::Rect(w * 2, h, w, h)));

    cv::cvtColor(grid, grid, cv::COLOR_GRAY2BGR);

    // Başlıklar
    cv::putText(grid, "1. Temiz (512x512)", cv::Point(15, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(grid, "2. Gauss Gurultulu", cv::Point(w + 15, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(grid, "3. Salt & Pepper", cv::Point(w * 2 + 15, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    cv::putText(grid, "Temiz Harmonik", cv::Point(15, h + 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
    cv::putText(grid, "Gauss Harmonik", cv::Point(w + 15, h + 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
    cv::putText(grid, "S&P Harmonik", cv::Point(w * 2 + 15, h + 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);

    cv::imwrite("/content/week8_lines_noise_dft_result.png", grid);

    std::cout << "==================================================" << std::endl;
    std::cout << "  GÖREV 8: lines.jpg (512x512) 2D HARMONİK ANALİZ " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "[OK] Çıktı kaydedildi -> /content/week8_lines_noise_dft_result.png" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week8_noise_2d_dft_512.cpp -o task_week8_noise_2d_dft_512 \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week8_noise_2d_dft_512