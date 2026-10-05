LABORATUVAR RAPORU: KÜMÜLATİF DAĞILIM FONKSİYONU (CDF) İLE HİSTOGRAM EŞİTLEME

1. AMAÇ
Düşük kontrastlı tek kanallı (grayscale) görüntünün dinamik aralığını, OpenCV'nin hazır fonksiyonları (equalizeHist, calcHist vb.) kullanılmadan C++ ile genişletmek.

2. YÖNTEM VE MATEMATİKSEL MODEL
- Histogram: 256 elemanlı dizide her gri seviyenin piksel sayısı frekans olarak toplandı.
- CDF: Kümülatif toplam alındı: CDF[i] = CDF[i-1] + Hist[i]
- Dönüşüm (LUT): round(((CDF[v] - CDF_min) / (Toplam_Piksel - CDF_min)) * 255)
- Güncelleme: Görüntüdeki her piksel, hesaplanan LUT tablosu üzerinden yeniden haritalandı.

3. SONUÇ VE DOĞRULAMA
- Orijinal görüntüdeki dar piksel aralığı [0, 255] bandına yayıldı, görsel kontrast ve doku detayları belirginleşti.
- Kod, OpenCV'nin yerleşik cv::equalizeHist fonksiyonu ile mutlak fark (absdiff) testine sokulmuş; maksimum piksel farkı 0, ortalama piksel farkı 0.0000 olarak doğrulanmıştır.