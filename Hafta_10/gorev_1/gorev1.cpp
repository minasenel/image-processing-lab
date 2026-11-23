/*
Hafta 10 - Gorev 1: Stereo eslestirme ile fark haritasi (disparity map)

Girdi (ayni klasorde bulunan stereo cift, Colab'da /content/ icine yuklenir):
  scene1.row3.col1.ppm -> SOL goruntu  (kamera 1)
  scene1.row3.col3.ppm -> SAG goruntu  (kamera 3, ayni satirda daha sagda)

Yontem:
1. Her iki goruntu griye cevrilir (StereoBM gri goruntu ister).
2. cv::StereoBM ile piksel bazinda fark (disparity) hesaplanir: LOKAL blok
   eslestirme yontemidir - her piksel icin sadece kendi kucuk penceresindeki
   (blockSize x blockSize) bilgiye bakarak sag goruntude ayni satirda en
   iyi eslesen pikselin KAC PIKSEL SOLA kaydigi bulunur (StereoSGBM gibi
   satir boyunca komsu pikseller arasinda global/yari-global bir yumusatma
   YAPMAZ, sadece yerel pencere benzerligine gore karar verir).
3. Ham disparity (16-bit, 4 ondalik sabit nokta) normalize edilip COLORMAP_JET
   ile renklendirilir: SICAK (kirmizi/sari) = buyuk disparity = KAMERAYA YAKIN,
   SOGUK (mavi) = kucuk disparity = KAMERADAN UZAK. Eslesme bulunamayan
   (dokusuz/kapanma) bolgeler siyah/gecersiz birakilir.
4. Sol goruntu, sag goruntu ve renkli disparity haritasi yan yana tek bir
   gorsel olarak kaydedilir.

Yorum (soruya cevap, ayrica gorev1-rapor.md'de):
- Renkler MUTLAK derinligi degil, GORECELI (bagil) derinligi gosterir:
  disparity = (f * baseline) / derinlik oldugundan buyuk disparity -> yakin,
  kucuk disparity -> uzak. Ancak kamera odak uzakligi (f) ve iki kamera
  arasindaki mesafe (baseline) bilinmeden piksel disparity'si METRE/CM gibi
  gercek bir derinlik birimine CEVRILEMEZ; sadece "hangi nokta digerinden
  daha yakin/uzak" seklinde SIRALI bir bilgi verir.
- Ayrica tek renkli/dokusuz yuzeylerde (ornegin duz beyaz duvar), kapanma
  (occlusion) bolgelerinde ve parlak/yansitici alanlarda eslesme basarisiz
  olur; bu bolgelerde harita gurultulu veya gecersiz cikar. Yani bu yontem
  gercek bir derinlik sensorunden (LiDAR, ToF) degil, SADECE goruntu
  eslesmesinden gelen, kalibrasyon yapilmadan olcekli (metrik) olmayan bir
  "yakin-uzak" tahmini saglar.
*/

%%bash
cat << 'EOF' > week10_task1_disparity.cpp
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

int main() {
    cv::Mat left  = cv::imread("/content/scene1.row3.col1.ppm", cv::IMREAD_COLOR);
    cv::Mat right = cv::imread("/content/scene1.row3.col3.ppm", cv::IMREAD_COLOR);

    if (left.empty() || right.empty()) {
        std::cerr << "[HATA] Stereo cift okunamadi. scene1.row3.col1.ppm ve "
                  << "scene1.row3.col3.ppm dosyalarini /content/ icine yukleyin."
                  << std::endl;
        return -1;
    }

    cv::Mat leftGray, rightGray;
    cv::cvtColor(left,  leftGray,  cv::COLOR_BGR2GRAY);
    cv::cvtColor(right, rightGray, cv::COLOR_BGR2GRAY);

    // StereoBM: LOKAL (yerel) blok eslestirme. Her piksel icin yalnizca
    // kendi blockSize x blockSize penceresindeki yogunluk deseni, sag
    // goruntudeki ayni pencereyle karsilastirilir; en dusuk fark (SAD)
    // veren kayma o pikselin disparity degeri olarak secilir. Komsu
    // piksellerin kararlari birbirini ETKILEMEZ (global/yari-global bir
    // yumusatma veya enerji minimizasyonu yoktur) - bu yuzden "lokal" yontem
    // denir; StereoSGBM'den (yari-global) daha hizli ama daha gurultulu/
    // bosluklu (dokusuz bolgelerde eslesme bulamayan) sonuc verir.
    int numDisparities = 16 * 6; // 96, 16'nin kati olmak zorunda
    int blockSize = 15;          // tek sayi, 5-21 araliginda tipik

    cv::Ptr<cv::StereoBM> bm = cv::StereoBM::create(numDisparities, blockSize);
    bm->setPreFilterCap(31);
    bm->setTextureThreshold(10);
    bm->setUniquenessRatio(10);
    bm->setSpeckleWindowSize(100);
    bm->setSpeckleRange(2);
    bm->setDisp12MaxDiff(1);
    bm->setMinDisparity(0);

    cv::Mat dispRaw; // CV_16S, gercek disparity * 16 (sabit nokta)
    bm->compute(leftGray, rightGray, dispRaw);

    cv::Mat disp32f;
    dispRaw.convertTo(disp32f, CV_32F, 1.0 / 16.0);

    // Gecersiz/eslesememis pikseller (negatif disparity) maskelenir.
    cv::Mat validMask = disp32f > 0;

    double minVal, maxVal;
    cv::minMaxLoc(disp32f, &minVal, &maxVal, nullptr, nullptr, validMask);
    std::cout << "Disparity araligi: [" << minVal << ", " << maxVal << "] piksel" << std::endl;

    cv::Mat disp8u;
    disp32f.convertTo(disp8u, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    disp8u.setTo(0, ~validMask);

    cv::Mat dispColor;
    cv::applyColorMap(disp8u, dispColor, cv::COLORMAP_JET);
    dispColor.setTo(cv::Scalar(0, 0, 0), ~validMask);

    cv::Mat label(30, left.cols, CV_8UC3, cv::Scalar(30, 30, 30));
    auto withLabel = [](const cv::Mat& img, const std::string& text) {
        cv::Mat out = img.clone();
        cv::Mat bar(30, out.cols, CV_8UC3, cv::Scalar(30, 30, 30));
        cv::putText(bar, text, cv::Point(8, 21), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        cv::Mat cell;
        cv::vconcat(bar, out, cell);
        return cell;
    };

    cv::Mat leftCell  = withLabel(left,      "Sol (col1)");
    cv::Mat rightCell = withLabel(right,     "Sag (col3)");
    cv::Mat dispCell  = withLabel(dispColor, "StereoBM Disparity (JET: kirmizi=yakin, mavi=uzak)");

    cv::Mat grid;
    cv::hconcat(std::vector<cv::Mat>{leftCell, rightCell, dispCell}, grid);

    std::string outPath = "/content/gorev1_disparity.png";
    cv::imwrite(outPath, grid);
    std::cout << "Sonuc kaydedildi: " << outPath << std::endl;

    return 0;
}
EOF

g++ -std=c++17 -O3 week10_task1_disparity.cpp -o week10_task1_disparity \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d \
    -Wl,-rpath,/content/opencv_install/lib

./week10_task1_disparity
