# IMAGEPROC - Sayısal Görüntü İşleme ve Hızlandırma Laboratuvarı

Bu Repo, Sayısal Görüntü İşleme dersi kapsamında gerçekleştirilen temel görüntü işleme algoritmaları, çok çekirdekli CPU paralelleştirmeleri (`std::thread`, ROI) ve CUDA donanım hızlandırmalı uygulamaları içermektedir.

---

## 📁 Dizin Yapısı

```text
IMAGEPROC/
├── Hafta_1/
│   └── odev_1/
│       ├── odev1-cpu.cpp       # CPU tabanlı renk dönüşümü ve ölçekleme
│       ├── odev1-cuda.cpp      # CUDA GpuMat tabanlı donanım hızlandırmalı işlem
│       └── odev1-rapor.md      # Çalışma zamanı ve mimari analiz raporu
├── Hafta_2/
│   ├── gorev_1/                # CUDA ile Renk Dönüşümü ve Yeniden Boyutlandırma
│   ├── gorev_2/                # Tek Çekirdek (Sequential) Kuantalama (64 ve 85 katsayıları)
│   ├── gorev_3/                # 4 Dilimli ROI ve Çok İş Parçacıklı (std::thread) Kuantalama
│   ├── gorev_4/                # Paralel ROI Tabanlı Histogram Hesaplama ve İndirgeme (Reduction)
│   ├── gorev_5/                # Doğrusal Parlaklık/Kontrast Dönüşümü ve Histogram Taşma Analizi
│   └── gorev_6/                # Bit Düzeyinde Maskeleme ile 4-Kadran Kuantalama & Latency Kıyaslaması
├── Hafta_3/
├── Hafta_4/
├── Hafta_5/
├── Hafta_6/
└── Hafta_7/