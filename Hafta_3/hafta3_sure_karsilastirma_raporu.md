# Hafta 3: GPU (Colab) ve Yerel CPU Süre Karşılaştırması

**Kapsam:** gorev_1 (kontrast germe), gorev_2 (histogram eşitleme), gorev_3 (CLAHE)
**Girdi:** `Hafta_3/image2.png`, 1400×933, gri seviye (yaklaşık 1,3 MP)
**GPU:** Google Colab, NVIDIA Tesla T4
**Kodlar:** `Hafta_3/sure_karsilastirma/`
- `cpu_benchmark.cpp`: yerel CPU, tek thread ve çok thread, döngülü ölçüm
- `colab_gpu_benchmark.cpp`: Colab CUDA, döngülü ölçüm

---

## 1. Amaç

Colab'de alınan süreleri, aynı algoritmaların yerelde çok thread'li CPU üzerinde çalıştırılmasıyla elde edilen sürelerle karşılaştırmak. Ayrıca GPU süresinin neden yüksek göründüğünü açıklamak ve GPU'yu adil biçimde (döngülü ölçümle) değerlendirmek için bir yöntem sunmak.

## 2. Mevcut Colab ölçümleri hakkında önemli tespitler

1. **Yalnızca gorev_1 CUDA üzerinde çalışmış.** `gorev_1.cpp` kodu `cv::cuda::GpuMat`, `cv::cuda::minMaxLoc` gibi CUDA fonksiyonlarını kullanıyor. gorev_2 (`equalizeHist` ve manuel eşitleme) ile gorev_3 (`createCLAHE` ve manuel CLAHE) kodlarında hiç CUDA çağrısı yok. Bu kodlar Colab'in **CPU'sunda ve tek thread ile** çalışmış. Dolayısıyla gorev_2 ve gorev_3 için şu an elimizde bir GPU ölçümü yok. Bu iki görevin CUDA karşılıkları (`cv::cuda::equalizeHist`, `cv::cuda::createCLAHE`) `colab_gpu_benchmark.cpp` içine eklendi.
2. **Tüm Colab ölçümleri tek seferlik, "soğuk" çalıştırma.** Program bir kez çalışıp bir kez ölçüm alıyor. GPU'da ilk çağrı; CUDA kernel'lerinin yüklenmesini, sürücüdeki tembel (lazy) başlatmayı ve bellek tahsisini de içerir. gorev_1'de ölçülen **31 308 µs** büyük ölçüde bu tek seferlik maliyetten oluşuyor; asıl hesaplamanın süresi bunun çok küçük bir kısmı.

## 3. Ölçüm ortamı ve yöntemi

| | Yerel CPU | Colab (mevcut sonuçlar) |
|---|---|---|
| Donanım | Apple M4, 10 çekirdek (4P + 6E) | Colab VM CPU ve NVIDIA GPU |
| OpenCV | 5.0.0 (Homebrew) | `/content/opencv_install` (CUDA'lı derleme) |
| Derleyici | Apple clang 16, `-O3` | g++, `-O3` |
| Ölçüm | 1 soğuk çağrı + 10 ısınma + **300 ölçülen iterasyon** | tek çağrı |

- Süreye yalnızca işlemin kendisi dahildir; `imread` ve `imwrite` dahil değildir. Bu, orijinal kodlarla aynıdır.
- Çok thread için `cv::parallel_for_` kullanıldı. Thread sayısı `cv::setNumThreads(1)` ve `cv::setNumThreads(10)` ile ayarlandı. Hazır OpenCV fonksiyonları da aynı thread havuzunu kullanır.
- Manuel kodların paralel sürümleri orijinal algoritmayla piksel düzeyinde aynıdır. Değişen tek şey işin dağıtılmasıdır:
  - **gorev_1:** min/max değerleri satır bloklarında yerel olarak bulunup birleştirilir. Ardından germe işlemi tek geçişte uygulanır.
  - **gorev_2:** her thread kendi yerel histogramını sayar, sonra histogramlar toplanır. LUT uygulama adımı da paralel çalışır.
  - **gorev_3:** 64 bloğun kırpılmış histogram/LUT hesabı ve satır bazlı bilinear enterpolasyon paralel çalışır.
- **Doğrulama:** Paralel germe ve paralel eşitleme çıktıları hazır fonksiyonların çıktılarıyla birebir aynıdır (max |fark| = 0). Manuel CLAHE ile OpenCV CLAHE arasındaki fark max 7 gri seviye, ortalama 0,66'dır. Bu fark, iki implementasyonun kırpma ve enterpolasyon ayrıntılarındaki farklılıktan kaynaklanır ve orijinal tek thread'li kodda da vardır.
- Ölçüm iki kez tekrarlandı. Ortalamalar iki çalıştırma arasında %5'in altında değişti.

## 4. Yerel CPU sonuçları (300 iterasyon, µs)

| İşlem | Thread | İlk çağrı | Ortalama | Medyan | Min | Hızlanma (1→10) |
|---|---:|---:|---:|---:|---:|---:|
| gorev_1 germe (GPU kodundaki adımların CPU karşılığı) | 1 | 1826 | 1520 | 1471 | 1407 | |
| | 10 | 1509 | 1501 | 1487 | 1421 | ×1,0 |
| gorev_1 germe (manuel, `parallel_for_`) | 1 | 857 | 814 | 818 | 755 | |
| | 10 | 449 | **341** | 337 | 263 | **×2,4** |
| gorev_2 `equalizeHist` (hazır) | 1 | 664 | **657** | 655 | 632 | |
| | 10 | 716 | 681 | 678 | 546 | ×1,0 |
| gorev_2 eşitleme (manuel) | 1 | 1157 | 1115 | 1115 | 1069 | |
| | 10 | 860 | 726 | 723 | 606 | ×1,5 |
| gorev_3 CLAHE (hazır) | 1 | 2305 | 2341 | 2352 | 2172 | |
| | 10 | 531 | **500** | 474 | 447 | **×4,7** |
| gorev_3 CLAHE (manuel) | 1 | 3504 | 3430 | 3438 | 3246 | |
| | 10 | 677 | 655 | 643 | 615 | **×5,2** |

**Yorum**
- Bir piksel başına az işlem yapan algoritmalar (germe, eşitleme) bellek bant genişliğiyle sınırlıdır. Bu yüzden 10 thread ancak ×1,5–2,4 hızlanma sağlar. OpenCV'nin `convertTo/subtract/multiply` zinciri ile `equalizeHist` fonksiyonu bu görüntü boyutunda thread sayısından bağımsız kaldı.
- CLAHE hesaplama ağırlıklıdır (blok histogramları ve her piksel için 4 LUT'tan enterpolasyon). Bu nedenle çok thread'den en çok faydalanan işlem CLAHE oldu (×4,7–5,2).

## 5. Colab ölçümleri ile karşılaştırma (µs)

| İşlem | Colab (tek ölçüm) | Colab'de çalıştığı yer | Yerel CPU, 1 thread (ort.) | Yerel CPU, 10 thread (ort.) |
|---|---:|---|---:|---:|
| gorev_1 germe | 31 308 | **GPU (CUDA)** | 814 | 341 |
| gorev_2 `equalizeHist` (hazır) | 6 020 | CPU, 1 thread | 657 | 681 |
| gorev_2 eşitleme (manuel) | 2 085 | CPU, 1 thread | 1 115 | 726 |
| gorev_3 CLAHE (hazır) | 15 855 | CPU, 1 thread | 2 341 | 500 |
| gorev_3 CLAHE (manuel) | 16 875 | CPU, 1 thread | 3 430 | 655 |

Mevcut verilere göre **gorev_1'de GPU, yerel çok thread'li CPU'dan yaklaşık 90 kat yavaş görünüyor.** Bunun nedeni GPU'nun yavaş olması değil, ölçüm yöntemidir:

- 31 ms'lik değer tek bir soğuk çağrıdır. CUDA kernel'lerinin ilk yüklenmesi ve sürücü başlatma maliyeti bu süreye dahildir. Döngülü ölçümde bu maliyet yalnızca ilk iterasyonda ödenir.
- Ara GpuMat tamponları (`d_float`, `d_sub`, `d_scaled`) ilk çağrıda `cudaMalloc` ile tahsis edilir. Sonraki iterasyonlarda aynı tamponlar yeniden kullanılır.

gorev_2 ve gorev_3'teki Colab–yerel farkı ise GPU ile ilgili değildir; iki CPU arasındaki donanım farkını gösterir. Colab VM'nin CPU'su, M4'ün performans çekirdeklerinden belirgin biçimde yavaştır. Hazır fonksiyonlarda ilk çağrıya OpenCV'nin kendi tembel başlatma maliyeti de eklenir.

## 6. GPU'nun döngülü ölçümü (Colab'de çalıştırılacak)

GPU'yu adil ölçmek için `sure_karsilastirma/colab_gpu_benchmark.cpp` hazırlandı. Dosyanın içeriği bir Colab hücresine yapıştırılıp çalıştırılmalıdır (`/content/image2.png` yüklü olmalı). Program şunları yapar:

1. CUDA context'ini ve ilk upload'u ölçümden **önce** başlatır ve bu süreyi ayrıca yazdırır.
2. Her işlem için soğuk ilk çağrıyı, 10 ısınma iterasyonunu ve **300 ölçülen iterasyonu** çalıştırır. Ara tamponlar döngü dışında tanımlıdır.
3. Her iterasyon `Stream::Null().waitForCompletion()` ile senkronize edilir. Böylece ölçülen süre GPU işinin gerçekten bittiği ana kadardır; yalnızca kernel'in başlatılması değil, tamamlanması ölçülür.
4. İki senaryoyu ayrı ayrı ölçer:
   - **sadece kernel:** veri zaten GPU'dadır. Bu, gorev_1'deki orijinal ölçümle aynı kapsamdır.
   - **upload + kernel + download:** CPU'daki görüntüyle başlayıp sonucu CPU'ya geri almayı da içerir. CPU ile birebir karşılaştırmak için gerçekçi olan senaryo budur.
5. gorev_2 ve gorev_3'ün CUDA karşılıklarını (`cv::cuda::equalizeHist`, `cv::cuda::createCLAHE(2.0, 8×8)`) da ölçer.

### 6.1 Colab GPU sonuçları (Tesla T4, 300 iterasyon, µs)

```
[BILGI] Girdi: 1400x933 | GPU: Tesla T4 | iterasyon: 300
[BILGI] CUDA context + ilk upload: 188161.9 us

Islem                                     Ilk(us)    Ort(us)     Medyan        Min   Toplam(us)
gorev_1 germe (sadece kernel)             24039.0     1194.0     1180.8     1139.4     358224.9
gorev_1 germe (upload+kernel+down)         2778.6     2083.3     2045.0     1983.1     625016.0
gorev_2 equalizeHist (sadece kernel)       2809.9       86.1       85.0       83.8      25848.1
gorev_2 equalizeHist (up+k+down)            951.9      942.1      938.9      914.2     282655.6
gorev_3 CLAHE (sadece kernel)              2114.0      208.0      207.4      206.4      62405.9
gorev_3 CLAHE (up+k+down)                  1107.7     1031.9     1025.5     1010.5     309584.6
```

### 6.2 GPU ve CPU karşılaştırması (ortalama, µs)

| İşlem | GPU ilk çağrı | GPU (sadece kernel) | GPU (up+kernel+down) | CPU 1 thread | CPU 10 thread | Kernel'e göre kazanan |
|---|---:|---:|---:|---:|---:|---|
| gorev_1 germe (aynı OpenCV adımları) | 24 039 | 1 194 | 2 083 | 1 520 | 1 501 | **GPU** (×1,26) |
| gorev_1 germe (CPU'da manuel paralel) | | 1 194 | 2 083 | 814 | 341 | **CPU** (×3,5) |
| gorev_2 eşitleme (hazır) | 2 810 | **86** | 942 | 657 | 681 | **GPU** (×7,6) |
| gorev_2 eşitleme (CPU'da manuel) | | 86 | 942 | 1 115 | 726 | **GPU** (×8,4) |
| gorev_3 CLAHE (hazır) | 2 114 | **208** | 1 032 | 2 341 | 500 | **GPU** (×2,4) |
| gorev_3 CLAHE (CPU'da manuel) | | 208 | 1 032 | 3 430 | 655 | **GPU** (×3,1) |

Hızlanma oranları, en iyi CPU süresinin (1 veya 10 thread) GPU kernel süresine bölünmesiyle hesaplandı.

300 iterasyonun toplam süresi:

| İşlem | GPU toplam (sadece kernel) | CPU toplam (10 thread, en iyi) |
|---|---:|---:|
| gorev_2 eşitleme | **25,8 ms** | 197 ms (657 µs × 300) |
| gorev_3 CLAHE | **62,4 ms** | 150 ms (500 µs × 300) |

### 6.3 Yorum

1. **Döngülü ölçüm 31 ms'lik değeri açıklıyor.** gorev_1'in GPU'daki ilk çağrısı 24 039 µs sürdü; Colab'de ölçülen 31 308 µs ile aynı büyüklükte. Isınmadan sonra aynı işlem 1 194 µs'ye iniyor, yani yaklaşık 20 kat kısalıyor. CUDA context'in başlatılması ise ayrıca 188 ms sürüyor. Tek seferlik ölçüm, GPU'nun hesaplama hızını değil bu başlangıç maliyetlerini ölçüyordu.
2. **Veri GPU'dayken eşitleme ve CLAHE'de GPU açıkça daha hızlı.** `equalizeHist`, en iyi CPU sonucundan 7,6 kat; CLAHE, 10 thread'li M4'ten 2,4 kat, tek thread'li CPU'dan 11 kat daha hızlı.
3. **gorev_1 germede GPU, manuel paralel CPU kodunu geçemedi.** GPU, CPU'daki aynı OpenCV adım zincirinden biraz hızlı (1 194 µs'ye karşı 1 501 µs). Manuel paralel CPU kodu ise 341 µs. GPU'daki zincirin yavaş olmasının nedenleri:
   - `cv::cuda::minMaxLoc` her çağrıda sonucu CPU'ya kopyalıyor ve GPU'nun bitirmesini bekliyor.
   - Görüntü 32 bit float'a çevrilip üç ayrı kernel'den geçiyor. Bellek trafiği 8 bitlik işlemin yaklaşık 4 katı.
   - Daha hızlı bir GPU sürümü için iki yol var: `minMax` sonrasında tek bir `convertTo(d_out, CV_8U, scale, -min*scale)` çağrısı kullanmak ya da min/max ve germeyi tek bir özel CUDA kernel'inde birleştirmek. Bu sürüm ölçülmedi.
4. **Tek bir küçük görüntüde aktarım maliyeti baskın.** Upload ve download dahil edildiğinde GPU süresi 940–2 080 µs'ye çıkıyor. Bu senaryoda üç işlemde de 10 thread'li CPU daha hızlı. 1,3 MP'lik görüntünün PCIe üzerinden gidiş-dönüşü (~850 µs) hesaplama süresinden uzun. GPU'nun avantajı, veri GPU'da kaldığında ve birden fazla işlem art arda uygulandığında ortaya çıkıyor.

## 7. Sonuç

- Colab'deki ilk GPU ölçümü (31 ms), tek seferlik CUDA başlatma maliyeti yüzünden yanıltıcıydı. Döngülü ölçümde bu maliyet ilk çağrıya kalıyor; sonraki çağrılar 20 kat daha hızlı.
- gorev_2 ve gorev_3'ün orijinal Colab ölçümleri GPU değil, tek thread'li CPU ölçümleriydi. Döngülü ölçümde bu iki görevin CUDA sürümleri ayrıca ölçüldü.
- **Veri GPU belleğindeyken (sadece kernel süresi):** histogram eşitlemede GPU 7,6 kat, CLAHE'de 2,4 kat daha hızlı (10 thread'li Apple M4'e göre). 300 iterasyonda toplam süre eşitlemede 25,8 ms'ye karşı 197 ms, CLAHE'de 62,4 ms'ye karşı 150 ms.
- **İstisnalar:** gorev_1'deki germe zinciri GPU'da float dönüşümleri ve senkronizasyon yüzünden manuel paralel CPU kodundan yavaş kaldı (1 194 µs'ye karşı 341 µs). Görüntünün CPU'dan GPU'ya gidip gelmesi dahil edildiğinde ise, bu boyuttaki tek bir görüntü için üç işlemde de çok thread'li CPU daha hızlı.

## Ek: Yeniden çalıştırma

```bash
# Yerel CPU (macOS, Homebrew OpenCV 5)
cd Hafta_3/sure_karsilastirma
export PKG_CONFIG_PATH=/opt/homebrew/opt/opencv/lib/pkgconfig
clang++ -std=c++17 -O3 cpu_benchmark.cpp -o cpu_benchmark $(pkg-config --cflags --libs opencv5)
./cpu_benchmark ../image2.png 300
```
