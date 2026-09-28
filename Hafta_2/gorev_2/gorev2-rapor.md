--------------------------------------------------------------------------------
GÖREV 2: TEK ÇEKİRDEK (SEQUENTIAL) KUANTALAMA & BİT DERİNLİĞİ AZALTMA
--------------------------------------------------------------------------------
1. Konusu ve Amacı:
   8-bit (256 gri seviye) derinliğindeki bir portre görüntüsünün piksel başına 
   düşen seviye sayısını 2-bit'e (4 ton) düşürerek kuantalama yöntemlerinin 
   görsel etkisini incelemek.

2. Uygulanan Yöntemler:
   - Basamak Tabanlı (Klasik): (val / 64) * 64 formülüyle pikseller doğrudan 
     {0, 64, 128, 192} değerlerine indirgendi.
   - Dinamik Aralığa Yayılmış: (val / 64) * 85 formülüyle tüm [0, 255] aralığı 
     eşit paylaşılarak {0, 85, 170, 255} kontrast skalası korundu.

3. Sonuçlar:
   - Girdi            : 512 x 512 (human_face.jpg)
   - Üretilenler      : woman_4tones_64.png, woman_4tones_85.png
   - Gözlem           : 64 katsayılı çıktı koyu bir tonda kalırken, 85 katsayılı 
                        çıktı dinamik kontrastı daha iyi korudu.