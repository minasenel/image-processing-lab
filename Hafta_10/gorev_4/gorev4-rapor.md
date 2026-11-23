# Hafta 10 - Görev 4: Kendi SGM (Semi-Global Matching) Gerçeklememiz

## Ne yaptık?
Görev 2/3'teki lokal blok eşleştirmeyi (her piksel bağımsız, sadece kendi
7×7 SAD maliyetine bakarak winner-take-all seçim) **SGM mantığına** göre
genişlettik. `cv::StereoSGBM` kullanılmadı; algoritma sıfırdan 3 adımda
yazıldı:

1. **Maliyet (cost) hacmi** — her piksel ve her `d = 0..15` için 7×7 blok
   SAD skoru `cost[d](y,x)` olarak saklandı (Görev 2/3 ile aynı).
2. **Agregasyon (semi-global düzeltme)** — bu maliyet, görüntü üzerinde
   4 ayrı 1 boyutlu yol boyunca (sol→sağ, sağ→sol, yukarı→aşağı,
   aşağı→yukarı) zincirleme olarak düzeltildi:
   ```
   L_r(p,d) = C(p,d) + min( L_r(p-r,d),
                             L_r(p-r,d-1) + P1,
                             L_r(p-r,d+1) + P1,
                             min_k L_r(p-r,k) + P2 )
                - min_k L_r(p-r,k)
   ```
3. **Toplama + karar** — 4 yoldan gelen `L_r` toplanıp `S(p,d)` elde edildi,
   her piksel için `S(p,d)`'yi minimize eden `d` seçildi.

Not: Gerçek `cv::StereoSGBM` genelde 8 (bazen 16) yön kullanır; biz ödev
kapsamında öğretici/basitleştirilmiş bir versiyon olarak **4 temel yön**
(dikey + yatay eksenler) kullandık. Algoritmanın özü — maliyet agregasyonu
+ P1/P2 smoothness terimi — birebir aynı.

## Smoothness (P1 / P2) parametresi ne işe yarar?
Lokal blok eşleştirmenin (Görev 2/3) en büyük sorunu, her pikselin
**tamamen bağımsız** karar vermesidir: komşu piksellerin birbirinden hiç
haberi olmaz. Bu, özellikle dokusuz/düz yüzeylerde ve gürültülü bölgelerde
**"speckle" (noktacık) gürültüsü** olarak görünen, komşusundan anlamsızca
farklı tekil disparity değerlerine yol açar.

SGM bunu, komşu piksellerin disparity'lerinin birbirine **yakın olmasını
ödüllendiren** (aksi halde ceza uygulayan) iki parametre ile çözer:

- **P1** — yol üzerindeki komşu piksel ile disparity **sadece 1 birim**
  değişiyorsa uygulanan **küçük** ceza. Eğimli/yavaş değişen yüzeyleri
  (örn. bir rampa, açılı bir duvar) gerçek disparity gradyanını bozmadan
  hafifçe pürüzsüzleştirir.
- **P2** — komşu piksel ile disparity **1'den fazla** değişiyorsa uygulanan,
  **farkın büyüklüğüne bağlı olmayan sabit** bir **büyük** ceza. Sabit
  olması kritik: gerçek bir derinlik sıçraması (bir nesnenin kenarı, ön
  plan/arka plan geçişi) hâlâ mümkündür, sadece "pahalı" hale getirilir —
  yani SGM, **gürültüyü** (anlamsız, tek piksellik sıçramalar) bastırırken
  **gerçek kenarları** aşırı bulanıklaştırmaz. Eğer ceza, farkın
  büyüklüğüyle orantılı olsaydı (örn. kare farkı gibi), büyük ve gerçek
  derinlik sıçramaları da aşırı cezalandırılır, kenarlar aşırı
  yumuşatılırdı.

### Pratik etki
- **P1, P2 = 0** olsaydı: formüldeki smoothness terimi devre dışı kalır,
  her piksel yalnızca kendi `C(p,d)` değerine göre karar verir — bu da
  tam olarak Görev 2/3'teki **lokal** (gürültülü, noktacıklı) sonuca geri
  döner.
- **P1, P2 çok büyükse**: komşuluk baskısı kendi maliyetinden çok daha
  ağır basar, disparity haritası aşırı pürüzsüzleşir; gerçek nesne
  kenarları bile komşu bölgeyle aynı disparity'ye "yapışabilir" (kenar
  bulanıklaşması / "foreground fattening").
- **Dengeli P1/P2** (bu ödevde `P1 = 8·N², P2 = 32·N²`, `N` = pencere
  kenarı, `cv::StereoSGBM`'in önerdiği tipik oranla aynı): düz/dokusuz
  bölgelerde gürültüyü bastırırken, gerçek kenarlarda hâlâ keskin bir
  geçişe izin verir.

## Beklenen gözlem
Görev 2/3'teki (tamamen lokal) disparity haritasıyla bu SGM çıktısı
karşılaştırıldığında, SGM haritasının özellikle düz/dokusuz yüzeylerde
**daha az noktacıklı gürültü** içermesi, buna karşın nesne kenarlarının
hâlâ (P2 sabit olduğu için) makul ölçüde korunmuş olması beklenir.
