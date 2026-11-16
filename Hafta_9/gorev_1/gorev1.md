# Farklı Renk Uzayları
Farklı renk uzaylarının (RGB, HSV/HSB, YCbCr, Lab, CMYK vb.) kullanım amacına göre avantajları şöyle özetlenebilir:

## RGB : RGB (Red-Green-Blue)

- Donanımla (kameralar, ekranlar) doğrudan uyumlu — yakalama ve görüntülemede ek dönüşüm gerektirmez.
- Basit, anlaşılır; görüntü işleme kütüphanelerinde varsayılan format.
- Dezavantajı: renk ve parlaklık bilgisi birbirine karışık (her kanal hem renk hem aydınlık bilgisi taşır), bu yüzden aydınlatma değişimlerine karşı hassastır.

## HSV / HSB (Hue-Saturation-Value)

- Rengi (Hue), doygunluğu (Saturation) ve parlaklığı (Value) ayırır.

- Renk tabanlı segmentasyon (örn. belirli bir rengi tespit etme) için çok daha kolaydır — çünkü Hue kanalı aydınlatma değişimlerinden RGB'ye göre daha az etkilenir.

- İnsan algısına RGB'den daha yakın bir mantıkla çalışır (biz de rengi "ne renk, ne kadar canlı, ne kadar parlak" diye düşünürüz).

## YCbCr / YUV

- Parlaklık (Luma, Y) ile renk bilgisini (Chroma, Cb/Cr) tamamen ayırır.

- Sıkıştırma (JPEG, video codec'leri) için idealdir çünkü insan gözü parlaklık değişimine renk değişiminden daha duyarlıdır — bu sayede chroma kanalları alt örneklenerek (subsampling) veri boyutu düşürülebilir.

- Aydınlatma/gölge etkilerinden bağımsız işlem yapmak isteyen görüntü işleme algoritmalarında (örn. cilt tonu tespiti) avantajlıdır.

## Lab (CIELAB)

- Algısal olarak düzgün (perceptually uniform) — iki renk arasındaki sayısal fark, insan gözünün algıladığı farkla orantılıdır.

- Renk farkı ölçümü, renk düzeltme ve cihazdan bağımsız (device-independent) işler için en uygun uzaydır.

- Parlaklık (L) kanalı ayrı olduğundan aydınlatmadan bağımsız analiz kolaylaşır.

## CMYK (Cyan-Magenta-Yellow-Key/Black)

- Baskı/matbaa işlerinde doğrudan mürekkep miktarlarıyla eşleştiği için baskı çıktısı tahmini ve renk ayrımı için pratiktir.



------------------------------------------------------------

    Hangi Renk Uzayını Kullanmalı?
Uygulamaya göre değişir. 
- Segmentasyon / renk tespiti-> HSV
- Sıkıştırma/video işleme → YCbCr/YUV
- Algısal renk farkı/kalibrasyon → Lab
- Baskı → CMYK
- Genel görüntüleme/yakalama → RGB