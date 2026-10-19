Canny kenar bulma algoritmasını sıfırdan yazınız. Aşağıdaki adımların her biri açık açık alt fonksiyonlar şeklinde gösterilmelidir:
Gürültü Bastırma (5x5 Gauss Filtre kullanarak)
Sobel Filtre ile gradyan hesaplama
Non-maximum suppression
Hysteresis Thresholding


çıktı: 
BILGI] Girdi Boyutu: 800x600

[BILGI] Manuel Canny (5x5 Gauss + Sobel + NMS + Hysteresis) suresi: 103.484 ms
[OK] Ara ciktilar ve nihai sonuc /content/ icine kaydedildi.
[OK] Karsilastirma icin OpenCV Canny ciktisi da kaydedildi -> /content/image3_opencv_canny_compare.png

# Canny Kenar Algılama Algoritması
Canny kenar algılama algoritması, görüntü işleme  yaygın olarak kullanılan bir kenar algılama yöntemidir. Bu algoritma, dört ana adımda çalışır: gürültü bastırma, gradyan hesaplama, non-maximum suppression ve hysteresis thresholding. Aşağıda her bir adımın alt fonksiyonlarıyla birlikte açıklaması verilmiştir.

## Not: Bu adımlar daha önce parça parça yazılmıştı
Canny'nin 4 adımının büyük kısmı, bu haftanın (Hafta 4) önceki görevlerinde zaten tek tek yazılmıştı; bu ödevde yapılan şey bu parçaları doğru sırada birleştirmek ve eksik iki adımı (NMS, Hysteresis) eklemekti:
- **Gauss gürültü bastırma**: Görev 1, Görev 3 ve Görev 5'te 3x3 Gauss kernel (`1/16 * [1 2 1; 2 4 2; 1 2 1]`) zaten kullanılmıştı. Bu görevde istenen 5x5 boyutuna genişletildi.
- **Sobel gradyan**: Görev 3'te dikey (Gx) ve yatay (Gy) Sobel kernelleri ayrı ayrı uygulanıp görselleştirilmişti. Bu görevde aynı kernellerle gradyan **genliği** (magnitude) ve **yönü** (angle) hesaplanarak bir sonraki adıma (NMS) girdi olarak hazırlandı.
- **Non-maximum suppression** ve **Hysteresis thresholding**: Önceki haftalarda hiç yazılmamıştı, bu ödevde ilk kez eklendi.

---

## Her Adımın Neden Gerekli Olduğu

### 1. Gürültü Bastırma (5x5 Gauss Filtre)
Canny algoritması, kenarları bulmak için görüntünün **türevini** (Sobel gradyanını) alır. Türev işlemi, doğası gereği yüksek frekanslı değişikliklere aşırı duyarlıdır — ve gürültü (sensör gürültüsü, JPEG sıkıştırma artefaktları vb.) tam olarak yüksek frekanslı, rastgele parlaklık değişimidir. Gauss filtresi uygulanmadan doğrudan Sobel/Laplacian çalıştırılırsa, görüntüdeki her küçük gürültü pikseli de bir "kenar" gibi algılanır ve sonuç tamamen gürültüyle dolar (bu durum Hafta 4 Görev 5 ve Görev 6 raporlarında Laplacian için de aynı şekilde gözlemlenmişti: "Gauss yumuşatması olmadan doğrudan uygulanan Laplacian, görüntüyü tamamen gürültüyle doldurur").
- **Neden 5x5 (3x3 değil)?** Daha büyük bir pencere, daha fazla komşu pikseli ortalamaya dahil ettiği için daha güçlü bir yumuşatma sağlar. 3x3 Gauss hafif gürültüyü bastırırken, 5x5 daha agresif bir ön temizlik yapar; bu da sonraki Sobel adımının daha az sahte kenar üretmesini sağlar. Bedeli: görüntü biraz daha fazla bulanıklaşır, bu yüzden çok ince gerçek kenarlar da hafifçe zayıflayabilir — bu bir **gürültü bastırma / detay koruma dengesidir**.
- Bu adım olmadan Canny, pratikte "gürültü algılayıcı" haline gelirdi, kenar algılayıcı olmaktan çıkardı.

### 2. Sobel Filtre ile Gradyan Hesaplama
Bir pikselin "kenar" olup olmadığını anlamanın matematiksel yolu, o noktadaki parlaklık değişim hızını (gradyanı) ölçmektir. Düz/homojen bir bölgede parlaklık değişmez, gradyan ~0'dır; bir kenarda parlaklık ani değişir, gradyan büyüktür.
- **Gx ve Gy neden ayrı hesaplanır?** Kenarlar her yönde olabilir (yatay, dikey, çapraz). Gx yalnızca yatay yöndeki değişimi, Gy yalnızca dikey yöndeki değişimi yakalar. İkisi birlikte, gradyanın tam bir **vektörünü** verir.
- **Magnitude (`G = sqrt(Gx²+Gy²)`) neden gerekli?** Bu, kenarın "ne kadar güçlü/belirgin" olduğunu, yönden bağımsız tek bir sayıyla ifade eder — bir sonraki adımda (Hysteresis) bu değer eşiklenecektir.
- **Angle (`θ = atan2(Gy,Gx)`) neden gerekli?** Kenarın **yönünü** bilmek şarttır, çünkü Non-Maximum Suppression adımı "kenara dik yöndeki komşularla" karşılaştırma yapar. Yön bilgisi olmadan NMS'nin hangi komşularla karşılaştırma yapacağı belirlenemez.
- Sobel'in Prewitt'e göre tercih edilme sebebi (Görev 3/4'te açıklandığı gibi) merkez satır/sütuna verdiği ek ağırlık (×2) sayesinde gürültüye karşı biraz daha dayanıklı, daha pürüzsüz bir gradyan üretmesidir.

### 3. Non-Maximum Suppression (NMS)
Sobel'den çıkan ham gradyan haritasında, gerçek bir kenarın etrafında genellikle **birkaç piksel kalınlığında** bulanık bir "kenar bandı" oluşur (çünkü gradyan, kenara yakın birden fazla pikselde yüksek değer alır). Bu, pratikte işe yaramaz bir kenar haritasıdır — bir kenarın tam olarak nerede olduğu belirsizdir ve kenarlar gerçekte olduğundan daha kalın görünür (Görev 4/5/6 raporlarında Prewitt/Sobel/LoG çıktılarının "kalın" veya "çift çizgili" olmasının bahsedildiği problem budur).
- NMS'nin amacı: kenar yönüne **dik** yöndeki (gradyan yönündeki) komşu piksellerle karşılaştırma yaparak, yalnızca **lokal maksimum** olan pikseli korumak, diğerlerini sıfırlamak.
- Sonuç olarak kenarlar tam olarak **1 piksel genişliğine** inceltilir. Bu, Canny'nin diğer tüm yöntemlerden (Prewitt, Sobel, LoG, Mexican Hat — Görev 7 karşılaştırma tablosunda görüldüğü gibi) daha "temiz" ve "net" kenar üretmesinin asıl sebebidir.
- Açının 4 ana yöne (0°, 45°, 90°, 135°) yuvarlanması, hangi iki komşu pikselin karşılaştırılacağını basitleştirmek için yapılır; gerçek açı bu 4 yöne en yakın olana atanır.

### 4. Hysteresis Thresholding
NMS'den sonra elde edilen ince kenar haritası hâlâ gürçültü kaynaklı küçük, anlamsız kenar parçacıkları içerebilir; aynı zamanda gerçek bir kenarın bazı noktalarında (örneğin hafif gölgeleme nedeniyle) gradyan değeri geçici olarak düşebilir ve kenar kırılabilir.
- **Tek bir eşik yeterli olmaz**: Eşik çok yüksek seçilirse zayıf ama gerçek kenarlar kaybolur (kenarlar kırık/kopuk çıkar); çok düşük seçilirse gürültü kaynaklı sahte kenarlar da kenar sayılır. Bu klasik "ya hep ya hiç" eşikleme probleminin çözümü **iki eşik** kullanmaktır.
- **Güçlü kenar (`mag ≥ high`)**: Kesinlikle gerçek bir kenardır, doğrudan kabul edilir.
- **Zayıf kenar (`low ≤ mag < high`)**: Tek başına güvenilir değildir, ama eğer bir güçlü kenara **komşuysa** (8-bağlantılı), aslında o güçlü kenarın bir uzantısı/devamı olma ihtimali yüksektir — bu yüzden kabul edilir. Komşu değilse, büyük olasılıkla gürültüdür ve silinir.
- Bu mekanizma, Hafta 2 Görev 5'teki `cv::saturate_cast` ile yapılan basit `[0,255]` eşik/clip mantığının daha gelişmiş bir versiyonu gibi düşünülebilir: burada eşikleme artık tek bir pikselin kendi değerine değil, **komşuluk/bağlantı bilgisine** de bakıyor.
- Sonuç: kopuk kenarlar birleşir, tek başına duran gürültü benekleri elenir — Canny'nin "kesintisiz ve gürültüden arınmış" kenar haritası üretmesinin temel sebebi budur.

---

## Genel Değerlendirme
Dört adım birbirine sıkı sıkıya bağlıdır ve sırası önemlidir: Gauss olmadan Sobel gürültüden boğulur; Sobel'in ürettiği magnitude/angle çifti olmadan NMS çalışamaz; NMS ile incelmemiş bir kenar haritasında Hysteresis'in "komşuluk bağlama" mantığı anlamsız kalır (çünkü kenarlar zaten kalın bloklar halindedir). Bu yüzden Canny, tek bir filtre değil, **birbirini tamamlayan 4 aşamalı bir hat (pipeline)**'tır — ve bu da onu Görev 7'deki karşılaştırmada diğer tek-adımlı filtrelerden (Prewitt, Sobel, LoG, Mexican Hat) daha yavaş ama en doğru/temiz sonucu veren yöntem yapar.

