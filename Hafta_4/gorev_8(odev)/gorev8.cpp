%%bash
cat << 'EOF' > task8_canny_manual.cpp
// GÖREV 8 (ÖDEV): Canny kenar bulma algoritmasını SIFIRDAN yazma.
// Not: Bu görevdeki 4 adımın büyük kısmı önceki haftalarda parça parça zaten yazılmıştı.
// Aşağıda her fonksiyonun yanında hangi eski görevden geldiği / neyin değiştiği not edildi.

#include <iostream>
#include <cmath>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>   // sadece dogrulama icin cv::Canny ile karsilastirmada kullanilacak

using Clock = std::chrono::high_resolution_clock;

// ------------------------------------------------------------------
// ADIM 1: GURULTU BASTIRMA (5x5 GAUSS FILTRE)
// Kaynak: Hafta_4/gorev_1, gorev_3, gorev_5 -> applyConvolution() ile 3x3 gauss kernel
//         (1/16 * [1 2 1; 2 4 2; 1 2 1]) zaten yazilmisti.
// Kaynak: Hafta_4/gorev_6 (Mexican Hat, 7x7) -> degisken "radius" ile pencere boyutunu
//         genel tutma mantigi oradan alindi.
// DEGISIKLIK: Odev 5x5 istiyor, elimizdeki kernel 3x3'tu. Standart 5x5 Gauss (sigma~1,
// toplam=256) matrisiyle genisletildi. Ciktiyi float (CV_32FC1) tutuyoruz ki Sobel ve
// NMS adimlarinda deger kaybi (clamping) olmasin.
// ------------------------------------------------------------------
cv::Mat gaussianBlur5x5(const cv::Mat& src) {
    // Hafta_3/gorev_4 (convolution.cpp) deki 3x3 box/weighted kernel mantiginin 5x5 hali
    const float kernel[5][5] = {
        {1.0f/256, 4.0f/256,  6.0f/256, 4.0f/256, 1.0f/256},
        {4.0f/256, 16.0f/256, 24.0f/256, 16.0f/256, 4.0f/256},
        {6.0f/256, 24.0f/256, 36.0f/256, 24.0f/256, 6.0f/256},
        {4.0f/256, 16.0f/256, 24.0f/256, 16.0f/256, 4.0f/256},
        {1.0f/256, 4.0f/256,  6.0f/256, 4.0f/256, 1.0f/256}
    };

    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, CV_32FC1);
    const int radius = 2; // 5x5 -> yaricap 2 (Hafta_4/gorev_6'daki 7x7 icin radius=3 mantigiyla aynı)

    // Hafta_3/gorev_4'teki "kenar tasmasini onlemek icin 1'den basla" mantigi,
    // burada kernel yaricapi (radius) kadar kenardan iceri baslatilarak genellestirildi.
    for (int r = radius; r < src.rows - radius; ++r) {
        for (int c = radius; c < src.cols - radius; ++c) {
            float sum = 0.0f;
            for (int kr = -radius; kr <= radius; ++kr) {
                const uchar* row = src.ptr<uchar>(r + kr);
                for (int kc = -radius; kc <= radius; ++kc) {
                    sum += row[c + kc] * kernel[kr + radius][kc + radius];
                }
            }
            dst.at<float>(r, c) = sum;
        }
    }
    return dst;
}

// ------------------------------------------------------------------
// ADIM 2: SOBEL FILTRE ILE GRADYAN HESAPLAMA
// Kaynak: Hafta_4/gorev_3 (task3_gauss_sobel.cpp) -> Gx ve Gy Sobel kernelleri
//         (dikey: [-1 0 1; -2 0 2; -1 0 1], yatay: [-1 -2 -1; 0 0 0; 1 2 1]) oradan alindi.
//         O görevde Gx ve Gy ayri ayri goruntu olarak kaydedilmisti (take_abs ile).
// DEGISIKLIK: Canny icin Gx ve Gy ayri gorsel olarak degil, piksel piksel birlestirilip
// gradyan genligi (magnitude = sqrt(Gx^2+Gy^2)) ve gradyan yonu (angle = atan2(Gy,Gx))
// hesaplaniyor -> bu kisim yeni eklendi, cunku onceki gorevde sadece |Gx| veya |Gy|
// goruntusu isteniyordu, magnitude+angle ciftini Canny'nin NMS adimi icin istiyoruz.
// ------------------------------------------------------------------
void sobelGradient(const cv::Mat& blurred, cv::Mat& magnitude, cv::Mat& angle) {
    // Hafta_4/gorev_3'teki ayni Gx/Gy kernelleri
    const float gx_k[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const float gy_k[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    magnitude = cv::Mat::zeros(blurred.size(), CV_32FC1);
    angle     = cv::Mat::zeros(blurred.size(), CV_32FC1);

    for (int r = 1; r < blurred.rows - 1; ++r) {
        for (int c = 1; c < blurred.cols - 1; ++c) {
            float gx = 0.0f, gy = 0.0f;
            for (int kr = -1; kr <= 1; ++kr) {
                for (int kc = -1; kc <= 1; ++kc) {
                    float px = blurred.at<float>(r + kr, c + kc); // Adim 1'den gelen float goruntu
                    gx += px * gx_k[kr + 1][kc + 1];
                    gy += px * gy_k[kr + 1][kc + 1];
                }
            }
            magnitude.at<float>(r, c) = std::sqrt(gx * gx + gy * gy);           // G = sqrt(Gx^2 + Gy^2)
            angle.at<float>(r, c)     = std::atan2(gy, gx) * 180.0f / CV_PI;    // theta (derece)
        }
    }
}

// ------------------------------------------------------------------
// ADIM 3: NON-MAXIMUM SUPPRESSION (NMS)
// Kaynak: YOK -> bu adim onceki haftalarda hic yazilmamisti, ilk kez burada yaziliyor.
// Mantik: Her pikselin gradyan yonundeki (theta) iki komsusuna bakilir. Eger piksel bu
// iki komsusundan daha buyuk degilse (lokal maksimum degilse) bastirilir (0 yapilir).
// Aci, hesaplama kolayligi icin 4 ana yone (0, 45, 90, 135 derece) yuvarlanir.
// (Sinav notu: "45 derecelik olsaydi nasil bulurduk?" sorusunun cevabi tam burasi.)
// ------------------------------------------------------------------
cv::Mat nonMaxSuppression(const cv::Mat& magnitude, const cv::Mat& angle) {
    cv::Mat out = cv::Mat::zeros(magnitude.size(), CV_32FC1);

    for (int r = 1; r < magnitude.rows - 1; ++r) {
        for (int c = 1; c < magnitude.cols - 1; ++c) {
            float ang = angle.at<float>(r, c);
            if (ang < 0) ang += 180.0f; // yonu 0-180 araligina indir (simetrik)

            float mag = magnitude.at<float>(r, c);
            float n1, n2;

            if ((ang >= 0 && ang < 22.5f) || (ang >= 157.5f && ang <= 180.0f)) {
                // yatay yon (0 derece) -> sol/sag komsu
                n1 = magnitude.at<float>(r, c - 1);
                n2 = magnitude.at<float>(r, c + 1);
            } else if (ang >= 22.5f && ang < 67.5f) {
                // 45 derece -> capraz komsu
                n1 = magnitude.at<float>(r - 1, c + 1);
                n2 = magnitude.at<float>(r + 1, c - 1);
            } else if (ang >= 67.5f && ang < 112.5f) {
                // dikey yon (90 derece) -> ust/alt komsu
                n1 = magnitude.at<float>(r - 1, c);
                n2 = magnitude.at<float>(r + 1, c);
            } else {
                // 135 derece -> ters capraz komsu
                n1 = magnitude.at<float>(r - 1, c - 1);
                n2 = magnitude.at<float>(r + 1, c + 1);
            }

            out.at<float>(r, c) = (mag >= n1 && mag >= n2) ? mag : 0.0f;
        }
    }
    return out;
}

// ------------------------------------------------------------------
// ADIM 4: HYSTERESIS THRESHOLDING
// Kaynak: YOK -> bu adim da onceki haftalarda hic yazilmamisti, ilk kez burada yaziliyor.
// Benzerlik notu: Hafta_2/gorev_5'teki "cv::saturate_cast ile [0,255] araligina kilitleme"
// (clipping) mantigi burada da var ama tek esik yerine IKI esik (low/high) ve komsuluk
// bazli baglanti kontrolu ekleniyor.
// Mantik:
//   mag >= high  -> kesin (strong) kenar
//   mag <  low   -> kenar degil, silinir
//   low<=mag<high -> zayif (weak) kenar: sadece 8-komsulugunda bir strong kenara
//                    baglıysa kenar sayilir, degilse silinir.
// ------------------------------------------------------------------
cv::Mat hysteresisThreshold(const cv::Mat& nms, float low, float high) {
    cv::Mat strong = cv::Mat::zeros(nms.size(), CV_8UC1);
    cv::Mat weak   = cv::Mat::zeros(nms.size(), CV_8UC1);

    for (int r = 0; r < nms.rows; ++r) {
        for (int c = 0; c < nms.cols; ++c) {
            float v = nms.at<float>(r, c);
            if (v >= high) strong.at<uchar>(r, c) = 255;
            else if (v >= low) weak.at<uchar>(r, c) = 255;
        }
    }

    cv::Mat result = strong.clone();

    // Zayif kenarlari, guclu bir kenara 8-komsulukla baglıysa dahil et.
    // Basit iteratif yayilim (flood-fill benzeri); degisiklik olmayana kadar tekrar eder.
    bool changed = true;
    while (changed) {
        changed = false;
        for (int r = 1; r < result.rows - 1; ++r) {
            for (int c = 1; c < result.cols - 1; ++c) {
                if (weak.at<uchar>(r, c) && !result.at<uchar>(r, c)) {
                    for (int dr = -1; dr <= 1 && !result.at<uchar>(r, c); ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                            if (result.at<uchar>(r + dr, c + dc)) {
                                result.at<uchar>(r, c) = 255;
                                changed = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

int main() {
    cv::Mat src = cv::imread("/content/image3.jpg", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cerr << "[HATA] /content/image3.jpg bulunamadi!" << std::endl;
        return 1;
    }

    std::cout << "[BILGI] Girdi Boyutu: " << src.cols << "x" << src.rows << std::endl << std::endl;

    auto t0 = Clock::now();

    // Hafta_4/gorev_7'deki gibi her adim ayri bir fonksiyon cagrisi, ara ciktilar
    // kaydedilerek pipeline'in her asamasi gorulebilir hale getirildi.
    cv::Mat blurred = gaussianBlur5x5(src);                               // ADIM 1
    cv::Mat magnitude, angle;
    sobelGradient(blurred, magnitude, angle);                             // ADIM 2
    cv::Mat suppressed = nonMaxSuppression(magnitude, angle);             // ADIM 3

    // Esik degerleri: Hafta_4/gorev_7'de cv::Canny(src, dst, 50, 150) ile ayni 1:3
    // orani kullanildi, kendi sonucumuzu OpenCV'nin Canny'si ile karsilastirabilmek icin.
    const float LOW_THRESH = 50.0f;
    const float HIGH_THRESH = 150.0f;
    cv::Mat edges = hysteresisThreshold(suppressed, LOW_THRESH, HIGH_THRESH); // ADIM 4

    auto t1 = Clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "[BILGI] Manuel Canny (5x5 Gauss + Sobel + NMS + Hysteresis) suresi: "
              << elapsed_ms << " ms" << std::endl;

    // Ara ciktilar da kaydedildi (raporda adim adim gorsellestirme icin)
    cv::Mat blurred_8u, magnitude_8u;
    blurred.convertTo(blurred_8u, CV_8UC1);
    cv::normalize(magnitude, magnitude_8u, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    cv::imwrite("/content/image3_step1_gauss5x5.png", blurred_8u);
    cv::imwrite("/content/image3_step2_sobel_magnitude.png", magnitude_8u);
    cv::imwrite("/content/image3_step4_canny_manual.png", edges);
    std::cout << "[OK] Ara ciktilar ve nihai sonuc /content/ icine kaydedildi." << std::endl;

    // Dogrulama: Hafta_3/gorev_2'deki "absdiff ile hazir fonksiyonla karsilastirma"
    // mantigiyla kendi sonucumuzu OpenCV'nin hazir Canny'si (gorev_7'de kullanilmisti)
    // ile karsilastiriyoruz.
    cv::Mat opencv_canny;
    cv::Canny(src, opencv_canny, (int)LOW_THRESH, (int)HIGH_THRESH);
    cv::imwrite("/content/image3_opencv_canny_compare.png", opencv_canny);
    std::cout << "[OK] Karsilastirma icin OpenCV Canny ciktisi da kaydedildi -> "
                 "/content/image3_opencv_canny_compare.png" << std::endl;

    return 0;
}
EOF

# Derle ve Calistir
g++ -std=c++17 -O3 task8_canny_manual.cpp -o task8_canny_manual \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./task8_canny_manual
