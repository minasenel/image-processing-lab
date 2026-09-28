--------------------------------------------------------------------------------
GÖREV 5: DOĞRUSAL PARLAKLIK VE KONTRAST DÖNÜŞÜMÜ & HİSTOGRAM TAŞMA ANALİZİ
--------------------------------------------------------------------------------
1. Konusu ve Amacı:
   Görüntü işlemede doğrusal piksel dönüşüm denklemi olan g(x,y) = α * f(x,y) + β 
   işleminin piksel yoğunlukları ve dinamik aralık üzerindeki etkilerini incelemek.
   Çarpma (α = 0.75) ve ekleme (β = 20) işlemlerinin histogram grafiğinde 
   yarattığı büzülme, kayma ve doygunluk (clipping/saturation) davranışlarını 
   test etmek.

2. Uygulanan Yöntem ve Kodlama Detayları:
   - Giriş Görseli: 3840 x 2160 çözünürlüğünde gri tonlamalı (CV_8UC1) görüntü.
   - Matematiksel Formülasyon:
     * α Katsayısı (0.75): Kontrastı daraltan ölçek katsayısı.
     * β Değeri (+20): Parlaklığı öteleyen bias/offset katsayısı.
   - Doygunluk Aritmetiği: Taşmaları (overflow ve underflow) engellemek adına 
     cv::saturate_cast<uchar>() fonksiyonu kullanılarak değerler [0, 255] 
     aralığında kilitlendi.
   - Histogram Hesabı: Her varyasyon için 256 bin'lik histogram vektörü 
     oluşturularak piksel frekansları çıkarıldı ve kritik aralıklar doğrulandı.

3. Test Edilen Senaryolar ve Sayısal Sonuçlar:

   A) YALNIZCA 0.75 İLE ÇARPMA (val * 0.75):
      - Teorik Beklenti : Maksimum parlaklık 255 * 0.75 = 191.25 değerine düşer.
                          Görüntünün dinamik aralığı daralır, kontrast azalır.
      - Ölçülen Sonuç   : [192, 255] aralığındaki piksel sayısı = 0
      - Değerlendirme   : Üst tonlar tamamen boşalmış, beyaz tonlar griye 
                          çekilerek kontrast düşüşü doğrulanmıştır.

   B) YALNIZCA 20 EKLEME (val + 20):
      - Teorik Beklenti : Tüm histogram şekli bozulmadan 20 birim sağa ötelenir.
                          En koyu piksel en az 20 olur. 235'ten büyük pikseller 
                          255 tavanına çarparak doymaya uğrar.
      - Ölçülen Sonuç   : [0, 19] aralığındaki piksel sayısı = 0
                          255 değerinde doyan piksel sayısı  = 10.859 adet
      - Değerlendirme   : Alt tonlar tamamen boşalarak siyah seviyesi yükselmiş, 
                          10.859 adet piksel tavana yapışarak (clipping) parlak 
                          detay kaybı oluşmuştur.

   C) BİRLİKTE UYGULAMA (val * 0.75 + 20):
      - Teorik Beklenti : Histogram hem 0.75 oranında büzülür hem de 20 birim 
                          sağa kayar. Yeni dinamik sınır [20, 211] aralığı olur.
      - Ölçülen Sonuç   : [0, 19] aralığındaki piksel sayısı   = 0
                          [212, 255] aralığındaki piksel sayısı = 0
      - Değerlendirme   : Görsel verisi tamamen [20, 211] bandına hapsedilmiştir. 
                          Hem en koyu siyahlar grimsi olmuş hem de en açık beyazlar 
                          bastırılarak soluk ve düşük kontrastlı bir görüntü elde 
                          edilmiştir.

4. Üretilen Çıktı Dosyaları:
   - custom_trans_only_075.png    (Kontrastı düşürülmüş görsel)
   - custom_trans_only_plus_20.png (Parlaklığı artırılmış ve doyurulmuş görsel)
   - custom_trans_075_plus_20.png  (Her iki dönüşümün birleşik sonucu)
--------------------------------------------------------------------------------