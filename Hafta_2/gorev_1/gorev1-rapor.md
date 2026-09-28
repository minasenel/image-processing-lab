--------------------------------------------------------------------------------
GÖREV 1: CUDA İLE DONANIM HIZLANDIRMALI RENK DÖNÜŞÜMÜ VE BOYUTLANDIRMA (RESIZE)
--------------------------------------------------------------------------------
1. Konusu ve Amacı:
   Görüntünün ana bellekten (Host/RAM) grafik işlemci belleğine (Device/VRAM) 
   aktarılması, CUDA çekirdekleri üzerinde RGB->Gri dönüşümü yapılması ve iki 
   farklı ölçekte (0.5x ve 2.0x) asenkron boyutlandırılması.

2. CUDA Kodunda Yapılan İşlemler:
   - Bellek Transferi (Upload): cv::cuda::GpuMat nesnesi kullanılarak CPU'daki 
     Mat verisi PCIe veri yolu üzerinden GPU VRAM'ine taşındı (d_bgr.upload).
   - GPU Tabanlı Renk Uzayı Dönüşümü: cv::cuda::cvtColor fonksiyonu çağrılarak 
     her bir pikselin BGR'dan Grayscale'e dönüşümü binlerce CUDA iş parçacığı 
     (thread) üzerinde paralel olarak koşturuldu.
   - GPU Boyutlandırma (Warping): cv::cuda::resize ile enterpolasyon işlemleri 
     GPU çekirdeklerinde hesaplanarak donanım seviyesinde hızlandırıldı.
   - Bellek Transferi (Download): İşlenen matrisler GPU'dan CPU ana belleğine 
     geri çekildi (d_half.download, d_double.download).

3. Sonuçlar:
   - Taban Çözünürlük : 1512 x 982
   - 0.5x Çıktı       : 756 x 491 (gray_half.jpg)
   - 2.0x Çıktı       : 3024 x 1964 (gray_double.jpg)
   - Arşiv Çıktısı    : processed_images.zip