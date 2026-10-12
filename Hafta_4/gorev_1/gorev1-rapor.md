Görev 1 - Prewitt Kenar Algılama Filtreleme
Prewwit filtresi nasıl çalışıyor?
Prewitt filtresi, görüntü işleme ve bilgisayarla görme alanında kullanılan bir kenar algılama yöntemidir. Bu filtre, özellikle görüntülerdeki kenarları ve geçişleri belirlemek için tasarlanmıştır. Prewitt filtresi, bir görüntünün yatay ve dikey yönlerdeki gradyanlarını hesaplayarak kenarları tespit eder.

# dikey prewitt filtresi
Dikey Prewitt filtresi, bir görüntüdeki dikey kenarları tespit etmek için kullanılan bir kenar algılama yöntemidir. Bu filtre, görüntünün yatay ve dikey yönlerdeki gradyanlarını hesaplayarak kenarları belirler. Dikey Prewitt filtresi, özellikle dikey geçişleri ve kenarları vurgulamak için tasarlanmıştır.

Kernel Martisi:
-1 0 1
-1 0 1 
-1 0 1 

Bu matrise göre, görüntüdeki her pikselin etrafındaki 3x3 komşuluk pikselleri ile çarpılarak yeni bir değer elde edilir. Bu işlem, görüntüdeki dikey kenarları vurgular ve geçişleri belirler.

# Gaussian Filtresi ile Prewitt Filtresinin farkı:

kernel matrisi:

$$G = \frac{1}{16} \begin{bmatrix} 1 & 2 & 1 \\ 2 & 4 & 2 \\ 1 & 2 & 1 \end{bmatrix} = \begin{bmatrix} 0.0625 & 0.125 & 0.0625 \\ 0.125 & 0.25 & 0.125 \\ 0.0625 & 0.125 & 0.0625 \end{bmatrix}$$

Gaussian yumuşatma işlemi yaparken prewitt filtresi, kenarları belirlemek için gradyan hesaplaması yapar. Gaussian filtresi, görüntüyü bulanıklaştırarak gürültüyü azaltırken, Prewitt filtresi kenarları vurgular ve geçişleri belirler. Bu nedenle, Prewitt filtresi genellikle kenar algılama ve şekil tanıma gibi uygulamalarda kullanılırken, Gaussian filtresi daha çok gürültü azaltma ve görüntü iyileştirme amacıyla kullanılır.


