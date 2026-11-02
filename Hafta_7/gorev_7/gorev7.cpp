%%bash
cat << 'EOF' > task_week7_2d_dft_vertical.cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef std::complex<double> Complex;

// 1. 1D-DFT FONKSİYONU (Satır ve Sütun Dönüşümleri İçin)
std::vector<Complex> compute1D_DFT(const std::vector<Complex>& input) {
    size_t N = input.size();
    std::vector<Complex> output(N);
    double angle_factor = 2.0 * M_PI / static_cast<double>(N);

    for (size_t k = 0; k < N; ++k) {
        Complex sum(0.0, 0.0);
        for (size_t n = 0; n < N; ++n) {
            double angle = angle_factor * k * n;
            Complex twiddle(std::cos(angle), -std::sin(angle)); // Euler: e^(-j * 2*pi*k*n / N)
            sum += input[n] * twiddle;
        }
        output[k] = sum;
    }
    return output;
}

// 2. RESMİ 2'NİN KUVVETİNE KIRPMA (CENTER CROP TO 2^N)
cv::Mat cropToPowerOfTwo(const cv::Mat& src) {
    int w = src.cols;
    int h = src.rows;

    // Görsel boyutlarına uyan en büyük 2'nin kuvvetini bulma (ör: 1080x1920 -> 512x512)
    int target_dim = 1;
    while (target_dim * 2 <= std::min(w, h) && target_dim * 2 <= 512) { 
        target_dim *= 2; 
    }

    int start_x = (w - target_dim) / 2;
    int start_y = (h - target_dim) / 2;

    cv::Rect roi(start_x, start_y, target_dim, target_dim);
    cv::Mat cropped = src(roi).clone();

    std::cout << "[BOYUT BİLGİSİ] Orijinal Görsel: " << w << "x" << h 
              << " -> Ortadan Kırpılan (2^N): " << target_dim << "x" << target_dim << std::endl;

    return cropped;
}

// 3. 2D-DFT VE HARMONİK SPEKTRUMU HESAPLAMA
cv::Mat compute2D_DFT(const cv::Mat& img, std::vector<std::vector<Complex>>& complex_out) {
    int rows = img.rows;
    int cols = img.cols;

    std::vector<std::vector<Complex>> matrix(rows, std::vector<Complex>(cols));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            matrix[r][c] = Complex(static_cast<double>(img.at<uchar>(r, c)), 0.0);
        }
    }

    // A) Satır Satır 1D-DFT
    for (int r = 0; r < rows; ++r) {
        matrix[r] = compute1D_DFT(matrix[r]);
    }

    // B) Sütun Sütun 1D-DFT
    for (int c = 0; c < cols; ++c) {
        std::vector<Complex> col_in(rows);
        for (int r = 0; r < rows; ++r) col_in[r] = matrix[r][c];

        std::vector<Complex> col_out = compute1D_DFT(col_in);

        for (int r = 0; r < rows; ++r) matrix[r][c] = col_out[r];
    }

    complex_out = matrix;

    // C) Harmonik Genliği (Magnitude) ve Logaritmik Ölçekleme
    cv::Mat magnitude_img(rows, cols, CV_64FC1);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double mag = std::abs(matrix[r][c]);
            magnitude_img.at<double>(r, c) = std::log(1.0 + mag);
        }
    }

    // D) FFTShift (Alçak Frekansları / DC Bileşenini Merkeze Taşıma)
    int cx = cols / 2;
    int cy = rows / 2;

    cv::Mat q0(magnitude_img, cv::Rect(0, 0, cx, cy));   // Sol Üst
    cv::Mat q1(magnitude_img, cv::Rect(cx, 0, cx, cy));  // Sağ Üst
    cv::Mat q2(magnitude_img, cv::Rect(0, cy, cx, cy));  // Sol Alt
    cv::Mat q3(magnitude_img, cv::Rect(cx, cy, cx, cy)); // Sağ Alt

    cv::Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);

    // [0, 255] Aralığına Normalizasyon
    cv::normalize(magnitude_img, magnitude_img, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    return magnitude_img;
}

int main() {
    // 1. vertical.jpg RESMİNİ OKUMA
    std::string img_path = "/content/vertical.jpg";
    cv::Mat input_img = cv::imread(img_path, cv::IMREAD_GRAYSCALE);

    if (input_img.empty()) {
        std::cerr << "[HATA] " << img_path << " bulunamadı! Lütfen görseli Colab /content/ dizinine yükleyin." << std::endl;
        return -1;
    }

    // 2. 2'nin Kuvveti Boyutuna Kırpma (Center Crop)
    cv::Mat cropped_img = cropToPowerOfTwo(input_img);

    // 3. 2D-DFT Hesaplama
    std::vector<std::vector<Complex>> complex_spectrum;
    std::cout << "[1/2] 2D-DFT hesaplanıyor (Bu işlem piksel boyutuna göre biraz zaman alabilir)..." << std::endl;
    cv::Mat dft_spectrum = compute2D_DFT(cropped_img, complex_spectrum);

    // 4. Yan Yana Karşılaştırma Paneli Oluşturma ve Kaydetme
    int w = cropped_img.cols;
    int h = cropped_img.rows;
    cv::Mat panel(h, w * 2, CV_8UC1);

    cropped_img.copyTo(panel(cv::Rect(0, 0, w, h)));
    dft_spectrum.copyTo(panel(cv::Rect(w, 0, w, h)));

    cv::cvtColor(panel, panel, cv::COLOR_GRAY2BGR);
    cv::putText(panel, "1. Kirpilan vertical.jpg (" + std::to_string(w) + "x" + std::to_string(h) + ")", 
                cv::Point(15, 25), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 0), 1);
    cv::putText(panel, "2. 2D-DFT Harmonik Spektrumu", 
                cv::Point(w + 15, 25), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 0), 1);

    cv::imwrite("/content/week7_2d_dft_vertical_result.png", panel);

    std::cout << "==================================================" << std::endl;
    std::cout << "        2D-DFT vertical.jpg SONUÇ RAPORU          " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Girdi Görseli        : " << img_path << std::endl;
    std::cout << "İşlenen Kırpılmış Boyut : " << w << "x" << h << " (2^N Şartı Sağlandı)" << std::endl;
    std::cout << "[OK] Çıktı kaydedildi -> /content/week7_2d_dft_vertical_result.png" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 task_week7_2d_dft_vertical.cpp -o task_week7_2d_dft_vertical \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task_week7_2d_dft_vertical