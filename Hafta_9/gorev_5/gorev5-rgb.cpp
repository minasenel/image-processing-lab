%%bash
cat << 'EOF' > week9_task5_skin_detection_rgb.cpp
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // 1. Goruntuyu ac (BGR = OpenCV'nin varsayilan RGB sirasi)
    std::string input_path = "/content/people.jpg";
    cv::Mat bgr = cv::imread(input_path, cv::IMREAD_COLOR);

    if (bgr.empty()) {
        std::cerr << "[HATA] " << input_path << " bulunamadi!" << std::endl;
        return -1;
    }
    std::cout << "Goruntu acildi: " << bgr.cols << "x" << bgr.rows << std::endl;

    // 2. RGB renk uzayinda ten rengi kurali (Peer vd. - literaturde yaygin kullanilan kural):
    //    R > 95, G > 40, B > 20
    //    max(R,G,B) - min(R,G,B) > 15
    //    |R - G| > 15
    //    R > G  ve  R > B
    // Bu kural dogrudan R, G, B degerleri uzerinde calisir, baska bir uzaya gecmez.
    cv::Mat result = bgr.clone();

    for (int y = 0; y < bgr.rows; y++) {
        for (int x = 0; x < bgr.cols; x++) {
            const cv::Vec3b& px = bgr.at<cv::Vec3b>(y, x);
            int B = px[0];
            int G = px[1];
            int R = px[2];

            int maxC = std::max({R, G, B});
            int minC = std::min({R, G, B});

            bool isSkin = (R > 95) && (G > 40) && (B > 20) &&
                          ((maxC - minC) > 15) &&
                          (std::abs(R - G) > 15) &&
                          (R > G) && (R > B);

            if (!isSkin) {
                // Ten OLMAYAN bolgeleri tam kirmiziya boya (BGR: 0,0,255)
                result.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255);
            }
            // Ten bolgeleri oldugu gibi birakiliyor (orijinal renk)
        }
    }

    // 3. Sonucu kaydet
    std::string out_path = "/content/people_nonskin_red_rgb.bmp";
    cv::imwrite(out_path, result);
    std::cout << "Sonuc kaydedildi: " << out_path << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week9_task5_skin_detection_rgb.cpp -o week9_task5_skin_detection_rgb \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task5_skin_detection_rgb
