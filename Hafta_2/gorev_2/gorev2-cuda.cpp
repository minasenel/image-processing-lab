%%bash
# Standart kadın portresini indir
curl -s -o human_face.jpg "https://upload.wikimedia.org/wikipedia/en/7/7d/Lenna_%28test_image%29.png"

cat << 'EOF' > quantize_woman.cpp
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
    // 1. Kadın portresini tek kanallı gri (grayscale) olarak oku
    cv::Mat img = cv::imread("human_face.jpg", cv::IMREAD_GRAYSCALE);

    if (img.empty()) {
        std::cerr << "Resim yuklenemedi!" << std::endl;
        return 1;
    }

    int rows = img.rows;
    int cols = img.cols;

    // Yalnızca 4 farklı parlaklık değeri içerecek matrisler
    cv::Mat out_64 = cv::Mat::zeros(rows, cols, CV_8UC1);
    cv::Mat out_85 = cv::Mat::zeros(rows, cols, CV_8UC1);

    // 2. Piksel piksel satır ve sütun taraması
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            uchar val = img.at<uchar>(r, c);

            // 256 seviyeyi 4 eşit dilime bölmek için tamsayı olarak 64'e böl (0, 1, 2, 3)
            uchar level = val / 64;

            // Klasik formül: (val / 64) * 64 -> Piksel değerleri sadece {0, 64, 128, 192}
            out_64.at<uchar>(r, c) = level * 64;

            // 0-255 aralığına tam yayılmış hali: {0, 85, 170, 255}
            out_85.at<uchar>(r, c) = level * 85;
        }
    }

    // 3. Görselleri kaydet
    cv::imwrite("woman_4tones_64.png", out_64);
    cv::imwrite("woman_4tones_85.png", out_85);

    std::cout << "Kadin portresi icin 4 seviyeli kuantalama tamamlandi!" << std::endl;
    std::cout << "- woman_4tones_64.png: Pikseller yalnizca 0, 64, 128, 192 degerlerini icerir." << std::endl;
    std::cout << "- woman_4tones_85.png: Pikseller yalnizca 0, 85, 170, 255 degerlerini icerir." << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 quantize_woman.cpp -o quantize_woman \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./quantize_woman

# Sonuçları tek zipte topla
zip -j -o woman_4tones.zip woman_4tones_64.png woman_4tones_85.png
