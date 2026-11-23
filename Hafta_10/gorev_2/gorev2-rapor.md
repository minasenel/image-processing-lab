# Hafta 10 - Görev 2: Kendi Blok Eşleştirme Fonksiyonum (StereoBM Kullanmadan)

## Fikir
Görev 1'de `cv::StereoBM` hazır sınıfını kullandık. Görev 2'de aynı işi,
OpenCV'nin stereo fonksiyonlarına **hiç dokunmadan**, kendi yazdığımız
`computeSAD()` fonksiyonuyla elle yaptık.

## `computeSAD()` fonksiyonu ne yapıyor?
```cpp
double computeSAD(leftGray, rightGray, xLeft, xRight, y)
```
Bu fonksiyon, **tek bir piksel** değil, o pikselin etrafındaki **7×7'lik bir
pencereyi** (`HALF_WIN = 3` → sağa/sola/yukarı/aşağı 3'er piksel) karşılaştırır:

- Sol görüntüde `(xLeft, y)` merkezli 7×7 pencere alınır.
- Sağ görüntüde `(xRight, y)` merkezli 7×7 pencere alınır (aynı satır, `y` sabit).
- Pencerelerin **49 karşılıklı pikseli** tek tek gezilir, her çift için
  `|solPiksel - sağPiksel|` (mutlak fark) hesaplanır ve hepsi toplanır.

Bu toplam, **SAD (Sum of Absolute Differences)** skorudur. SAD ne kadar
**küçükse**, iki pencere o kadar **benzer** demektir — yani fark aradığımız
için düşük skor = iyi eşleşme, yüksek skor = kötü eşleşme.

## Disparity nasıl bulunuyor? (winner-take-all)
`main()` içinde, sol görüntüdeki her `(x, y)` pikseli için:

1. `d = 0`'dan `d = 15`'e kadar (toplam 16 aday, ödevin istediği
   "max disparity 16") **tek tek** denenir.
2. Her `d` için, sağ görüntüde `xr = x - d` sütunundaki pencere ile
   `computeSAD(x, xr, y)` çağrılıp skor hesaplanır.
3. 16 adayın içinden **en küçük SAD skorunu veren `d`** seçilir — buna
   "winner-take-all" (kazanan tek elemanı alır) denir, çünkü sadece en iyi
   skoru veren aday kazanıyor, diğerleri tamamen elenir.
4. Seçilen `d` değeri, o pikselin **disparity** değeri olarak `disparity`
   matrisine yazılır.

## Neden kenarlar boş (siyah) kalıyor?
- Pencere görüntü dışına taşmasın diye `y` ve `x` döngüleri `HALF_WIN` (3)
  piksel içeriden başlıyor ve bitiyor — üst/alt/sol/sağ kenarda 3 piksellik
  bir şerit hiç işlenmiyor.
- Ayrıca `d` büyüdükçe sağ görüntüdeki pencere sola kayar; `xr - HALF_WIN < 0`
  olduğunda (pencere sol kenardan taşacaksa) döngü `break` ile durur. Bu
  yüzden görüntünün en soldaki `MAX_DISPARITY` (16) piksellik şeridi de
  eşleşme denenemeden geçersiz (siyah) kalır — bu, `cv::StereoBM`'de de
  görülen, aramanın fiziksel olarak mümkün olmadığı aynı sınır etkisidir.

## Görev 1 (StereoBM) ile farkı
Mantık olarak ikisi de **lokal blok eşleştirme + SAD + winner-take-all**
yapıyor; fark, Görev 1'de bu işlemi OpenCV'nin optimize edilmiş, hazır
`cv::StereoBM` sınıfı yaparken, Görev 2'de aynı adımları **sıfırdan,
kendi döngülerimizle** yazmış olmamız. Sonuçta üretilen disparity
haritalarının görünümü (renk dağılımı, geçersiz bölgeler) bu yüzden
birbirine çok yakın olmalı.
