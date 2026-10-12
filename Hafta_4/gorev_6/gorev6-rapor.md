# Görev 6: Mexican Hat (LoG) Filtresi ve Kenar Algılama Operatörlerinin Karşılaştırma Raporu

not: gauss olmadan -> Hafta_4/gorev_6/image3_direct_7x7_laplacian.png

## 1. Mexican Hat (LoG) Filtresi Nedir?

Mexican Hat (Marr-Hildreth) filtresi, **Gauss (Gaussian) yumuşatma** ile **Laplacian ikinci türev** işlemlerini ayrı ayrı yapmak yerine, matematiksel olarak **tek bir matris ($7 \times 7$)** içinde birleştiren izotropik (yön bağımsız) bir kenar algılama filtresidir. 

Matrisin merkezindeki yüksek değerden etrafındaki negatif çukura ve dışarıdaki sıfırlanmaya doğru olan formasyonu bir "Meksika Şapkası" kesitini andırdığı için bu adla anılır.

* **Çalışma Prensibi:** Görüntüdeki yumuşatmayı ve türev almayı aynı anda gerçekleştirir. Çekirdek matrisindeki tüm elemanların toplamı **0**'dır. Bu sayede düz/sabit parlaklıktaki alanlarda tepki vermez ($0$ üretir), yalnızca piksel yoğunluğunun hızla değiştiği kenarlarda ve doku sınırlarında yüksek tepki verir.

---

## 2. Diğer Kenar Filtreleriyle Karşılaştırma

`image3.jpg` (tuğla/duvar dokulu görsel) üzerinde uygulanan Prewitt, Sobel, 3x3 Laplacian ve 7x7 Mexican Hat filtrelerinin davranışsal farkları aşağıda özetlenmiştir:

| Özellik / Kriter | Prewitt ($3 \times 3$) | Sobel ($3 \times 3$) | Laplacian ($3 \times 3$) | Mexican Hat ($7 \times 7$) |
| :--- | :--- | :--- | :--- | :--- |
| **Türev Tipi** | 1. Türev (Birinci Derece) | 1. Türev (Birinci Derece) | 2. Türev (İkinci Derece) | **2. Türev (İkinci Derece)** |
| **Yön Bağımlılığı** | Yönlü ($G_x, G_y$ ayrı) | Yönlü ($G_x, G_y$ ayrı) | Yönsüz (İzotropik) | **Yönsüz (İzotropik)** |
| **İşlem Adımı** | Doğrudan türev | Doğrudan türev | Önceden Gauss şarttır | **Gömülü Gauss + Türev (Tek Adım)** |
| **Gürültü Hassasiyeti** | Yüksek | Orta | Aşırı Yüksek | **Düşük / Kontrollü** |
| **Çıktı Karakteri** | Yönsel tek çizgi | Kalın ve net kenar | İnce / Çift çizgi (Gürültülü) | **İnce, kesintisiz çift çizgi (Temiz)** |

---

## 3. Öne Çıkan Farklar ve Değerlendirme

1. **Yön Bağımsızlığı (İzotropik Yapı):**
   * Prewitt ve Sobel filtreleri dikey ($G_x$) ve yatay ($G_y$) kenarları ayrı ayrı yakalayabilirken, Mexican Hat filtresi **yönden bağımsızdır**. Tuğla derzlerinin dikey ve yatay çizgilerini, köşe birleşimlerini ve çapraz geçişleri tek bir matris geçişinde aynı anda tespit eder.

2. **Gürültü Bastırma Kapasitesi ($7 \times 7$ Etkisi):**
   * $3 \times 3$ boyutundaki standart Laplacian filtresi, önceden bir yumuşatma yapılmazsa yüzeydeki en küçük pürüzü kenar sanarak görüntüyü tamamen gürültüyle doldurur.
   * $7 \times 7$ Mexican Hat filtresi ise geniş etki alanı ($7 \times 7$ penceresi) sayesinde yumuşatma işini kendi bünyesinde yapar. Böylece görüntüdeki parazitleri eler, sadece gerçek nesne sınırlarını öne çıkarır.

3. **Çift Çizgi Etkisi (Double-Edge Effect):**
   * Birinci türev filtreleri (Sobel/Prewitt) kenarları tek bir parlak çizgi olarak gösterirken, ikinci türev tabanlı Mexican Hat filtresi kenar geçişinin başladığı ve bittiği sınırlarda tepki vererek kenarların etrafında çok daha **ince ve çift katmanlı** kontürler oluşturur.

4. **Pratik Uygulama Kolaylığı:**
   * Görüntüyü önce Gauss ile bulanıklaştırıp ardından Laplacian uygulamak yerine (Görev 5), $7 \times 7$ Mexican Hat tek bir konvolüsyon adımıyla aynı sonucu çok daha kararlı şekilde üretir.