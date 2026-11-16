=====================================================================
      HAFTA 9 - GÖREV 2: BMP ÇÖZÜNÜRLÜK VE KANAL BOYUT ANALİZİ       
=====================================================================
1. RENKLİ BMP (color_1000x1000.bmp - 3 Kanal, OpenCV ile yeniden kaydedildi):
   - Çözünürlük         : 1000 x 1000 piksel
   - Kanal Sayısı       : 3 (24-bit: B, G, R)
   - Teorik Veri Boyutu : 3000000 Byte (2.86 MB)
   - Diskteki Boyut     : 3000054 Byte (2.86 MB)
   - BMP Başlık (Header): 54 Byte
---------------------------------------------------------------------
2. GRİ TONLU BMP (gray_1000x1000.bmp - 1 Kanal):
   - Çözünürlük         : 1000 x 1000 piksel
   - Kanal Sayısı       : 1 (8-bit: Tek Kanal)
   - Teorik Veri Boyutu : 1000000 Byte (0.95 MB)
   - Diskteki Boyut     : 1001078 Byte (0.95 MB)
   - BMP Başlık+Palet   : 1078 Byte (1024+54-> 1024-byte palet + 54-byte header)
---------------------------------------------------------------------
3. JPEG KARŞILAŞTIRMASI (sıkıştırmalı format):
   - Renkli JPEG Boyutu : 277386 Byte (0.26 MB)
   - Gri JPEG Boyutu    : 156594 Byte (0.15 MB)
   - Renkli: BMP/JPEG Oranı : 10.82 kat (BMP kaç kat büyük)
   - Gri   : BMP/JPEG Oranı : 6.39 kat (BMP kaç kat büyük)
---------------------------------------------------------------------
4. KIYASLAMA ÖZETİ:
   - Boyut Oranı (Renkli BMP / Gri BMP): 3.00 kat
=====================================================================

Diskteki boyut, kanal sayısına göre nasıl değişiyor?
Diskteki boyut, kanal sayısına göre doğrudan değişmektedir. Renkli BMP dosyası 3 kanala sahipken (B, G, R), gri tonlu BMP dosyası sadece 1 kanala sahiptir. Bu nedenle, renkli BMP dosyasının teorik veri boyutu ve disk boyutu, gri tonlu BMP dosyasının yaklaşık üç katıdır.