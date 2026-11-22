/*
sign_01.png -> 1 kirmizi cember tespit edildi.
sign_02.png -> 1 kirmizi cember tespit edildi.
sign_03.png -> 1 kirmizi cember tespit edildi.
sign_04.png -> 1 kirmizi cember tespit edildi.
sign_05.png -> 1 kirmizi cember tespit edildi.
sign_06.png -> 1 kirmizi cember tespit edildi.
sign_07.png -> 1 kirmizi cember tespit edildi.
Toplam 7 gorsel islendi. Tek parca grid sonucu kaydedildi: "/content/traffic_signs_out/grid_sonuc.png"

Hafta 9 - Gorev 7: Kirmizi cember iceren trafik isaretlerinin tespiti
(https://www.kaggle.com/datasets/tabishalam0/traffic-signs-dataset-germany-uk-usa
 veri setinden kucuk bir alt kume kullanilmistir: /content/traffic_signs/*.jpg)

Yontem:
1. Goruntu HSV ve Lab renk uzaylarina donusturulur.
   - HSV'de kirmizi renk hem 0 derece civarinda hem de 180 derece civarinda
     (donen renk tekeri) bulundugu icin IKI ayri esik araligi (0-10 ve 170-180)
     kullanilip birlestirilir.
   - Lab uzayindaki 'a' kanali (yesil-kirmizi ekseni) ek bir dogrulama katmani
     olarak kullanilir: yuksek 'a' degeri kirmizi/magenta tonlari gosterir.
2. Iki renk uzayindan gelen maskeler AND ile birlestirilir, morfolojik
   islemlerle gurultu temizlenir.
3. Hough Circle Transform, hafifce yumusatilmis kirmizi maskenin KENDISI
   uzerinde calistirilir (griye cevrilmis orijinal goruntu degil). Bu sayede
   arama sadece kirmizi bolgelerin net kenarlarini gorur; dokudan/arka
   plandan gelen sahte kenarlarla ugrasmaz.
4. Bulunan her cember icin, kirmizi maskenin cember alaninin ne kadarini
   kapladigi iki farkli sekilde dogrulanir (dolu disk orani ve halka orani),
   cunku trafik isaretleri hem tamamen kirmizi dolu disk (dur isareti) hem de
   kirmizi halka + beyaz ic alan (hiz siniri/girilmez) seklinde olabilir.
   Dogrulamayi gecen cemberler orijinal goruntu uzerine cizilir.
5. Her goruntu icin 4 asama (orijinal | kirmizi maske | Hough'un kullandigi
   Canny kenar haritasi | tespit sonucu) yan yana bir satir olusturur; tum
   goruntulerin satirlari alt alta birlestirilip TEK bir grid goruntusu
   olarak kaydedilir (coklu cikti dosyasi yerine tek gorsel dayanak).

Klasor yapisi (Colab):
  /content/traffic_signs/   -> icine kaggle veri setinden secilen 10-20 ornek
                                 (dur, girilmez, hiz siniri vb. kirmizi cemberli
                                 isaret) .jpg/.png olarak atilir.
  /content/traffic_signs_out/grid_sonuc.png -> TEK parca sonuc grid'i buraya yazilir.
*/

%%bash
cat << 'EOF' > week9_task7_red_circle_signs.cpp
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fs = std::filesystem;

const cv::Size CELL_SIZE(220, 220);
const int LABEL_BAR_H = 24;

// Tek bir gorseli (gri/renkli farketmez) etiket barli, sabit boyutlu bir
// "hucre"ye donusturur. Grid'deki tum hucreler boylece ayni boyutta olur.
cv::Mat makeLabeledCell(const cv::Mat& img, const std::string& label) {
    cv::Mat colorImg;
    if (img.channels() == 1) cv::cvtColor(img, colorImg, cv::COLOR_GRAY2BGR);
    else colorImg = img;

    cv::Mat resized;
    cv::resize(colorImg, resized, CELL_SIZE);

    cv::Mat cell(CELL_SIZE.height + LABEL_BAR_H, CELL_SIZE.width, CV_8UC3, cv::Scalar(30, 30, 30));
    resized.copyTo(cell(cv::Rect(0, LABEL_BAR_H, CELL_SIZE.width, CELL_SIZE.height)));
    cv::putText(cell, label, cv::Point(6, 17), cv::FONT_HERSHEY_SIMPLEX, 0.45,
                cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    return cell;
}

// HSV + Lab birlesimiyle kirmizi bolge maskesi uretir.
cv::Mat redMask(const cv::Mat& bgr) {
    cv::Mat hsv, lab;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    cv::cvtColor(bgr, lab, cv::COLOR_BGR2Lab);

    // HSV'de kirmizi renk tekerin iki ucunda yer alir (0-10 ve 170-180)
    cv::Mat maskLow, maskHigh, maskHSV;
    cv::inRange(hsv, cv::Scalar(0, 90, 60), cv::Scalar(10, 255, 255), maskLow);
    cv::inRange(hsv, cv::Scalar(170, 90, 60), cv::Scalar(180, 255, 255), maskHigh);
    cv::bitwise_or(maskLow, maskHigh, maskHSV);

    // Lab 'a' kanali: 128 = notr, >150 kirmizi/magenta tonlarina isaret eder
    std::vector<cv::Mat> labCh;
    cv::split(lab, labCh);
    cv::Mat maskLab;
    cv::threshold(labCh[1], maskLab, 150, 255, cv::THRESH_BINARY);

    cv::Mat combined;
    cv::bitwise_and(maskHSV, maskLab, combined);

    cv::morphologyEx(combined, combined, cv::MORPH_OPEN,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    cv::morphologyEx(combined, combined, cv::MORPH_CLOSE,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9)));
    cv::dilate(combined, combined,
               cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)), cv::Point(-1, -1), 1);

    return combined;
}

// Bir gorseli isler ve sonucu grid'e eklenecek TEK bir satir (4 hucre:
// orijinal, kirmizi maske, Hough'un kullandigi kenar haritasi, tespit
// sonucu) olarak dondurur. Basarisiz olursa bos Mat doner.
cv::Mat processImage(const fs::path& inPath) {
    cv::Mat bgr = cv::imread(inPath.string(), cv::IMREAD_COLOR);
    if (bgr.empty()) {
        std::cerr << "[UYARI] okunamadi: " << inPath << std::endl;
        return cv::Mat();
    }

    cv::Mat mask = redMask(bgr);

    // Hough Circle Transform'u, griye cevrilmis orijinal goruntu yerine
    // dogrudan (hafifce yumusatilmis) kirmizi maskenin kendisi uzerinde
    // calistiriyoruz. Boylece arama SADECE kirmizi bolgelerin net kenarlarini
    // gorur; dokudan/arka plandan gelen sahte kenarlar devreye girmez.
    cv::Mat maskBlurred;
    cv::GaussianBlur(mask, maskBlurred, cv::Size(5, 5), 0);

    const double cannyUpper = 60.0;  // Canny ust esigi (param1)
    const double cannyLower = cannyUpper / 2.0;

    // HoughCircles icinde Canny kenarlarini kullanir; burada AYNI esiklerle
    // o kenar haritasini kendimiz de uretip gorsellestiriyoruz. Bu, cember
    // aramasinin HANGI kenarlar uzerinden yapildigina dair gorsel dayanaktir.
    cv::Mat houghEdges;
    cv::Canny(maskBlurred, houghEdges, cannyLower, cannyUpper);

    std::vector<cv::Vec3f> circles;
    double minDist = std::max(bgr.rows, bgr.cols) / 12.0;
    cv::HoughCircles(maskBlurred, circles, cv::HOUGH_GRADIENT,
                      1.0,        // dp
                      minDist,    // merkezler arasi min uzaklik
                      cannyUpper, // Canny ust esigi (param1)
                      25.0,       // merkez tespit esigi (param2)
                      10,         // min yaricap
                      std::min(bgr.rows, bgr.cols) / 2); // max yaricap

    cv::Mat result = bgr.clone();
    std::vector<cv::Vec3i> accepted; // dogrulanmis, tekillestirilmis cemberler
    for (const auto& c : circles) {
        cv::Point center(cvRound(c[0]), cvRound(c[1]));
        int radius = cvRound(c[2]);
        if (radius <= 0) continue;

        // Yanlis pozitifleri eleme: trafik isaretleri ya TAMAMEN kirmizi disk
        // (dur isaretinin govdesi gibi) ya da kirmizi HALKA (hiz siniri/girilmez
        // isaretlerindeki beyaz ic alanli kirmizi cember) seklinde olur.
        // Bu yuzden iki ayri orani da kontrol ediyoruz.
        cv::Mat diskMask = cv::Mat::zeros(mask.size(), CV_8UC1);
        cv::circle(diskMask, center, radius, cv::Scalar(255), -1);
        cv::Mat diskOverlap;
        cv::bitwise_and(mask, diskMask, diskOverlap);
        double diskRatio = cv::countNonZero(diskOverlap) / (double)std::max(1, cv::countNonZero(diskMask));

        int ringThickness = std::max(4, (int)std::round(radius * 0.22));
        cv::Mat ringMask = cv::Mat::zeros(mask.size(), CV_8UC1);
        cv::circle(ringMask, center, radius, cv::Scalar(255), ringThickness);
        cv::Mat ringOverlap;
        cv::bitwise_and(mask, ringMask, ringOverlap);
        double ringRatio = cv::countNonZero(ringOverlap) / (double)std::max(1, cv::countNonZero(ringMask));

        bool isFilledRedDisk = diskRatio >= 0.55;
        bool isRedRing = ringRatio >= 0.45;
        if (!isFilledRedDisk && !isRedRing) continue;

        // Ayni isareti birden fazla kez isaretlemeyi onle (ornegin oval
        // isaretlerde Hough birbirine cok yakin birkac cember uretebilir):
        // merkezleri birbirine yakin, dogrulanmis bir cember zaten varsa atla.
        bool duplicate = false;
        for (const auto& a : accepted) {
            double dist = cv::norm(cv::Point(a[0], a[1]) - center);
            if (dist < 0.5 * (a[2] + radius)) { duplicate = true; break; }
        }
        if (duplicate) continue;

        accepted.push_back(cv::Vec3i(center.x, center.y, radius));
    }

    for (const auto& a : accepted) {
        cv::Point center(a[0], a[1]);
        int radius = a[2];
        cv::circle(result, center, radius, cv::Scalar(0, 255, 0), 3);
        cv::circle(result, center, 3, cv::Scalar(255, 0, 0), -1);
    }
    int found = (int)accepted.size();
    std::cout << inPath.filename().string() << " -> " << found
              << " kirmizi cember tespit edildi." << std::endl;

    // Tum goruntuler icin TEK bir sonuc dosyasi istendigi icin, her goruntu
    // ayri ayri kaydedilmiyor; bunun yerine 4 asamayi (orijinal, maske,
    // Hough kenarlari, sonuc) yan yana gosteren bir satir uretiliyor. main()
    // butun satirlari alt alta birlestirip TEK bir grid goruntusu kaydeder.
    cv::Mat origCell   = makeLabeledCell(bgr,        inPath.filename().string());
    cv::Mat maskCell   = makeLabeledCell(mask,       "Kirmizi Maske (HSV+Lab)");
    cv::Mat edgeCell   = makeLabeledCell(houghEdges, "Hough Kenarlari (Canny)");
    cv::Mat resultCell = makeLabeledCell(result,     "Tespit Sonucu: " + std::to_string(found) + " cember");

    cv::Mat row;
    cv::hconcat(std::vector<cv::Mat>{origCell, maskCell, edgeCell, resultCell}, row);
    return row;
}

int main() {
    fs::path inDir = "/content/traffic_signs";
    fs::path outDir = "/content/traffic_signs_out";

    if (!fs::exists(inDir)) {
        std::cerr << "[HATA] " << inDir << " klasoru bulunamadi. "
                  << "Kaggle veri setinden secilen ornek gorselleri bu klasore yukleyin."
                  << std::endl;
        return -1;
    }

    // Dosyalari isme gore sirala ki grid her calistirmada ayni sirada olsun.
    std::vector<fs::path> paths;
    for (const auto& entry : fs::directory_iterator(inDir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp") {
            paths.push_back(entry.path());
        }
    }
    std::sort(paths.begin(), paths.end());

    std::vector<cv::Mat> rows;
    for (const auto& p : paths) {
        cv::Mat row = processImage(p);
        if (!row.empty()) rows.push_back(row);
    }

    if (rows.empty()) {
        std::cerr << "[HATA] Islenebilen gorsel bulunamadi." << std::endl;
        return -1;
    }

    cv::Mat grid;
    cv::vconcat(rows, grid);

    fs::create_directories(outDir);
    fs::path gridPath = outDir / "grid_sonuc.png";
    cv::imwrite(gridPath.string(), grid);

    std::cout << "Toplam " << rows.size() << " gorsel islendi. "
              << "Tek parca grid sonucu kaydedildi: " << gridPath << std::endl;
    return 0;
}
EOF

g++ -std=c++17 -O3 week9_task7_red_circle_signs.cpp -o week9_task7_red_circle_signs \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task7_red_circle_signs
