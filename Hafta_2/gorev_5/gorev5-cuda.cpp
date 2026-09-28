%%bash
# 1. Ortam kontrolü
if [ ! -d "/content/opencv_install" ]; then
    if [ ! -f "/content/opencv_cuda_t4.tar.gz" ]; then
        gdown "1qqkBs9lI7q--4AwHj9SNhVJOTXNk-DzP" -O /content/opencv_cuda_t4.tar.gz
    fi
    tar -xzf /content/opencv_cuda_t4.tar.gz -C /content
fi

# 2. C++ kaynak kodunu oluştur
cat << 'EOF' > task5_brightness.cpp
#include <iostream>
#include <vector>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

std::vector<int> get_histogram(const cv::Mat& mat) {
    std::vector<int> hist(256, 0);
    for (int r = 0; r < mat.rows; ++r) {
        const uchar* ptr = mat.ptr<uchar>(r);
        for (int c = 0; c < mat.cols; ++c) {
            hist[ptr[c]]++;
        }
    }
    return hist;
}

int main() {
    cv::Mat src = cv::imread("image.jpeg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "image.jpeg bulunamadi! Sol panele yuklediginizden emin olun." << std::endl;
        return 1;
    }

    int rows = src.rows;
    int cols = src.cols;

    cv::Mat out_combo = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::Mat out_mult  = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::Mat out_add   = cv::Mat::zeros(rows, cols, CV_8UC1);

    for (int r = 0; r < rows; ++r) {
        const uchar* s_ptr = src.ptr<uchar>(r);
        uchar* c_ptr = out_combo.ptr<uchar>(r);
        uchar* m_ptr = out_mult.ptr<uchar>(r);
        uchar* a_ptr = out_add.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c) {
            uchar val = s_ptr[c];
            c_ptr[c] = cv::saturate_cast<uchar>(val * 0.75f + 20.0f);
            m_ptr[c] = cv::saturate_cast<uchar>(val * 0.75f);
            a_ptr[c] = cv::saturate_cast<uchar>(val + 20);
        }
    }

    cv::imwrite("custom_trans_075_plus_20.png", out_combo);
    cv::imwrite("custom_trans_only_075.png", out_mult);
    cv::imwrite("custom_trans_only_plus_20.png", out_add);

    std::vector<int> h_src   = get_histogram(src);
    std::vector<int> h_combo = get_histogram(out_combo);
    std::vector<int> h_mult  = get_histogram(out_mult);
    std::vector<int> h_add   = get_histogram(out_add);

    std::cout << "1. Sadece 0.75 ile Carpilirsa [192, 255] arasi piksel: " 
              << std::accumulate(h_mult.begin() + 192, h_mult.end(), 0) << "\n";
    std::cout << "2. Sadece 20 Eklenirse [0, 19] arasi piksel: " 
              << std::accumulate(h_add.begin(), h_add.begin() + 20, 0) 
              << " | 255 doyan piksel: " << h_add[255] << "\n";
    std::cout << "3. Birlikte Yapilirsa [0, 19] arasi piksel: " 
              << std::accumulate(h_combo.begin(), h_combo.begin() + 20, 0) 
              << " | [212, 255] arasi piksel: " 
              << std::accumulate(h_combo.begin() + 212, h_combo.end(), 0) << "\n";

    return 0;
}
EOF

# 3. Derle ve Çalıştır
g++ -std=c++17 -O3 task5_brightness.cpp -o task5_brightness \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./task5_brightness