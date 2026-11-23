/*
Hafta 10 - Gorev 2: cv::StereoBM KULLANMADAN kendi blok eslestirme
fonksiyonumuzla stereo fark haritasi (disparity map) olusturma

Girdi (Gorev 1 ile ayni stereo cift, Colab'da /content/ icine yuklenir):
  scene1.row3.col1.ppm -> SOL goruntu
  scene1.row3.col3.ppm -> SAG goruntu

Yontem (OpenCV'nin hazir stereo fonksiyonlari KULLANILMADAN, elle yazildi):
1. Her piksel TEK BASINA degil, etrafindaki (2*HALF_WIN+1) x (2*HALF_WIN+1)
   boyutunda bir PENCERE (blok) ile ele alinir.
2. Sol goruntudeki (x, y) pikseli icin, sag goruntude AYNI SATIRDA (y sabit),
   d = 0'dan MAX_DISPARITY-1'e (16 deger: 0..15) kadar her aday kaymaya
   TEK TEK bakilir: (x, y) merkezli sol pencere ile (x-d, y) merkezli sag
   pencere karsilastirilir.
3. Karsilastirma, iki pencerenin karsilikli piksellerinin MUTLAK FARKLARININ
   TOPLAMI (SAD - Sum of Absolute Differences) ile yapilir. SAD ne kadar
   KUCUKSE pencereler o kadar BENZER, yani eslesme o kadar IYI demektir
   (fark aradigimiz icin kucuk skor = iyi eslesme).
4. 16 adayin icinden EN KUCUK SAD skorunu veren d degeri, o pikselin
   disparity degeri olarak secilir (winner-take-all / en iyi eslesen piksel).
5. Pencere goruntu disina tasan kenar pikselleri (ust/alt/sol/sag HALF_WIN
   kadar serit) ve d arttiginda sag goruntude pencere disina tasan bolgeler
   islenemez; bu alanlar gecersiz (siyah) birakilir.
6. Elde edilen disparity haritasi, Gorev 1'deki gibi COLORMAP_JET ile
   renklendirilir (kirmizi/sari = yakin, mavi = uzak) ve sol/sag goruntu
   ile yan yana kaydedilir.

Not: Bu, cv::StereoBM'in kendisinin de icinde yaptigi islemin (yerel/lokal
blok eslestirme, SAD maliyet fonksiyonu, winner-take-all) sifirdan, OpenCV'nin
hazir stereo sinifi kullanilmadan, sadece temel piksel erisimiyle yazilmis
halidir.
*/

%%bash
cat << 'EOF' > week10_task2_manual_block_matching.cpp
#include <iostream>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

static const int HALF_WIN = 3;        // pencere yaricapi -> (2*3+1)=7x7 blok
static const int MAX_DISPARITY = 16;  // d = 0..15 arasinda aranir

// Sol goruntudeki (xLeft, y) merkezli pencere ile sag goruntudeki
// (xRight, y) merkezli penceresinin SAD (mutlak farklarin toplami)
// skorunu hesaplar. Bu, OpenCV'nin hazir stereo fonksiyonu KULLANILMADAN
// yazilmis, odevin istedigi "kendi blok eslestirme fonksiyonu"dur.
double computeSAD(const cv::Mat& leftGray, const cv::Mat& rightGray,
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

    const int rows = leftGray.rows;
    const int cols = leftGray.cols;

    cv::Mat disparity = cv::Mat::zeros(rows, cols, CV_32F);
    cv::Mat validMask  = cv::Mat::zeros(rows, cols, CV_8U);

    // Pencere tasmasin diye kenarlardan HALF_WIN kadar icerde kaliyoruz.
    for (int y = HALF_WIN; y < rows - HALF_WIN; ++y) {
        for (int x = HALF_WIN; x < cols - HALF_WIN; ++x) {
            double bestSAD = std::numeric_limits<double>::max();
            int bestD = -1;

            for (int d = 0; d < MAX_DISPARITY; ++d) {
                int xr = x - d; // sag goruntudeki aday sutun (sola kaymis)
                if (xr - HALF_WIN < 0) break; // pencere sol kenardan tasti

                double sad = computeSAD(leftGray, rightGray, x, xr, y);
                if (sad < bestSAD) {
                    bestSAD = sad;
                    bestD = d;
                }
            }

            if (bestD >= 0) {
                disparity.at<float>(y, x) = (float)bestD;
                validMask.at<uchar>(y, x) = 255;
            }
        }
    }

    double minVal, maxVal;
    cv::minMaxLoc(disparity, &minVal, &maxVal, nullptr, nullptr, validMask);
    std::cout << "Disparity araligi: [" << minVal << ", " << maxVal << "] piksel" << std::endl;

    cv::Mat disp8u;
    disparity.convertTo(disp8u, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    disp8u.setTo(0, ~validMask);

    cv::Mat dispColor;
    cv::applyColorMap(disp8u, dispColor, cv::COLORMAP_JET);
    dispColor.setTo(cv::Scalar(0, 0, 0), ~validMask);

    auto withLabel = [](const cv::Mat& img, const std::string& text) {
        cv::Mat out = img.clone();
        cv::Mat bar(30, out.cols, CV_8UC3, cv::Scalar(30, 30, 30));
        cv::putText(bar, text, cv::Point(8, 21), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        cv::Mat cell;
        cv::vconcat(bar, out, cell);
        return cell;
    };

    cv::Mat leftCell  = withLabel(leftBGR,   "Sol (col1)");
    cv::Mat rightCell = withLabel(rightBGR,  "Sag (col3)");
    cv::Mat dispCell  = withLabel(dispColor, "Manuel Blok Eslestirme - SAD (JET)");

    cv::Mat grid;
    cv::hconcat(std::vector<cv::Mat>{leftCell, rightCell, dispCell}, grid);

    std::string outPath = "/content/gorev2_disparity.png";
    cv::imwrite(outPath, grid);
    std::cout << "Sonuc kaydedildi: " << outPath << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week10_task2_manual_block_matching.cpp -o week10_task2_manual_block_matching \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week10_task2_manual_block_matching
