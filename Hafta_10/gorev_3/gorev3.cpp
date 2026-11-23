/*
Hafta 10 - Gorev 3: Blok benzerligi icin 3 farkli yontem - SAD, SSD, ZNCC
(fonksiyon pointer ile)

Girdi (Gorev 1-2 ile ayni stereo cift, Colab'da /content/ icine yuklenir):
  scene1.row3.col1.ppm -> SOL goruntu
  scene1.row3.col3.ppm -> SAG goruntu

Yontem:
1. Gorev 2'deki manuel blok eslestirme iskeleti aynen korunur (7x7 pencere,
   max disparity 16, winner-take-all). Fark, benzerlik/fark olcumunun TEK BIR
   yonteme (SAD) degil, AYNI imzaya (parametre sayisi/turu) sahip UC AYRI
   inline fonksiyona (SAD, SSD, ZNCC) birakilmasidir.
2. Her uc fonksiyon da ayni tipte bir fonksiyon pointer'i
   (ScoreFunc = double(*)(const cv::Mat&, const cv::Mat&, int, int, int))
   ile uyumludur; bu sayede computeDisparityMap() fonksiyonu HANGI yontemin
   kullanilacagini bilmeden, sadece bir fonksiyon pointer'i parametresi
   alarak calisir (ayni dongu kodu uc yontem icin de tekrar yazilmaz).
3. Uc yontem de "DUSUK SKOR = IYI ESLESME" kuralina uyacak sekilde
   TASARLANMISTIR (boylece ayni winner-take-all - min skor secme - mantigi
   uc yontem icin de aynen calisir):
   - SAD (Sum of Absolute Differences): |fark|'larin toplami. Dusuk SAD
     = benzer blok.
   - SSD (Sum of Squared Differences): fark karelerinin toplami. Dusuk SSD
     = benzer blok (SAD'a benzer ama buyuk farklari daha agir cezalandirir).
   - ZNCC (Zero-mean Normalized Cross-Correlation): -1..+1 araliginda,
     +1 = mukemmel benzer, -1 = tam ters. Bu bir BENZERLIK (similarity)
     olcusu oldugu ve "yuksek=iyi" calistigi icin, diger ikisiyle AYNI
     "dusuk=iyi" kuralina uydurmak amaciyla MALIYET (cost) olarak
     (1 - ZNCC) dondurulur: ZNCC=+1 (mukemmel eslesme) -> cost=0.
4. Hoca ayrica pencere yaricapi 3, 7 ve 13 (7x7, 15x15, 27x27 blok) icin de
   hesaplama istedi. Fonksiyon pointer imzasi degismesin diye HALF_WIN
   parametre olarak eklenmedi; main() her yaricap icin global HALF_WIN
   degiskenini degistirip AYNI SAD/SSD/ZNCC fonksiyonlarini ve AYNI
   computeDisparityMap() dongusunu farkli pencere boyutuyla yeniden
   calistiriyor.
5. Her (pencere yaricapi, yontem) kombinasyonu icin ayri bir disparity
   haritasi hesaplanir, COLORMAP_JET ile renklendirilir; sonuclar tek bir
   grid'de (satirlar = pencere yaricapi, sutunlar = SAD | SSD | ZNCC)
   sol/sag goruntuyle birlikte kaydedilir. Her kombinasyonun hesaplama
   suresi (std::chrono) konsola yazdirilir.
*/

%%bash
cat << 'EOF' > week10_task3_sad_ssd_zncc.cpp
#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>
#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// HALF_WIN artik SABIT degil: hoca 3, 7 ve 13 pencere yaricaplari (7x7,
// 15x15, 27x27 blok) icin de hesaplama istedi. Fonksiyon pointer imzasi
// (ScoreFunc) degismesin diye HALF_WIN parametre olarak eklenmedi; bunun
// yerine main() her yaricap icin bu global degiskeni degistirip ayni
// SAD/SSD/ZNCC fonksiyonlarini ve ayni computeDisparityMap() dongusunu
// farkli pencere boyutuyla yeniden calistiriyor.
static int HALF_WIN = 3;
static const int MAX_DISPARITY = 16;  // d = 0..15 arasinda aranir

// Her uc blok-benzerligi fonksiyonu da AYNI imzaya sahiptir, bu yuzden
// ayni fonksiyon pointer turuyle tutulabilirler.
using ScoreFunc = double(*)(const cv::Mat&, const cv::Mat&, int, int, int);

// SAD: mutlak farklarin toplami. Dusuk skor = benzer blok.
inline double computeSAD(const cv::Mat& leftGray, const cv::Mat& rightGray,
                          int xLeft, int xRight, int y) {
    double sad = 0.0;
    for (int dy = -HALF_WIN; dy <= HALF_WIN; ++dy) {
        const uchar* lRow = leftGray.ptr<uchar>(y + dy);
        const uchar* rRow = rightGray.ptr<uchar>(y + dy);
        for (int dx = -HALF_WIN; dx <= HALF_WIN; ++dx) {
            int diff = (int)lRow[xLeft + dx] - (int)rRow[xRight + dx];
            sad += std::abs(diff);
        }
    }
    return sad;
}

// SSD: fark karelerinin toplami. SAD'a benzer ama buyuk tek tek farklari
// (karesi alindigi icin) cok daha agir cezalandirir.
inline double computeSSD(const cv::Mat& leftGray, const cv::Mat& rightGray,
                          int xLeft, int xRight, int y) {
    double ssd = 0.0;
    for (int dy = -HALF_WIN; dy <= HALF_WIN; ++dy) {
        const uchar* lRow = leftGray.ptr<uchar>(y + dy);
        const uchar* rRow = rightGray.ptr<uchar>(y + dy);
        for (int dx = -HALF_WIN; dx <= HALF_WIN; ++dx) {
            double diff = (double)lRow[xLeft + dx] - (double)rRow[xRight + dx];
            ssd += diff * diff;
        }
    }
    return ssd;
}

// ZNCC: ortalamadan sapmalarin normalize edilmis carpimi (-1..+1), parlaklik/
// kontrast farklarina SAD ve SSD'den daha dayaniklidir. +1 = mukemmel
// eslesme oldugu icin, digerleriyle AYNI "dusuk skor=iyi" kuralina
// uydurmak icin MALIYET olarak (1 - ZNCC) dondurulur.
inline double computeZNCCCost(const cv::Mat& leftGray, const cv::Mat& rightGray,
                               int xLeft, int xRight, int y) {
    const int N = (2 * HALF_WIN + 1) * (2 * HALF_WIN + 1);

    double meanL = 0.0, meanR = 0.0;
    for (int dy = -HALF_WIN; dy <= HALF_WIN; ++dy) {
        const uchar* lRow = leftGray.ptr<uchar>(y + dy);
        const uchar* rRow = rightGray.ptr<uchar>(y + dy);
        for (int dx = -HALF_WIN; dx <= HALF_WIN; ++dx) {
            meanL += lRow[xLeft + dx];
            meanR += rRow[xRight + dx];
        }
    }
    meanL /= N;
    meanR /= N;

    double numerator = 0.0, denomL = 0.0, denomR = 0.0;
    for (int dy = -HALF_WIN; dy <= HALF_WIN; ++dy) {
        const uchar* lRow = leftGray.ptr<uchar>(y + dy);
        const uchar* rRow = rightGray.ptr<uchar>(y + dy);
        for (int dx = -HALF_WIN; dx <= HALF_WIN; ++dx) {
            double lv = lRow[xLeft + dx] - meanL;
            double rv = rRow[xRight + dx] - meanR;
            numerator += lv * rv;
            denomL += lv * lv;
            denomR += rv * rv;
        }
    }

    double denom = std::sqrt(denomL * denomR);
    double zncc = (denom > 1e-6) ? (numerator / denom) : 0.0; // duz/dokusuz blok -> 0 kabul edilir
    return 1.0 - zncc; // cost: dusuk=iyi, zncc=+1 -> cost=0
}

// Hangi skor fonksiyonunun kullanilacagini BILMEDEN, sadece bir fonksiyon
// pointer'i parametresi alarak calisan genel blok eslestirme fonksiyonu.
cv::Mat computeDisparityMap(const cv::Mat& leftGray, const cv::Mat& rightGray,
                             ScoreFunc scoreFn, cv::Mat& validMaskOut) {
    const int rows = leftGray.rows;
    const int cols = leftGray.cols;

    cv::Mat disparity = cv::Mat::zeros(rows, cols, CV_32F);
    validMaskOut = cv::Mat::zeros(rows, cols, CV_8U);

    for (int y = HALF_WIN; y < rows - HALF_WIN; ++y) {
        for (int x = HALF_WIN; x < cols - HALF_WIN; ++x) {
            double bestScore = std::numeric_limits<double>::max();
            int bestD = -1;

            for (int d = 0; d < MAX_DISPARITY; ++d) {
                int xr = x - d;
                if (xr - HALF_WIN < 0) break;

                double score = scoreFn(leftGray, rightGray, x, xr, y);
                if (score < bestScore) {
                    bestScore = score;
                    bestD = d;
                }
            }

            if (bestD >= 0) {
                disparity.at<float>(y, x) = (float)bestD;
                validMaskOut.at<uchar>(y, x) = 255;
            }
        }
    }

    return disparity;
}

cv::Mat colorizeDisparity(const cv::Mat& disparity, const cv::Mat& validMask) {
    double minVal, maxVal;
    cv::minMaxLoc(disparity, &minVal, &maxVal, nullptr, nullptr, validMask);

    cv::Mat disp8u;
    disparity.convertTo(disp8u, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    disp8u.setTo(0, ~validMask);

    cv::Mat dispColor;
    cv::applyColorMap(disp8u, dispColor, cv::COLORMAP_JET);
    dispColor.setTo(cv::Scalar(0, 0, 0), ~validMask);
    return dispColor;
}

cv::Mat withLabel(const cv::Mat& img, const std::string& text) {
    cv::Mat out = img.clone();
    cv::Mat bar(30, out.cols, CV_8UC3, cv::Scalar(30, 30, 30));
    cv::putText(bar, text, cv::Point(8, 21), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    cv::Mat cell;
    cv::vconcat(bar, out, cell);
    return cell;
}

int main() {
    cv::Mat leftBGR  = cv::imread("/content/scene1.row3.col1.ppm", cv::IMREAD_COLOR);
    cv::Mat rightBGR = cv::imread("/content/scene1.row3.col3.ppm", cv::IMREAD_COLOR);

    if (leftBGR.empty() || rightBGR.empty()) {
        std::cerr << "[HATA] Stereo cift okunamadi. scene1.row3.col1.ppm ve "
                  << "scene1.row3.col3.ppm dosyalarini /content/ icine yukleyin."
                  << std::endl;
        return -1;
    }

    cv::Mat leftGray, rightGray;
    cv::cvtColor(leftBGR,  leftGray,  cv::COLOR_BGR2GRAY);
    cv::cvtColor(rightBGR, rightGray, cv::COLOR_BGR2GRAY);

    // Ayni imzaya sahip 3 fonksiyon, fonksiyon pointer listesi olarak tutulur.
    struct MethodEntry { const char* name; ScoreFunc fn; };
    MethodEntry methods[] = {
        {"SAD",  computeSAD},
        {"SSD",  computeSSD},
        {"ZNCC", computeZNCCCost},
    };

    // Hocanin istedigi 3 pencere yaricapi: 3 (7x7), 7 (15x15), 13 (27x27).
    const int windowRadii[] = {3, 7, 13};

    std::vector<cv::Mat> rows;
    rows.push_back([&] {
        cv::Mat lc = withLabel(leftBGR,  "Sol (col1)");
        cv::Mat rc = withLabel(rightBGR, "Sag (col3)");
        cv::Mat r;
        cv::hconcat(std::vector<cv::Mat>{lc, rc}, r);
        return r;
    }());

    for (int radius : windowRadii) {
        HALF_WIN = radius; // ayni SAD/SSD/ZNCC fonksiyonlari, farkli pencere boyutuyla
        int winSize = 2 * radius + 1;

        std::vector<cv::Mat> methodCells;
        for (const auto& m : methods) {
            cv::Mat validMask;
            auto t0 = std::chrono::high_resolution_clock::now();
            cv::Mat disparity = computeDisparityMap(leftGray, rightGray, m.fn, validMask);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            double minVal, maxVal;
            cv::minMaxLoc(disparity, &minVal, &maxVal, nullptr, nullptr, validMask);
            int validCount = cv::countNonZero(validMask);

            std::cout << "HALF_WIN=" << radius << " (" << winSize << "x" << winSize << ") "
                      << m.name << " -> sure: " << ms << " ms, "
                      << "disparity araligi: [" << minVal << ", " << maxVal << "], "
                      << "gecerli piksel: " << validCount << std::endl;

            cv::Mat dispColor = colorizeDisparity(disparity, validMask);
            cv::Mat dispCell = withLabel(dispColor,
                m.name + std::string(" (") + std::to_string(winSize) + "x" + std::to_string(winSize) + ")");
            methodCells.push_back(dispCell);
        }

        cv::Mat row;
        cv::hconcat(methodCells, row);
        rows.push_back(row);
    }

    // Satirlar farkli genislikte olabilir (sol+sag ikili vs tek disparity
    // hucresi); grid'de hepsini ayni genislige getiriyoruz.
    int gridWidth = rows[0].cols;
    for (auto& r : rows) {
        if (r.cols != gridWidth) {
            cv::Mat resized;
            cv::resize(r, resized, cv::Size(gridWidth, r.rows * gridWidth / r.cols));
            r = resized;
        }
    }

    cv::Mat grid;
    cv::vconcat(rows, grid);

    std::string outPath = "/content/gorev3_sad_ssd_zncc.png";
    cv::imwrite(outPath, grid);
    std::cout << "Sonuc kaydedildi: " << outPath << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week10_task3_sad_ssd_zncc.cpp -o week10_task3_sad_ssd_zncc \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week10_task3_sad_ssd_zncc
