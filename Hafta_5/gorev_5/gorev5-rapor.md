# görev 5: 

Bagaj resmi->greyscale dönüşümü-> rgb dönüşümü (jet paleti)


* Griye Dönüştürme (0-255 Matrisi): 3 kanallı renkli resim, her bir pikselin sadece parlaklık/yoğunluk değerini içeren 0 (kapkaranlık) ile 255 (bembeyaz) arasında tek kanallı bir matrise indirgenir.
* Renk Haritalama (Lookup Table - LUT): Kod doğrudan piksellerin parlaklığına bakar ve önceden tanımlanmış sabit bir renk skalasını (JET paleti) uygular:Parlaklığı 
* 200-255 arası (bembeyaz/yoğun) olan pikseller  Kırmızı / Koyu Kırmızı yapılır.
* Parlaklığı 100-190 arası (orta gri) olan pikseller  Yeşil / Sarı yapılır.
* Parlaklığı 0-90 arası (siyah/koyu) olan pikseller  Mavi / Lacivert yapılır.


Ödev kapsamında gri seviyeli tek kanallı X-ray görüntülerinde nesne yoğunluğunu en yüksek kontrastla belirginleştirmek için COLORMAP_JET paletini tercih ettik. JET paleti, düşük parlaklıktaki soğuk bölgeleri maviye, yüksek parlaklık ve X-ışını emilimine sahip yoğun alanları ise sıcak kırmızıya haritalayarak güvenlik operatörlerinin yüksek yoğunluklu nesnelere hızlıca odaklanmasını sağlar. Gerçek çift enerjili (Dual-Energy) malzeme ayrıştırma sensörü verisi yerine elimizde monokrom parlaklık haritası bulunduğu için, yoğunluk gradyanını görselleştirmede akademik ve teknik uygulamalarda en yaygın kullanılan bu palet tercih edilmiştir.

# ürettiğim resim ve orjinal resim arasında renk farklılıkları var. 
- ürettiğim resimde neden kırmızı tonlar ağırlıkta? 
- Ürettiğinim resimde kırmızı tonların ağırlıkta olmasının nedeni, COLORMAP_JET paletinin parlaklık değerlerine göre renkleri haritalamasıyla ilgilidir. JET paleti, düşük parlaklık değerlerini mavi tonlarla, orta parlaklık değerlerini yeşil ve sarı tonlarla, yüksek parlaklık değerlerini ise kırmızı tonlarla temsil eder.
