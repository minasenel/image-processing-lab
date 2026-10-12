# Sobel Filtresi 
Sobel filtresi: Merkeze yakın piksellerin ağüırlığını arttırır. Bu sayede kenarlar daha belirgin olur.Prewwit te sadece çıkartma-toplama varken, Sobel filtresinde fazladan 2 ile çarpma -> Bit shifting vardır. Prewwit biraz daha hızlıdır ama getirdiği avantajlardan ötürü endüstride Sobel tercih edilir.  
# Dikey sobel kernel :
-1 0 1
-2 0 2
-1 0 1
# yatay sobel kernel: 
-1 -2 -1 
 0  0  0
 1  2  1

Bit shifting den dolayı orta satır ve sütunlar 2 ile çarpılıyor. 
