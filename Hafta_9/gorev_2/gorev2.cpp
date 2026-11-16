%%bash
cat << 'EOF' > week9_task2_bmp_analysis.cpp
#include <iostream>
#include <fstream>
#include <iomanip>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// Dosyanın diskte kapladığı gerçek boyutu (Byte cinsinden) okuyan fonksiyon
std::uintmax_t getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open()) return 0;
    return file.tellg();
}

int main() {
    // 1. Doğrudan 1000x1000 boyutundaki renkli BMP dosyasını aç
    std::string color_bmp_path = "/content/image1.bmp";
    cv::Mat color_bmp = cv::imread(color_bmp_path, cv::IMREAD_COLOR);

    if (color_bmp.empty()) {
        std::cerr << "[HATA] " << color_bmp_path << " bulunamadı! Lütfen görseli /content/image1.bmp olarak yükleyin." << std::endl;
        return -1;
    }

    // 1000x1000 değilse boyutlandır
    if (color_bmp.cols != 1000 || color_bmp.rows != 1000) {
        cv::resize(color_bmp, color_bmp, cv::Size(1000, 1000), 0, 0, cv::INTER_CUBIC);
    }

    // ÖNEMLİ: İndirilen orijinal dosya .bmp uzantılı olsa da içeriği sıkıştırılmış
    // olabilir (örn. aslında JPEG). Gerçek, ham/sıkıştırılmamış bir BMP ile
    // kıyaslama yapabilmek için görseli YENİ bir dosyaya, OpenCV ile BMP olarak
    // yeniden kaydediyoruz ve ölçümü bu dosyadan yapıyoruz.
    std::string color_bmp_saved_path = "/content/color_1000x1000.bmp";
    cv::imwrite(color_bmp_saved_path, color_bmp);

    // Masaüstü ortamında çalıştırılıyorsa ekranda göster (Colab'da hata vermemesi için kapalı)
    // cv::imshow("1000x1000 Renkli BMP", color_bmp);
    // cv::waitKey(0);

    // 2. Gri tonlu (Grayscale) resme dönüştür ve BMP formatında kaydet
    cv::Mat gray_bmp;
    cv::cvtColor(color_bmp, gray_bmp, cv::COLOR_BGR2GRAY);

    std::string gray_bmp_path = "/content/gray_1000x1000.bmp";
    cv::imwrite(gray_bmp_path, gray_bmp);

    // 2b. Hocanın istediği JPEG kıyaslaması: renkli ve gri görselleri JPEG olarak da kaydet
    std::string color_jpg_path = "/content/color_1000x1000.jpg";
    std::string gray_jpg_path  = "/content/gray_1000x1000.jpg";
    cv::imwrite(color_jpg_path, color_bmp);
    cv::imwrite(gray_jpg_path, gray_bmp);

    // 3. Disk Boyutu, Çözünürlük ve Kanal Sayısı Kıyaslaması
    uintmax_t color_disk_bytes = getFileSize(color_bmp_saved_path);
    uintmax_t gray_disk_bytes  = getFileSize(gray_bmp_path);
    uintmax_t color_jpg_bytes  = getFileSize(color_jpg_path);
    uintmax_t gray_jpg_bytes   = getFileSize(gray_jpg_path);

    size_t color_raw_bytes = color_bmp.cols * color_bmp.rows * color_bmp.channels();
    size_t gray_raw_bytes  = gray_bmp.cols * gray_bmp.rows * gray_bmp.channels();

    std::cout << "=====================================================================" << std::endl;
    std::cout << "      HAFTA 9 - GÖREV 2: BMP ÇÖZÜNÜRLÜK VE KANAL BOYUT ANALİZİ       " << std::endl;
    std::cout << "=====================================================================" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "1. RENKLİ BMP (color_1000x1000.bmp - 3 Kanal, OpenCV ile yeniden kaydedildi):" << std::endl;
    std::cout << "   - Çözünürlük         : " << color_bmp.cols << " x " << color_bmp.rows << " piksel" << std::endl;
    std::cout << "   - Kanal Sayısı       : " << color_bmp.channels() << " (24-bit: B, G, R)" << std::endl;
    std::cout << "   - Teorik Veri Boyutu : " << color_raw_bytes << " Byte (" << color_raw_bytes / (1024.0 * 1024.0) << " MB)" << std::endl;
    std::cout << "   - Diskteki Boyut     : " << color_disk_bytes << " Byte (" << color_disk_bytes / (1024.0 * 1024.0) << " MB)" << std::endl;
    std::cout << "   - BMP Başlık (Header): " << (static_cast<intmax_t>(color_disk_bytes) - static_cast<intmax_t>(color_raw_bytes)) << " Byte" << std::endl;

    std::cout << "---------------------------------------------------------------------" << std::endl;

    std::cout << "2. GRİ TONLU BMP (gray_1000x1000.bmp - 1 Kanal):" << std::endl;
    std::cout << "   - Çözünürlük         : " << gray_bmp.cols << " x " << gray_bmp.rows << " piksel" << std::endl;
    std::cout << "   - Kanal Sayısı       : " << gray_bmp.channels() << " (8-bit: Tek Kanal)" << std::endl;
    std::cout << "   - Teorik Veri Boyutu : " << gray_raw_bytes << " Byte (" << gray_raw_bytes / (1024.0 * 1024.0) << " MB)" << std::endl;
    std::cout << "   - Diskteki Boyut     : " << gray_disk_bytes << " Byte (" << gray_disk_bytes / (1024.0 * 1024.0) << " MB)" << std::endl;
    std::cout << "   - BMP Başlık+Palet   : " << (static_cast<intmax_t>(gray_disk_bytes) - static_cast<intmax_t>(gray_raw_bytes)) << " Byte" << std::endl;

    std::cout << "---------------------------------------------------------------------" << std::endl;
    std::cout << "3. JPEG KARŞILAŞTIRMASI (sıkıştırmalı format):" << std::endl;
    std::cout << "   - Renkli JPEG Boyutu : " << color_jpg_bytes << " Byte (" << color_jpg_bytes / (1024.0 * 1024.0) << " MB)" << std::endl;
    std::cout << "   - Gri JPEG Boyutu    : " << gray_jpg_bytes  << " Byte (" << gray_jpg_bytes  / (1024.0 * 1024.0) << " MB)" << std::endl;
    std::cout << "   - Renkli: BMP/JPEG Oranı : " << static_cast<double>(color_disk_bytes) / color_jpg_bytes << " kat (BMP kaç kat büyük)" << std::endl;
    std::cout << "   - Gri   : BMP/JPEG Oranı : " << static_cast<double>(gray_disk_bytes)  / gray_jpg_bytes  << " kat (BMP kaç kat büyük)" << std::endl;

    std::cout << "---------------------------------------------------------------------" << std::endl;
    std::cout << "4. KIYASLAMA ÖZETİ:" << std::endl;
    std::cout << "   - Boyut Oranı (Renkli BMP / Gri BMP): " << static_cast<double>(color_disk_bytes) / gray_disk_bytes << " kat" << std::endl;
    std::cout << "=====================================================================" << std::endl;

    return 0;
}
EOF

# Derle ve Çalıştır
g++ -std=c++17 -O3 week9_task2_bmp_analysis.cpp -o week9_task2_bmp_analysis \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui \
    -Wl,-rpath,/content/opencv_install/lib

./week9_task2_bmp_analysis