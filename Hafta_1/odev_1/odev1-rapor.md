# İlk Ödev: Resmi Re-Size Etme 
Ödevde örnek resimleri, istenen (1024x768) boyutlara yeniden boyutlandırmak için CPU ve GPU (CUDA) kullanarak iki farklı yöntemle testler gerçekleştirdim. Aşağıda her iki yöntemin sonuçlarını ve performans karşılaştırmalarını bulabilirsiniz.

1. TEST ORTAMI VE DONANIM BILGILERI
--------------------------------------------------------------------------------
Yerel Ortam (CPU)   : Apple MacBook Pro (Apple Silicon)
                      - Derleyici: Clang++ (-std=c++17)
                      - Kutuphane: OpenCV 5.0.0 (C++ API)

Bulut Ortami (GPU)  : Google Colab (Linux Sanal Makine)
                      - GPU: NVIDIA Tesla T4 (16 GB VRAM, sm_75 mimarisi)
                      - Derleyici: G++ (-std=c++17) & NVCC (CUDA 12.8)
                      - Kutuphane: OpenCV 4.10.0 (Custom CUDA Build, C++ API)

Islem Parametreleri : Giris gorselleri taranarak 1024x768 piksel hedef boyutuna
                      interpolasyon ile yeniden boyutlandirilmistir (Resize).

# Google Colab Ortamında CUDA Sonuçları:

================ [COLAB CUDA RESIZE TESTI] ================
Dosya               Aktarim (ms)   Kernel (ms)    Toplam GPU (ms)
-----------------------------------------------------------
img1.jpg            3.15           0.22           3.37           
img3.jpg            2.27           0.14           2.41           
-----------------------------------------------------------
Ortalama Saf Kernel Suresi : 0.12 ms
Ortalama Uctan Uca Sure    : 1.93 ms

# CPU Sonuçları:
./resize_cpu
================ [YEREL CPU RESIZE TESTI] ================
Dosya               Islem Suresi (ms) 
-----------------------------------------------------------
img3.jpg            0.89              
img1.jpg            0.27              
-----------------------------------------------------------
Ortalama CPU Suresi: 0.58 ms

Ornek resimleri yeniden boyutlandırmak için hem CPU hem de GPU yöntemlerini kullanarak testler gerçekleştirdim. Sonuçlar, GPU'nun özellikle büyük resimlerde daha hızlı performans sağladığını göstermektedir. CUDA kullanımı ile kernel süresi oldukça düşük olup, toplam GPU süresi de oldukça hızlıdır. CPU yöntemi ise daha yavaş olmasına rağmen küçük resimler için yeterli performans sunmaktadır.

3. KARSILASTIRMALI ANALIZ TABLOSU
--------------------------------------------------------------------------------
+-------------------+---------------+-----------------+------------------------+
| Metrik            | CPU (Yerel)   | CUDA T4 (Colab) | Karsilastirma / Kazanc |
+-------------------+---------------+-----------------+------------------------+
| img1 (Saf Islem)  | 0.27 ms       | 0.22 ms         | CUDA %18.5 daha hizli  |
| img3 (Saf Islem)  | 0.89 ms       | 0.14 ms         | CUDA ~6.3x daha hizli  |
| Ortalama Hesaplama| 0.58 ms       | 0.12 ms         | CUDA ~4.8x daha hizli  |
| PCIe Veri Yolu Yuku| 0.00 ms      | 1.81 ms         | GPU veri tasima yukudur|
| Uctan Uca Toplam  | 0.58 ms       | 1.93 ms         | CPU ~3.3x daha avantajli|
+-------------------+---------------+-----------------+------------------------+


4. DEGERLENDIRME VE CIKARIMLAR
--------------------------------------------------------------------------------
1. Saf Hesaplama Basarimi (Kernel Execution):
   - Yuksek cozunurluklu gorselde (img3.jpg) CUDA cekirdekleri piksel bazli 
     paralel hesaplama avantajini net sekilde gostermis; CPU'nun 0.89 ms'de 
     yaptigi islemi 0.14 ms'ye indirerek yaklasik 6.3 kat hizlanma saglamistir.
   - Veri buyuklugu ve piksel matrisi genisledikce GPU'nun cok cekirdekli 
     mimarisi lineer olarak olceklenebilir basarim sunmaktadir.

2. Veri Yolu Darbogazi (PCIe Bottleneck):
   - GPU tarafındaki toplam harcanan zamanin yaklasik %90'i saf hesaplamada degil, 
     verinin ana bellekten (RAM) ekran karti bellegine (VRAM) aktarilmasi (Upload) 
     ve sonucun geri indirilmesi (Download) asamalarinda gecmistir.
   - Bu durum GPU programlamada "Memory Transfer Overhead" olarak tanimlanir.

3. Mimari Tercih Sonuclari:
   - Tek adimli ve kucuk boyutlu goruntu islemlerinde veriyi GPU bellegine tasiyip 
     geri almak toplam gecikmeyi artirdigindan (0.58 ms CPU vs 1.93 ms GPU), yerel 
     CPU uctan uca daha verimli calismaktadir.
   - Ancak resim uzerinde ardisik coklu filtreler uygulanacaksa (Resize -> Blur -> 
     Edge Detection vb.) veri surekli VRAM uzerinde kalacagi icin aktarim maliyeti 
     amorti edilir ve CUDA GPU belirgin sekilde onde olacaktir.
================================================================================