--------------------------------------------------------------------------------
GÖREV 6: BİT MASKELEME İLE 4-KADRAN (QUADRANT) KUANTALAMA & LATENCY ANALİZİ
--------------------------------------------------------------------------------
1. Konusu ve Amacı:
   Aritmetik bölme ve çarpma işlemleri yerine doğrudan donanım seviyesinde bit 
   kaydırma ve mantıksal VE (AND) maskelemesi uygulayarak kuantalama yapmak. 
   Görüntüyü 4 eşit kadrana (sol-üst, sağ-üst, sol-alt, sağ-alt) bölerek tek 
   çekirdek (sequential) ve 4 iş parçacıklı (std::thread) paralel çalışma 
   sürelerini (gecikme / latency) yüksek çözünürlüklü görüntü üzerinde kıyaslamak.

2. Uygulanan Yöntem ve Kodlama Detayları:
   - Giriş Görseli: 3840 x 2160 (8.294.400 piksel) çözünürlüğünde gri görüntü (image.jpeg).
   - Bit Maskeleme Mantığı:
     * 4-bit derinlik (16 gri seviye) hedefi için:
       mask = static_cast<uchar>(0xFF << (8 - 4))
     * İşlem sonucu '11110000' (onluk tabanda 240) maskesi elde edilir.
     * Her piksele uygulanan 'p[j] &= mask' işlemi, pikselin en önemsiz (LSB) 
       4 bitini tek bir işlemci çevriminde sıfırlar ve sadece en önemli 4 biti bırakır.
   - 4-Kadran (Quadrant) Bölümleme:
     * Görüntü boyutları tek/çift piksel kaybı yaşanmaması için şu şekilde bölündü:
       w1 = cols / 2,  w2 = cols - w1
       h1 = rows / 2,  h2 = rows - h1
     * cv::Rect kullanılarak derin kopyalama yapılmadan 4 alt matris referansı oluşturuldu:
       - Kadran 1 (Sol-Üst) : Rect(0, 0, w1, h1)
       - Kadran 2 (Sağ-Üst) : Rect(w1, 0, w2, h1)
       - Kadran 3 (Sol-Alt) : Rect(0, h1, w1, h2)
       - Kadran 4 (Sağ-Alt) : Rect(w1, h1, w2, h2)
   - Performans Ölçümü:
     * std::chrono::high_resolution_clock kullanılarak hem tek çekirdek hem de 
       4 thread için işlem süreleri mikrosaniye cinsinden ölçüldü.

3. Sayısal Performans ve Latency Sonuçları:
   - İşlenen Piksel Hacmi               : 8.294.400 piksel (4K)
   - Tek Çekirdek (Sequential) Süresi   : 4087 mikrosaniye (~4.08 ms)
   - 4 Thread Paralel (Quadrant) Süresi : 4707 mikrosaniye (~4.70 ms)

4. Teknik Analiz ve Sonuç Değerlendirmesi:
   - Bit maskeleme (AND) işlemi CPU seviyesinde doğrudan kayıtçılar (registers) 
     üzerinde tek döngüde bittiği için 8.3 milyon piksellik devasa veri seti 
     tek çekirdekte yalnızca 4 milisaniyede tamamlanmıştır.
   - 4 ayrı std::thread başlatma, işletim sistemi çekirdeğine iş parçacığı 
     dağıtma (context switch) ve t.join() ile senkronizasyon bekleme maliyeti 
     yaklaşık 1 milisaniyelik bir yük (overhead) oluşturmuştur.
   - İşlem karmaşıklığı son derece düşük (O(1) bit işlemi) olduğunda, iş 
     parçacığı yönetim maliyetinin paralel hesaplama kazancını aşabileceği 
     deneysel olarak doğrulanmıştır.

5. Üretilen Çıktı Dosyaları:
   - quantized_image.png  (Tek çekirdek kuantalama çıktısı)
   - quantized_image1.png (4 thread paralel kadran kuantalama çıktısı)
--------------------------------------------------------------------------------