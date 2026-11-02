# Zero Mean İşlemi ve Piksel Yakınlık Metrikleri Raporu

## Deney Sonuçları

| # | Metrik | Değer | Not |
|---|--------|------:|-----|
| 1 | Temiz Resim Ortalama Piksel Değeri | 194.2363 | |
| 2 | Bright Resim Ortalama Piksel Değeri | 214.9072 | |
| 3 | Zero Mean Sonrası Resim 1 Ortalaması | 0.0000 | (~0) |
| 4 | Zero Mean Sonrası Resim 2 Ortalaması | 0.0000 | (~0) |

| Metrik | Değer | Açıklama |
|--------|------:|----------|
| **ZNCC** (Zero Mean Cross Correlation) | 0.9851 | 1.0 = Birebir Aynı Doku |
| **MSE** (Mean Squared Error) | 970.3311 | Parlaklıktan dolayı kötü çıktı |
| **PSNR** (Peak Signal-to-Noise Ratio) | 18.2616 dB | |
| **MAE** (Mean Absolute Error) | 20.6709 | |

---

## 1. Zero Mean İşlemi Nedir ve Ne İşe Yarar?

### Matematiksel Tanım

Bir görüntünün tüm piksel değerlerinin ortalaması (μ) hesaplanır ve her bir piksel değerinden bu ortalama çıkarılır:

```
I_zero_mean(r, c) = I(r, c) - μ
```

### Ne Yapar?

Görüntünün ortalama parlaklık seviyesini (DC bileşenini) sıfıra çeker. Elde edilen yeni matrisin ortalama değeri tam olarak 0 (veya sayısal duyarlılığa bağlı olarak ~0.0000) olur.

### Temel Amacı ve Avantajı

İki farklı resim karşılaştırılırken ortam ışığı, parıltı veya global parlaklık farklılıklarını (*illumination shift*) elimine eder. Örneğin, bir resim tamamen aydınlık bir ortama çekilip tüm piksellerine sabit bir parlaklık (+20 / +50) eklense dahi, Zero Mean işlemi uygulandığında iki resim de aynı sıfır merkezli doku matrisine dönüşür.

---

## 2. Zero Mean Bileşenli ZNCC Metriği (Zero Mean Normalized Cross Correlation)

**ZNCC formülü:**

```
            Σᵢ [ (Aᵢ - μ_A) * (Bᵢ - μ_B) ]
ZNCC = ──────────────────────────────────────────
        √[ Σᵢ (Aᵢ - μ_A)² * Σᵢ (Bᵢ - μ_B)² ]
```

- ZNCC, Zero Mean mantığını doğrudan formülün içinde barındırır.
- `[-1.0, +1.0]` aralığında değer üretir (+1.0 tam yapısal eşleşmedir).
- Global parlaklık ve doğrusal kontrast değişimlerinden **etkilenmez**.
- Deney çıktısında görüldüğü üzere: Temiz ve parlaklığı değiştirilmiş resimler arasında `MSE = 970.33` (kötü) ve `PSNR = 18.26 dB` çıkmasına rağmen, `ZNCC = 0.9851` çıkarak dokunun **%98.5** oranında aynı korunduğunu doğru bir şekilde kanıtlamıştır.

---

## 3. Piksel Yakınlığını Ölçen Alternatif Metrikler

### 3.1 SSIM (Structural Similarity Index – Yapısal Benzerlik İndeksi)

- **Formül:** `SSIM(x,y) = [l(x,y)]^α * [c(x,y)]^β * [s(x,y)]^γ`
- **Çalışma Mantığı:** İnsan Görsel Algısını (HVS) modelleyerek Parlaklık (l), Kontrast (c) ve Yapı (s) bileşenlerini bağımsız olarak değerlendirir.
- **Aralık:** `[0.0, 1.0]` (1.0 = Birebir kusursuz yapısal eşleşme)
- **Avantajı:** İnsan gözünün "kaliteli resim" algısıyla **en uyumlu** metriktir. Kenar, doku ve bulanıklık bozulmalarını en hassas tespit eden metriktir.
- **Dezavantajı:** Kayan pencere (*sliding window*) kullandığı için işlem yükü yüksektir.

### 3.2 MSE (Mean Squared Error – Ortalama Kare Hata)

- **Formül:** `MSE = (1/N) * Σᵢ [ (Aᵢ - Bᵢ)² ]`
- **Çalışma Mantığı:** Pikseller arası farkların karelerinin ortalamasını alır.
- **Avantajı:** Çok hızlı hesaplanır, türevlenebilir olduğu için yapay zeka ve optimizasyon algoritmalarında kayıp fonksiyonu (*loss function*) olarak idealdir.
- **Dezavantajı:** Işık değişimine ve 1 piksel kaymaya (*shift*) aşırı hassastır. Görsel olarak iyi görünen ama aydınlatması farklı resmi "kötü" ilan eder.

### 3.3 PSNR (Peak Signal-to-Noise Ratio – Tepe Sinyal Gürültü Oranı)

- **Formül:** `PSNR = 10 * log10( 255² / MSE )` *(Birim: dB)*
- **Çalışma Mantığı:** Maksimum piksel gücünün gürültü hatasına oranını desibel ölçer.
- **Avantajı:** Sıkıştırma (JPEG vb.) ve filtreleme analizlerinde akademik standarttır.
- **Dezavantajı:** MSE tabanlı olduğu için insan gözünün algısal kalitesini tam yansıtmaz.

### 3.4 MAE / SAD (Mean Absolute Error / Sum of Absolute Differences)

- **Formül:** `MAE = (1/N) * Σᵢ [ |Aᵢ - Bᵢ| ]`
- **Çalışma Mantığı:** Mutlak piksel farklarının ortalamasıdır.
- **Avantajı:** Farkların karesi alınmadığı için Salt & Pepper gürültüsü gibi ekstrem aykırı değerlerden (*outliers*) MSE kadar aşırı etkilenmez.
- **Dezavantajı:** Sıfır noktasında türevi yoktur.

### 3.5 NCC (Normalized Cross Correlation – Normalize Çapraz Korelasyon)

- **Çalışma Mantığı:** Ortalamayı (*zero-mean*) çıkarmadan piksel vektörlerinin skaler çarpımının normlarına oranıdır.
- **Avantajı:** Ölçek değişimlerine dirençlidir.
- **Dezavantajı:** Sıfır-ortalama yapılmadığı için parlaklık eklemelerinden (*offset*) etkilenir.

---

## 4. Özet Metrik Karşılaştırma ve Seçim Rehberi

| Metrik | Işık/Parlaklık Bağımsızlığı | İnsan Algı Uyumu | İşlem Hızı | İdeal Kullanım |
|--------|:---:|:---:|:---:|---|
| **ZNCC** | Mükemmel (Zero-Mean) | Orta | Yavaş | Şablon Eşleme (Template) |
| **SSIM** | Yüksek | Mükemmel | Orta | Algısal Kalite Değerlendirme |
| **MSE** | Zayıf | Zayıf | Çok Hızlı | Optimizasyon / Model Eğitimi |
| **PSNR** | Zayıf | Orta | Hızlı | Sıkıştırma / Kodek Testi |
| **MAE/SAD** | Zayıf | Zayıf-Orta | Çok Hızlı | Hareket Tahmini (Motion Estimation) |
