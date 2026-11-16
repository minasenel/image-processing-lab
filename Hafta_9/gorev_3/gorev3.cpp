%%bash
cat << 'EOF' > week9_task3_8k_divide.cpp
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
    // 8K çözünürlük: 7680 x 4320, 3 kanal (BGR)
    const int width = 7680;
    const int height = 4320;
    std::string bmp_path = "/content/image_8k.bmp";

    // 1. 8K, 3 kanallı bir BMP görsel OLUŞTUR (rastgele piksel değerleriyle)
    cv::Mat img8k(height, width, CV_8UC3);
    cv::randu(img8k, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
    cv::imwrite(bmp_path, img8k);
    std::cout << "8K BMP olusturuldu: " << bmp_path
              << " (" << width << "x" << height << ", " << img8k.channels() << " kanal)" << std::endl;

    // 2. OpenCV ile aç
    cv::Mat img = cv::imread(bmp_path, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "[HATA] Goruntu acilamadi: " << bmp_path << std::endl;
        return -1;
    }

    // 3. Her piksel degerini dongu ile tek tek 2'ye bol - SADECE bu islemin suresini olc
    auto start = std::chrono::high_resolution_clock::now();

    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            cv::Vec3b& pixel = img.at<cv::Vec3b>(y, x);
            pixel[0] = pixel[0] / 2; // B
            pixel[1] = pixel[1] / 2; // G
            pixel[2] = pixel[2] / 2; // R
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end - start;

    std::cout << "Resmi 2'ye bolme suresi: " << elapsed_ms.count() << " ms" << std::endl;

    // 4. Sonucu BMP olarak kaydet (sureye dahil degil)
    std::string out_path = "/content/image_8k_divided.bmp";
    cv::imwrite(out_path, img);
    std::cout << "Sonuc kaydedildi: " << out_path << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week9_task3_8k_divide.cpp -o week9_task3_8k_divide \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task3_8k_divide
