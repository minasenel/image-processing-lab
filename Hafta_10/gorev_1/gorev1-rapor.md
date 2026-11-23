# Hafta 10 - Görev 1: Stereo Eşleştirme (Disparity Map)

## Yöntem
`scene1.row3.col1.ppm` (sol) ve `scene1.row3.col3.ppm` (sağ) görüntüleri `cv::StereoBM`
(**lokal** blok eşleştirme) ile eşleştirilerek piksel bazlı fark (disparity) haritası
üretildi. StereoBM, her piksel için yalnızca kendi `blockSize x blockSize` penceresindeki
yoğunluk deseniyle sağ görüntüdeki aynı pencereyi karşılaştırır (en düşük SAD farkını
veren kaymayı seçer); komşu piksellerin kararları birbirini etkilemez, yani global/yarı
global bir yumuşatma veya enerji minimizasyonu yoktur (StereoSGBM'den farkı budur).
Harita `COLORMAP_JET` ile renklendirildi ve sol/sağ görüntülerle yan yana
`gorev1_disparity.png` olarak kaydedildi.

## Renklerin anlamı
Disparity haritasındaki her piksel değeri, sol görüntüdeki bir noktanın sağ görüntüde
aynı satırda **kaç piksel sola kaymış** olarak bulunduğunu gösterir:

- **Sıcak renkler (kırmızı/sarı)** → büyük disparity → nesne kameralara **yakın**
- **Soğuk renkler (mavi)** → küçük disparity → nesne kameralara **uzak**
- **Siyah/boşluk bölgeler** → eşleştirme başarısız olmuş (dokusuz yüzey, kapanma vb.)

Bu ilişki `disparity = (f * baseline) / derinlik` formülüyle açıklanır: odak uzaklığı (f)
ve iki kamera arası mesafe (baseline) sabitken, derinlik arttıkça disparity küçülür.

## Gerçek bir derinlik ölçümü sağlıyor mu?
**Hayır, doğrudan değil.** Harita sadece **bağıl (göreceli)** bir yakın/uzak sıralaması
verir, metrik (cm/m) bir derinlik değeri vermez:

- Piksel disparity'sini gerçek derinliğe çevirmek için kameraların **odak uzaklığı** ve
  **baseline** (iki kamera arası fiziksel mesafe) kalibrasyon yoluyla bilinmelidir; bu
  bilgi olmadan sonuç sadece "hangi nokta hangisinden daha yakın" bilgisidir.
- Dokusuz/düz yüzeylerde (örn. düz bir duvar), kapanma (occlusion) bölgelerinde ve
  parlak/yansıtıcı alanlarda blok eşleştirme başarısız olur, harita gürültülü veya
  geçersiz (siyah) çıkar.
- Bu nedenle stereo eşleştirme, bir LiDAR veya ToF sensörü gibi doğrudan fiziksel
  derinlik ölçmez; iki 2D görüntü arasındaki **görsel benzerlikten** derinliği
  **tahmin eder**, dolayısıyla kalibre edilmeden mutlak/metrik bir ölçüm sayılamaz.
