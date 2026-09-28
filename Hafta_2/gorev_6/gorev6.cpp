// hocanın derste attığı kod. bit kaydırma yaparak hızlı quantizasyon yapıyor. 4 çekirdekli cpu ile paralel çalışıyor.
%%bash
cat << 'EOF' > hocanin_kodu.cpp
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace cv;
using namespace std;

void quantizeImage(Mat image, int bits) {
    const uchar mask = static_cast<uchar>(0xFF << (8 - bits));
    for (int i = 0; i < image.rows; i++) {
        uchar* p = image.ptr<uchar>(i);
        for (int j = 0; j < image.cols; j++)
            p[j] &= mask;
    }
}

int main() {
    // Yüklediğin 4K resim üzerinden okuyoruz
    Mat original = imread("image.jpeg", IMREAD_GRAYSCALE);
    if (original.empty()) {
        cerr << "image.jpeg bulunamadi!" << endl;
        return -1;
    }

    cout << "Gorsel Boyutu: " << original.cols << "x" << original.rows << endl;

    // 1. Sequential (Tek Cekirdek)
    Mat image = original.clone();
    auto start = chrono::high_resolution_clock::now();
    quantizeImage(image, 4);
    auto end = chrono::high_resolution_clock::now();
    imwrite("quantized_image.png", image);
    cout << "Quantization time (Tek Cekirdek)    : "
         << chrono::duration_cast<chrono::microseconds>(end - start).count() << " microseconds\n";

    // 2. Parallel (4 quadrants - 4 Thread)
    image = original.clone();
    int w1 = image.cols / 2, w2 = image.cols - w1;
    int h1 = image.rows / 2, h2 = image.rows - h1;

    start = chrono::high_resolution_clock::now();
    vector<thread> threads;
    threads.reserve(4);
    threads.emplace_back(quantizeImage, image(Rect(0, 0, w1, h1)), 4);
    threads.emplace_back(quantizeImage, image(Rect(w1, 0, w2, h1)), 4);
    threads.emplace_back(quantizeImage, image(Rect(0, h1, w1, h2)), 4);
    threads.emplace_back(quantizeImage, image(Rect(w1, h1, w2, h2)), 4);
    for (auto& t : threads) t.join();
    end = chrono::high_resolution_clock::now();

    cout << "Parallel quantization time (4 Thread): "
         << chrono::duration_cast<chrono::microseconds>(end - start).count() << " microseconds\n";
    imwrite("quantized_image1.png", image);

    return 0;
}
EOF

# Derle ve Calistir
g++ -std=c++17 -O3 -pthread hocanin_kodu.cpp -o hocanin_kodu \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./hocanin_kodu


öbür satır. 
import cv2
import matplotlib.pyplot as plt

orig = cv2.imread('image.jpeg', cv2.IMREAD_GRAYSCALE)
quant = cv2.imread('quantized_image1.png', cv2.IMREAD_GRAYSCALE)

plt.figure(figsize=(16, 8))

plt.subplot(1, 2, 1)
plt.title("Orijinal (8-bit / 256 Seviye)")
plt.imshow(orig, cmap='gray')
plt.axis('off')

plt.subplot(1, 2, 2)
plt.title("Kuantalanmış (4-bit / 16 Seviye)")
plt.imshow(quant, cmap='gray')
plt.axis('off')

plt.tight_layout()
plt.show()
