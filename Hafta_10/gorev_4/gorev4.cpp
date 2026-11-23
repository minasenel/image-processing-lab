/*
Hafta 10 - Gorev 4: SGM (Semi-Global Matching) algoritmasini sifirdan
gerceklestirme ve smoothness (P1/P2) parametresinin etkisi

Girdi (onceki goravlerle ayni stereo cift, Colab'da /content/ icine yuklenir):
  scene1.row3.col1.ppm -> SOL goruntu
  scene1.row3.col3.ppm -> SAG goruntu

Yontem (cv::StereoSGBM KULLANILMADAN, sifirdan yazildi):
1. ONCE klasik LOKAL maliyet (cost) hacmi cikariliyor: her piksel (x,y) ve
   her aday disparity d=0..15 icin, Gorev 2/3'teki gibi 7x7 pencere SAD
   skoru hesaplanip cost[d](y,x) olarak saklaniyor (eslesmenin mumkun
   olmadigi, pencerenin goruntu disina tastigi yerlerde cok buyuk bir
   maliyet - LARGE_COST - atanir).
2. Bu cost hacmi, Gorev 2/3'teki gibi SADECE piksel bazinda (bagimsiz,
   "winner-take-all") degerlendirilmiyor. Bunun yerine, SGM'nin ozu olan
   "semi-global" agregasyon uygulaniyor: cost, goruntu uzerinde 4 ayri
   1 boyutlu yol (sol->sag, sag->sol, yukari->asagi, asagi->yukari)
   boyunca ZINCIRLEME bir sekilde DUZELTILIYOR. Her yol icin, bir onceki
   pikselin agregasyon maliyeti L(p-r, d) kullanilarak:
       L_r(p,d) = C(p,d) + min( L_r(p-r,d),
                                 L_r(p-r,d-1) + P1,
                                 L_r(p-r,d+1) + P1,
                                 min_k L_r(p-r,k) + P2 )
                    - min_k L_r(p-r,k)
   formuluyle hesaplaniyor. Burada P1 ve P2, SMOOTHNESS (yumusak gecis)
   PARAMETRELERIDIR:
     - P1: komsu pikselle disparity SADECE 1 birim degisiyorsa uygulanan
       KUCUK ceza (egimli/yavas degisen yuzeyleri -ornegin bir rampayi-
       cok fazla cezalandirmadan hafifce duzlestirir).
     - P2: komsu pikselle disparity 1'den FAZLA degisiyorsa uygulanan,
       farkin buyuklugune bagli OLMAYAN (sabit) BUYUK ceza. Sabit
       oldugu icin gercek bir derinlik sicramasi (nesne kenari) varsa
       bu sicramaya "izin verir" ama o sicramayi pahali yapar; boylece
       GURULTU (tek piksellik rastgele sicramalar) bastirilirken GERCEK
       KENARLAR (object boundary) asiri bulaniklastirilmaz.
   P1/P2 olmasa (hepsi 0), bu algoritma tam olarak Gorev 2/3'teki LOKAL
   blok eslestirmeye (her piksel tamamen bagimsiz, gurultulu/noktacikli
   sonuc) geri doner.
3. 4 yoldan gelen L_r hacimleri toplanip S(p,d) = sum_r L_r(p,d) agregasyon
   (toplam) maliyeti elde edilir.
4. Son disparity, S(p,d)'yi MINIMIZE eden d secilerek (winner-take-all,
   ama bu kez KOMSULUK bilgisiyle "yumusatilmis" bir maliyet uzerinden)
   bulunur.

Not: Gercek cv::StereoSGBM 8 (bazen 16) yon kullanir; burada ogretici/
odev amacli, 4 temel yonle (sol-sag, yukari-asagi eksenleri) SADELESTIRILMIS
bir SGM gerceklestirilmistir. Algoritmanin ozu (cost agregasyonu + P1/P2
smoothness terimi) aynidir.
*/

%%bash
cat << 'EOF' > week10_task4_sgm.cpp
#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>
#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

static const int HALF_WIN = 3;          // pencere yaricapi -> 7x7 blok (cost icin)
static const int MAX_DISPARITY = 16;    // d = 0..15
static const float LARGE_COST = 1e6f;   // eslesme imkansizsa (sinir disi) atanan maliyet

// Smoothness (yumusak gecis) parametreleri:
//   P1 -> komsu pikselle disparity 1 birim degisirse kucuk ceza
//   P2 -> komsu pikselle disparity 1'den fazla degisirse (sabit) buyuk ceza
static const float P1 = 8.0f  * (2 * HALF_WIN + 1) * (2 * HALF_WIN + 1);
static const float P2 = 32.0f * (2 * HALF_WIN + 1) * (2 * HALF_WIN + 1);

// Gorev 2/3'teki ile ayni SAD blok maliyeti.
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

// Adim 1: lokal maliyet (cost) hacmini cikar. cost[d] boyutunda MAX_DISPARITY
// tane CV_32F matris; cost[d](y,x) = disparity d icin blok SAD skoru.
std::vector<cv::Mat> computeCostVolume(const cv::Mat& leftGray, const cv::Mat& rightGray) {
    int rows = leftGray.rows, cols = leftGray.cols;
    std::vector<cv::Mat> cost(MAX_DISPARITY);
    for (int d = 0; d < MAX_DISPARITY; ++d) {
        cost[d] = cv::Mat(rows, cols, CV_32F, cv::Scalar(LARGE_COST));
    }

    for (int y = HALF_WIN; y < rows - HALF_WIN; ++y) {
        for (int x = HALF_WIN; x < cols - HALF_WIN; ++x) {
            for (int d = 0; d < MAX_DISPARITY; ++d) {
                int xr = x - d;
                if (xr - HALF_WIN < 0) break; // bu disparity icin pencere sinir disi
                cost[d].at<float>(y, x) = (float)computeSAD(leftGray, rightGray, x, xr, y);
            }
        }
    }
    return cost;
}

// Adim 2: cost hacmini tek bir (dx,dy) yonunde (1 boyutlu yol) agregasyon
// (duzeltme) yap. SGM'nin smoothness (P1/P2) terimi burada devreye girer.
std::vector<cv::Mat> aggregateDirection(const std::vector<cv::Mat>& cost, int dx, int dy) {
    int rows = cost[0].rows, cols = cost[0].cols;
    std::vector<cv::Mat> L(MAX_DISPARITY);
    for (int d = 0; d < MAX_DISPARITY; ++d) L[d] = cv::Mat::zeros(rows, cols, CV_32F);

    // Yol boyunca ilerleme sirasi: (dx,dy) pozitifse artan, negatifse
    // azalan sirada gezilir ki "onceki piksel" (p - r) her zaman daha
    // once islenmis olsun.
    int yStart = (dy >= 0) ? 0 : rows - 1, yEnd = (dy >= 0) ? rows : -1, yStep = (dy >= 0) ? 1 : -1;
    int xStart = (dx >= 0) ? 0 : cols - 1, xEnd = (dx >= 0) ? cols : -1, xStep = (dx >= 0) ? 1 : -1;

    for (int y = yStart; y != yEnd; y += yStep) {
        for (int x = xStart; x != xEnd; x += xStep) {
            int py = y - dy, px = x - dx;
            bool hasPrev = (py >= 0 && py < rows && px >= 0 && px < cols);

            if (!hasPrev) {
                for (int d = 0; d < MAX_DISPARITY; ++d) L[d].at<float>(y, x) = cost[d].at<float>(y, x);
                continue;
            }

            float minPrev = std::numeric_limits<float>::max();
            for (int d = 0; d < MAX_DISPARITY; ++d) minPrev = std::min(minPrev, L[d].at<float>(py, px));

            for (int d = 0; d < MAX_DISPARITY; ++d) {
                float same  = L[d].at<float>(py, px);
                float plus  = (d + 1 < MAX_DISPARITY) ? L[d + 1].at<float>(py, px) + P1 : LARGE_COST;
                float minus = (d - 1 >= 0)            ? L[d - 1].at<float>(py, px) + P1 : LARGE_COST;
                float general = minPrev + P2;

                float smoothnessTerm = std::min(std::min(same, plus), std::min(minus, general));
                L[d].at<float>(y, x) = cost[d].at<float>(y, x) + smoothnessTerm - minPrev;
            }
        }
    }
    return L;
}

// Adim 3-4: 4 yondeki agregasyonlari topla (S = sum L_r), ardindan her
// piksel icin S(p,d)'yi minimize eden d'yi (winner-take-all) sec.
cv::Mat computeSGMDisparity(const cv::Mat& leftGray, const cv::Mat& rightGray, cv::Mat& validMaskOut) {
    std::vector<cv::Mat> cost = computeCostVolume(leftGray, rightGray);

    const int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // sol->sag, sag->sol, yukari->asagi, asagi->yukari

    std::vector<cv::Mat> S(MAX_DISPARITY);
    int rows = leftGray.rows, cols = leftGray.cols;
    for (int d = 0; d < MAX_DISPARITY; ++d) S[d] = cv::Mat::zeros(rows, cols, CV_32F);

    for (const auto& dir : directions) {
        std::vector<cv::Mat> L = aggregateDirection(cost, dir[0], dir[1]);
        for (int d = 0; d < MAX_DISPARITY; ++d) S[d] += L[d];
    }

    cv::Mat disparity = cv::Mat::zeros(rows, cols, CV_32F);
    validMaskOut = cv::Mat::zeros(rows, cols, CV_8U);

    for (int y = HALF_WIN; y < rows - HALF_WIN; ++y) {
        for (int x = HALF_WIN; x < cols - HALF_WIN; ++x) {
            float bestScore = std::numeric_limits<float>::max();
            int bestD = -1;
            for (int d = 0; d < MAX_DISPARITY; ++d) {
                float s = S[d].at<float>(y, x);
                if (s < bestScore) { bestScore = s; bestD = d; }
            }
            if (bestD >= 0 && cost[bestD].at<float>(y, x) < LARGE_COST) {
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

    auto t0 = std::chrono::high_resolution_clock::now();
    cv::Mat validMask;
    cv::Mat disparity = computeSGMDisparity(leftGray, rightGray, validMask);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    double minVal, maxVal;
    cv::minMaxLoc(disparity, &minVal, &maxVal, nullptr, nullptr, validMask);
    std::cout << "Kendi SGM'imiz -> sure: " << ms << " ms, "
              << "disparity araligi: [" << minVal << ", " << maxVal << "], "
              << "P1=" << P1 << ", P2=" << P2 << std::endl;

    cv::Mat dispColor = colorizeDisparity(disparity, validMask);

    cv::Mat leftCell  = withLabel(leftBGR,   "Sol (col1)");
    cv::Mat rightCell = withLabel(rightBGR,  "Sag (col3)");
    cv::Mat dispCell  = withLabel(dispColor, "Kendi SGM Disparity (JET)");

    cv::Mat grid;
    cv::hconcat(std::vector<cv::Mat>{leftCell, rightCell, dispCell}, grid);

    std::string outPath = "/content/gorev4_sgm_disparity.png";
    cv::imwrite(outPath, grid);
    std::cout << "Sonuc kaydedildi: " << outPath << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week10_task4_sgm.cpp -o week10_task4_sgm \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week10_task4_sgm
