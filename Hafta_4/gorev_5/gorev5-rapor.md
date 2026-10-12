# Laplacian Filtresi

# Laplacian of Gaussian  Kernel: 
Laplacian operatörü, görüntü işleme literatüründe ikinci türev (second-order derivative) tabanlı bir kenar algılama filtresidir. Birinci türev alan Prewitt ve Sobel operatörleri kenarların eğimini (gradient genliğini) bulurken, Laplacian türevin sıfırdan geçtiği noktaları (zero-crossing) tespit ederek kenarları yön bağımsız (izotropik) şekilde belirler.1. Matematiksel Model ve Çekirdek Matrisiİkinci türev gürültüye son derece hassas olduğu için uygulama öncesinde görüntünün mutlaka Gauss filtresi ile yumuşatılması gerekir. Bu birleşime LoG (Laplacian of Gaussian) adı verilir.
Standart 4-komşuluk $3 \times 3$ Laplacian çekirdeği:$$L = \begin{bmatrix} 0 & 1 & 0 \\ 1 & -4 & 1 \\ 0 & 1 & 0 \end{bmatrix} \quad \text{veya (8-komşuluk)} \quad \begin{bmatrix} 1 & 1 & 1 \\ 1 & -8 & 1 \\ 1 & 1 & 1 \end{bmatrix}$$

gauss kullanmamızın sebebi; sade laplacian gürültüye çok duyarlı olduğu için, görüntüyü önce Gauss filtresi ile yumuşatıp ardından Laplacian uygulamak daha sağlıklı sonuçlar verir. Bu sayede kenar algılama işlemi sırasında gürültüden kaynaklanan yanlış pozitif kenarlar minimize edilir.

# Prewitt, Sobel ve LoG (Laplacian of Gaussian) Kenar Algılama Operatörlerinin Karşılaştırmalı Analizi

## 1. Teorik Altyapı ve Matematiksel Yaklaşım

Görüntü işlemede kenar algılama, piksellerin gri seviye yoğunluklarındaki ani değişimlerin (yüksek frekanslı bileşenlerin) tespit edilmesine dayanır. Operatörler türev derecelerine göre iki ana gruba ayrılır:

* **Birinci Türev Tabanlı Operatörler (Prewitt & Sobel):** Görüntünün gradyan vektörünü ($\nabla I = [\frac{\partial I}{\partial x}, \frac{\partial I}{\partial y}]^T$) hesaplar. Kenarlar, gradyan genliğinin ($\vert{}\vert{}\nabla I\vert{}\vert{} = \sqrt{G_x^2 + G_y^2}$) lokal maksimum yaptığı noktalardır.
* **İkinci Türev Tabanlı Operatörler (Laplacian / LoG):** Görüntünün yoğunluk değişim hızını ($\nabla^2 I = \frac{\partial^2 I}{\partial x^2} + \frac{\partial^2 I}{\partial y^2}$) ölçer. Kenarlar, ikinci türevin işaret değiştirerek sıfırdan geçtiği **sıfır geçişi (zero-crossing)** noktaları olarak tespit edilir.

---

## 2. Operatör Bazlı Teknik Özellikler

### A. Prewitt Operatörü (Birinci Türev)
* **Çekirdek Yapısı:**
  $$G_x = \begin{bmatrix} -1 & 0 & 1 \\ -1 & 0 & 1 \\ -1 & 0 & 1 \end{bmatrix}, \quad G_y = \begin{bmatrix} -1 & -1 & -1 \\ 0 & 0 & 0 \\ 1 & 1 & 1 \end{bmatrix}$$
* **Karakteristiği:** Komşu piksellere eşit ağırlık verir. Tasarımı basittir ve hesaplama maliyeti düşüktür.
* **Dezavantajı:** Görüntüdeki yüksek frekanslı gürültülere karşı oldukça hassastır. Kenar çizgilerinde kırılmalar ve gürültü kaynaklı sahte kenarlar (spurious edges) oluşturabilir.

### B. Sobel Operatörü (Birinci Türev)
* **Çekirdek Yapısı:**
  $$G_x = \begin{bmatrix} -1 & 0 & 1 \\ -2 & 0 & 2 \\ -1 & 0 & 1 \end{bmatrix}, \quad G_y = \begin{bmatrix} -1 & -2 & -1 \\ 0 & 0 & 0 \\ 1 & 2 & 1 \end{bmatrix}$$
* **Karakteristiği:** Merkez piksel komşuluğuna daha yüksek katsayı ($2$) vererek türev alma işlemi esnasında bünyesinde hafif bir Gauss yumuşatması (smoothing) barındırır.
* **Avantajı:** Prewitt'e kıyasla gürültüyü daha iyi bastırır, daha yüksek kontrastlı ve kesintisiz kenar çizgileri üretir.

### C. LoG - Laplacian of Gaussian (İkinci Türev)
* **Çekirdek Yapısı (8-Komşuluk 3x3 Laplacian):**
  $$L = \begin{bmatrix} 1 & 1 & 1 \\ 1 & -8 & 1 \\ 1 & 1 & 1 \end{bmatrix}$$
* **Karakteristiği:** İkinci türev operatörü gürültüyü aşırı derecede büyüttüğü için, Laplacian öncesinde görüntüye mutlaka $3 \times 3$ Gauss filtresi uygulanır (LoG evrişimi).
* **Avantajı:** **İzotropiktir (yön bağımsızdır).** Prewitt ve Sobel gibi ayrı dikey ($G_x$) ve yatay ($G_y$) hesaplamalar gerektirmeden, tek bir matris geçişiyle tüm yönlerdeki kenarları aynı anda tespit eder.

---

## 3. Deneysel Bulgular ve Karşılaştırma Matrisi

`image3.jpg` (tuğla/duvar dokulu görsel) üzerinde yapılan testlerde elde edilen teknik sonuçlar:

| Parametre / Kriter | Prewitt Operatörü | Sobel Operatörü | LoG (Laplacian of Gaussian) |
| :--- | :--- | :--- | :--- |
| **Türev Derecesi** | 1. Türev (First-order) | 1. Türev (First-order) | 2. Türev (Second-order) |
| **Yön Bağımlılığı** | Yönlü ($G_x, G_y$ ayrı) | Yönlü ($G_x, G_y$ ayrı) | **Yönsüz (İzotropik)** |
| **Yumuşatma Etkisi** | Yok | Var (Merkez ağırlıklı $2$) | Var (Ön Gauss filtresi ile) |
| **Gürültü Duyarlılığı** | Yüksek | Orta | Gauss olmadan çok yüksek / LoG ile kontrollü |
| **Kenar Yapısı** | Kesikli / Parçalı | Kalın ve Belirgin | **İnce, Çift Çizgili (Double-Edge)** |
| **Doku & Detay Algısı** | Orta | Ana hatlar odaklı | **İnce detay ve köşelerde yüksek hassasiyet** |

---

## 4. Sonuç ve Değerlendirme

1. **Yönsel Tespit (Prewitt vs. Sobel):** Birinci türev filtreleri belirli yönlerdeki yapıları izole etmekte başarılıdır. $G_x$ dikey derz çizgilerini yakalarken, $G_y$ yatay katmanları ortaya çıkarır. Sobel, bünyesindeki ağırlıklandırma sayesinde Prewitt'e göre daha pürüzsüz ve gürültüsüz kenar haritası üretir.
2. **Yönsüz Tespit (LoG):** Laplacian operatörü yön bağımsız çalıştığı için görseldeki tuğla köşelerini, çapraz geçişleri ve ince yüzey pürüzlerini tek adımda yakalar.
3. **Çift Çizgi Etkisi (Double-Edge Effect):** İkinci türevin doğası gereği LoG filtresi, parlaklık geçişinin başladığı ve bittiği iki sınırda da tepki vererek kenarların etrafında kapalı, ince ve çift katmanlı çizgiler oluşturur.
4. **Gauss Ön Koşulunun Önemi:** Gauss yumuşatması olmadan doğrudan uygulanan bir Laplacian filtresi, tuğla yüzeyindeki mikroskobik pürüzleri kenar gibi algılayarak görüntüyü tamamen gürültüyle doldurur; ancak Gauss yumuşatması eklendiğinde (LoG) sadece gerçek nesne sınırları net bir şekilde ortaya çıkar.