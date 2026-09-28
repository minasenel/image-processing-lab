--------------------------------------------------------------------------------
GÖREV 3: ROI (REGION OF INTEREST) BÖLÜNTÜLEME VE ÇOK İŞ PARÇACIKLI KUANTALAMA
--------------------------------------------------------------------------------
1. Konusu ve Amacı:
   Görüntüyü 4 bağımsız ilgi alanına (ROI) bölerek std::thread yardımıyla 4 CPU 
   çekirdeğinde paralel kuantalama yapmak ve çekirdek yük dağılımını izlemek.

2. Kodda Yapılan İşlemler:
   - cv::Rect kullanılarak matris üzerinde kopyalama (deep copy) yapılmaksızın 
     aynı bellek bloğunu işaret eden 4 yatay dilim oluşturuldu.
   - 4 ayrı worker thread çalıştırılarak veri yarışması (data race) olmadan 
     bağımsız bellek bölgeleri eşzamanlı işlendi.
   - join() ile tüm iş parçacıklarının bitişi ana thread tarafından senkronize edildi.

3. Sonuçlar:
   - Parça Boyutları  : Her çekirdek için 512 x 128 piksel
   - Çekirdek 0..3    : Tümü 65.536'şar pikseli eşzamanlı tamamladı.
   - Üretilen Görsel  : woman_roi_multithread_quantized.png
  

------------------------------------------
Resim Boyutu: 512x512
Resim 4 farkli ROI parcasina bolunup 4 ayri CPU cekirdegine gonderiliyor...

[Cekirdek 1] ROI basariyla islendi -> Y: 128 | Yukseklik: 128 | Genislik: 512
[Cekirdek 0] ROI basariyla islendi -> Y: 0 | Yukseklik: 128 | Genislik: 512
[Cekirdek 3] ROI basariyla islendi -> Y: 384 | Yukseklik: 128 | Genislik: 512
[Cekirdek 2] ROI basariyla islendi -> Y: 256 | Yukseklik: 128 | Genislik: 512
sonuç: woman_multi_thread_quantized.png

Tum parcalar birlestirildi ve kaydedildi: woman_roi_multithread_quantized.png
  adding: woman_roi_multithread_quantized.png (deflated 9%)
-----------------------------------------------

# latency ( zaman ölçümü )
## todo::
latency süresi ölçerken sorun yaşadım. resim çözünürlüğünü büyütüp döngü ile 10 kez çalıştırıp ortalama süreyi alacağım. 

