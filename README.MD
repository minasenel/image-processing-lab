# IMAGEPROC - Sayısal Görüntü İşleme ve Hızlandırma Laboratuvarı

Bu repo, Sayısal Görüntü İşleme dersi kapsamında gerçekleştirilen temel görüntü işleme algoritmaları, çok çekirdekli CPU paralelleştirmeleri (`std::thread`, ROI) ve CUDA donanım hızlandırmalı uygulamaları içermektedir.

---

## 📁 Dizin Yapısı ve Görevler

### Hafta_1
- **odev_1**: CPU tabanlı renk dönüşümü/ölçekleme ve CUDA GpuMat ile donanım hızlandırmalı aynı işlemin karşılaştırılması.

### Hafta_2
- **gorev_1**: CUDA ile renk dönüşümü ve yeniden boyutlandırma.
- **gorev_2**: Tek çekirdek (sıralı) kuantalama, 64 ve 85 katsayılarıyla.
- **gorev_3**: 4 dilimli ROI ve çok iş parçacıklı (`std::thread`) kuantalama.
- **gorev_4**: Paralel ROI tabanlı histogram hesaplama ve indirgeme (reduction).
- **gorev_5**: Doğrusal parlaklık/kontrast dönüşümü ve histogram taşma analizi.
- **gorev_6**: Bit düzeyinde maskeleme ile 4-kadran kuantalama ve gecikme (latency) kıyaslaması.

### Hafta_3
- **gorev_1**: Tek kanallı görsel üzerinde CUDA tabanlı temel işlem.
- **gorev_2**: Histogram eşitleme — OpenCV hazır fonksiyonu ile ve sıfırdan (custom) yazılan algoritmanın karşılaştırılması.
- **gorev_3**: CLAHE (Contrast Limited Adaptive Histogram Equalization) — hazır ve manuel uygulama.
- **gorev_4**: Manuel 2D konvolüsyon — kutu (ortalama) filtresi ve merkez ağırlıklı filtre.
- **gorev_5**: Medyan filtre — hazır ve sıfırdan (custom) uygulama, salt & pepper gürültüsü üzerinde.
- **sure_karsilastirma**: gorev_1-3 için CPU (çok iş parçacıklı) ve GPU çalışma süresi kıyaslaması.

### Hafta_4
- **gorev_1**: Gauss yumuşatma + Prewitt kenar bulma (konvolüsyon tabanlı).
- **gorev_2**: Doğrudan (direkt) yatay Prewitt konvolüsyonu.
- **gorev_3**: Gauss + Sobel kenar bulma.
- **gorev_4**: Prewitt ve Sobel yöntemlerinin karşılaştırma raporu.
- **gorev_5**: Laplacian filtre uygulaması.
- **gorev_6**: 7x7 Mexican Hat (LoG) konvolüsyon çekirdeği.
- **gorev_7**: Kenar bulma yöntemlerinin (Prewitt, Canny vb.) süre kıyaslaması (benchmark).
- **gorev_8 (ödev)**: Canny kenar bulma algoritmasının sıfırdan (manuel) yazılması.

### Hafta_5
- **gorev_1**: Renkli görsel için OpenCV `calcHist` ile RGB histogram hesaplama.
- **gorev_2**: RGB histogramının sıfırdan (manuel) hesaplanması ve çizdirilmesi.
- **gorev_3**: RGB → Gri seviye dönüşümü — hazır `cvtColor` ve manuel uygulama.
- **gorev_4**: BGR → HSV dönüşümü ve sayısal hata (SAD/MAE) karşılaştırma raporu.
- **gorev_5**: Gri seviyeye manuel dönüşüm ve X-ray görsellerinde pseudo-renklendirme (pseudocolor).

### Hafta_6
- **gorev_1**: Standart ve olasılıksal Hough Transform (`HoughLines` / `HoughLinesP`) ile doğru tespiti (sudoku görseli).
- **gorev_2**: Hough Transform'un sıfırdan (custom) akümülatör matrisiyle uygulanması.
- **gorev_3**: Hough Circle Transform ile çember tespiti.
- **gorev_4**: 3 boyutlu akümülatörle (x, y, yarıçap) sıfırdan Hough çember dönüşümü.
- **gorev_5**: Eş merkezli çemberler için dinamik eşikleme ve genişletilmiş NMS ile Hough iyileştirmesi.

### Hafta_7
- **gorev_1**: Gürültülü sinüsoidal sinyal üretimi.
- **gorev_2**: Sinyale gürültü ekleme fonksiyonu.
- **gorev_3**: Farklı filtreleme yöntemlerinin karşılaştırılması.
- **gorev_4**: Ayrık Fourier Dönüşümü (DFT) ile spektrum analizi ve sızma (spectral leakage) gözlemi.
- **gorev_5**: DFT spektrumunda tepe (peak) frekans tespiti.
- **gorev_6**: Spektral eşikleme ve IDFT (Ters DFT) ile gürültüden arındırılmış sinyal rekonstrüksiyonu.
- **gorev_7**: 2 boyutlu DFT — dikey yönde uygulama.
- **gorev_8**: 512x512 görsel üzerinde gürültülü 2D-DFT analizi.
- **gorev_9**: 2D-IDFT ile görsel rekonstrüksiyonu ve ZNCC/MSE/PSNR ile kalite raporu.
- **gorev_10**: Frekans alanında farklı filtre türlerinin uygulanması.
