# Hafta 10 - Görev 3: Blok Benzerliği İçin SAD, SSD, ZNCC Karşılaştırması

## Fonksiyon pointer tasarımı
Hocanın istediği gibi, üç yöntem de **aynı parametre sayısı/türüyle**
(`const cv::Mat&, const cv::Mat&, int xLeft, int xRight, int y`) `inline`
fonksiyon olarak tanımlandı ve tek bir fonksiyon pointer türüyle tutuldu:

```cpp
using ScoreFunc = double(*)(const cv::Mat&, const cv::Mat&, int, int, int);
```

`computeDisparityMap()` fonksiyonu hangi yöntemin kullanılacağını **bilmeden**,
sadece bir `ScoreFunc` parametresi alıp winner-take-all döngüsünü çalıştırıyor;
`main()` içinde bu fonksiyon üç kez, sırayla `computeSAD`, `computeSSD`,
`computeZNCCCost` pointer'larıyla çağrılıyor. Böylece aynı döngü kodu üç
yöntem için de tekrar yazılmıyor.

Üçünün de **"düşük skor = iyi eşleşme"** kuralına uyması için ZNCC (doğası
gereği "yüksek = iyi" olan bir benzerlik ölçüsü) `cost = 1 - ZNCC` olarak
maliyete çevrildi: `ZNCC = +1` (mükemmel eşleşme) → `cost = 0`.

## Pencere boyutu: 3, 7, 13 yarıçapları
Hoca, tek bir pencere boyutu yerine `HALF_WIN` (pencere yarıçapı) = **3, 7
ve 13** için de hesaplama istedi (sırasıyla 7×7, 15×15, 27×27 blok).
Fonksiyon pointer imzası (`ScoreFunc`) sabit kalsın diye `HALF_WIN`
fonksiyonlara parametre olarak eklenmedi; bunun yerine global bir
değişkene çevrildi ve `main()` içinde her yarıçap için değiştirilip AYNI
`computeSAD`/`computeSSD`/`computeZNCCCost` fonksiyonları ve AYNI
`computeDisparityMap()` döngüsü yeniden çalıştırıldı. Sonuçta 3 pencere
boyutu × 3 yöntem = 9 ayrı disparity haritası üretilip tek bir grid'de
(satır = pencere boyutu, sütun = yöntem) karşılaştırıldı.

**Beklenen genel eğilim (pencere büyüdükçe):**
- Hesaplama süresi **kareyle orantılı artar** (27×27 pencere, 7×7'nin
  ~(27/7)² ≈ 15 katı kadar piksel karşılaştırması yapar) — bu artış SAD/SSD
  için de, ZNCC için de geçerlidir, ama ZNCC'nin tabanı daha yüksek
  olduğundan farkı daha da büyütür.
- Küçük pencere (3 → 7×7): gürültüye ve tekrarlayan dokulara karşı daha
  hassastır, disparity haritası daha "noktacıklı" (speckle) ve gürültülü
  çıkar; ama nesne kenarlarını daha **keskin** korur (küçük pencere, komşu
  farklı derinlikteki nesnelerin bilgisini birbirine daha az karıştırır).
- Büyük pencere (13 → 27×27): daha fazla piksel ortalandığı için gürültüye
  karşı daha **kararlı/yumuşak** bir harita verir, dokusuz bölgelerde bile
  daha tutarlı sonuç bulabilir; ama kenarlarda **"foreground fattening"**
  (önde duran nesnenin disparity'sinin arka plana taşması, kenarların
  bulanıklaşması) etkisi görülür, çünkü pencere aynı anda iki farklı
  derinlikteki yüzeyi de kapsayabilir.
- Orta pencere (7 → 15×15) genelde ikisi arasında bir denge noktasıdır:
  7×7'den daha az gürültülü, 27×27'den daha az kenar bulanıklığına sahiptir.

## Yöntemlerin tanımı
sınavda çıkabilir:
| Yöntem | Formül (NxN pencere üzerinde) | Skor yönü |
|---|---|---|
| **SAD** | Σ \|L(i) − R(i)\| | Düşük = iyi |
| **SSD** | Σ (L(i) − R(i))² | Düşük = iyi |
| **ZNCC** | 1 − [Σ(L(i)−L̄)(R(i)−R̄)] / √[Σ(L(i)−L̄)² · Σ(R(i)−R̄)²] | Düşük = iyi (maliyete çevrilmiş) |

## Avantaj / Dezavantaj Tablosu

| Yöntem | Avantajları | Dezavantajları |
|---|---|---|
| **SAD** | - En hızlı (çarpma yok, sadece çıkarma + mutlak değer)<br>- Uygulaması çok basit<br>- Donanımda (FPGA/gömülü sistem) ucuz | - Parlaklık/kontrast farklarına (iki kamera arasındaki pozlama/aydınlatma farkı) duyarlı<br>- Büyük tekil farklara (gürültü, kapanma) SSD kadar olmasa da duyarlı |
| **SSD** | - Küçük farkları SAD'dan daha az, büyük farkları çok daha ağır cezalandırır → net eşleşmelerde SAD'dan biraz daha "keskin" sonuç verebilir | - Kareler alındığı için gürültüye/aykırı değerlere (outlier) SAD'dan daha hassas<br>- Çarpma işlemi içerdiği için SAD'dan biraz daha maliyetli<br>- Parlaklık/kontrast farklarına SAD gibi hâlâ duyarlı |
| **ZNCC** | - Ortalamadan sapmaları normalize ettiği için parlaklık (offset) ve kontrast (gain) farklarına **dayanıklıdır** → iki kameranın pozlaması farklıysa en güvenilir sonucu verir<br>- Skor -1..+1 aralığına normalize olduğu için yorumlanması/eşiklenmesi kolaydır | - En yavaş: her pencere için önce ortalama, sonra normalize çarpım hesaplanır (SAD/SSD'nin ~2 katından fazla işlem)<br>- Düz/dokusuz (homojen) bölgelerde payda sıfıra yaklaşır, sayısal kararsızlık oluşabilir (kodda `denom > 1e-6` kontrolüyle bu durum 0 skoruna sabitlendi) |

## Genel sonuç
- Hız sıralaması (beklenen): **SAD < SSD < ZNCC** (SAD en hızlı, ZNCC en yavaş),
  çünkü ZNCC her pencere için iki geçişli ortalama+normalize hesabı
  gerektirir, SSD ise SAD'a göre bir çarpma işlemi ekler.
- Doğruluk/gürbüzlük açısından, görüntüler arasında parlaklık/kontrast farkı
  varsa **ZNCC** en tutarlı disparity haritasını üretir; iki görüntü zaten
  aynı pozlamada çekilmişse (bu ödevdeki stereo çiftte olduğu gibi) üçü de
  birbirine yakın sonuç verir, çünkü fark yaratan temel etken (parlaklık
  farkı) burada zaten yok.
- Pratikte, gerçek zamanlı/gömülü sistemlerde hız öncelikliyse SAD, görüntüler
  arasında aydınlatma farkı olan senaryolarda doğruluk öncelikliyse ZNCC
  tercih edilir; SSD ikisinin ortasında, aykırı değerlere karşı en hassas
  olduğu için genelde SAD'a tercih edilmez.
