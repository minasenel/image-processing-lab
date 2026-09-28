--------------------------------------------------------------------------------
GÖREV 4: 4 PARÇALI PARALEL HİSTOGRAM HESAPLAMA & İNDİRGEME (MAP-REDUCE)
--------------------------------------------------------------------------------
1. Konusu ve Amacı:
   Görüntünün parlaklık dağılımını 4 ROI dilimi üzerinden çok çekirdekle çıkarmak, 
   mutex kilitlenme maliyetini önlemek için yerel histogramlar kullanmak ve 
   sonuçları ana histogramda birleştirmek.

2. Kodda Yapılan İşlemler:
   - Her çekirdeğe özel std::vector<int> local_hist(256, 0) tanımlandı (Race condition önlendi).
   - Çekirdekler kendi dilimlerindeki parlaklıkları saydıktan sonra ana thread 
     üzerinde Reduction (İndirgeme) yapılarak global_histogram elde edildi.
   - Toplam piksel sayısı sağlama kontrolünden geçirildi.

3. Sonuçlar:
   - Girdi Boyutu     : 512 x 512 (262.144 piksel)
   - Çekirdek Payı    : 4 x 65.536 piksel
   - Örnek Frekanslar : Parlaklık 0 -> 0 adet | Parlaklık 64 -> 5104 adet
   - Doğrulama        : Histogram toplamı (262.144) == Resim piksel sayısı


------------------------------------------------------
Gorsel Boyutu : 512x512 (262144 piksel)

[Cekirdek 0] ROI islendi. Bu parcadaki piksel sayisi: 65536 (Beklenen: 65536)
[Cekirdek 1] ROI islendi. Bu parcadaki piksel sayisi: 65536 (Beklenen: 65536)
[Cekirdek 2] ROI islendi. Bu parcadaki piksel sayisi: 65536 (Beklenen: 65536)
[Cekirdek 3] ROI islendi. Bu parcadaki piksel sayisi: 65536 (Beklenen: 65536)

======================================================
                 HISTOGRAM SONUCLARI                  
======================================================
Ornek bazi parlaklik degerleri ve piksel adetleri:
- Parlaklik 0   (Tam Siyah) : 0 adet piksel
- Parlaklik 64  (Koyu Gri)  : 5104 adet piksel
- Parlaklik 128 (Orta Gri)  : 1603 adet piksel
- Parlaklik 192 (Acik Gri)  : 1036 adet piksel
- Parlaklik 255 (Tam Beyaz) : 0 adet piksel
------------------------------------------------------
Histogram Toplam Piksel Sayisi : 262144
Resmin Gercek Piksel Sayisi   : 262144

[DOGRULAMA BASARILI]: Histogram toplami resmin toplam piksel sayisina tam esittir!
======================================================
