# Projenin Amacı
Proje motion detection ile hareket eden nesnelerin tespitine odaklanır. Kullanıcıya bir arayüz sunar.
<<<<<<< HEAD
1. Videodan nesne respiti yapılır ( YOLOv9, YOLOv10, YOLO11)
2. Aynı nesne kareler boyunca izlenir
3. Bu bounding boxlar aracılığı ile hareketlilik analiz edilir. Bunun için ayrı bir algoritma yazmak gerekebilir.
4. Algılanan hareket olayları gerçek zamanlı olarak takip edilir.
5. İstatistikler ve sonuçlar dışa aktarılır.

#Temel Özellikler
1. Ana Pencere İskeleti
2. Video Seçici
3. Model Export / Import
4. Model Seçme Paneli
5. Nesne Seçme Paneli: Hangi sınıfların izleneceği belirlenir (checkbox).
6. Hareket Analizi Ayarları: Min/max hız eşiği gibi parametreler.
7. Process: Parametreleri onaylayıp pipeline ı tetikler.
8. Başlat/Durdur: İşlem kontrolü, duraklatma/devam.
9. Real-Time Ön İzleme
10. Sonuç İstatistiklerini Kaydetme: ortalama hız, toplam hareket süresi, FPS gibi metrikleri CSV veya JSON olarak yazar.
11. Sonuç Çıktılarını Kaydetme : Nesne ID, zaman damgası, pozisyon, hız bilgileri JSON veya CSV olarak dışa aktarılır.

Projeme yukarıdaki temel bileşenleri sırası ile eklemeye başlayacağım.

Program Sürümleri:
1. QT 6.9.1 
2. QT Creator 17.0
 
=======
1- Videodan nesne respiti yapılır ( YOLOv9, YOLOv10, YOLO11)
2- Aynı nesne kareler boyunca izlenir
3- Bu bounding boxlar aracılığı ile hareketlilik analiz edilir. Bunun için ayrı bir algoritma yazmak gerekebilir.
4- Algılanan hareket olayları gerçek zamanlı olarak takip edilir.
5- İstatistikler ve sonuçlar dışa aktarılır.

#Temel Özellikler
1-Ana Pencere İskeleti
2-Video Seçici
3-Model Export / Import
4-Model Seçme Paneli
5-Nesne Seçme Paneli: Hangi sınıfların izleneceği belirlenir (checkbox).
6-Hareket Analizi Ayarları: Min/max hız eşiği gibi parametreler.
7-Process: Parametreleri onaylayıp pipeline ı tetikler.
8-Başlat/Durdur: İşlem kontrolü, duraklatma/devam.
9-Real-Time Ön İzleme
10-Sonuç İstatistiklerini Kaydetme: ortalama hız, toplam hareket süresi, FPS gibi metrikleri CSV veya JSON olarak yazar.
11-Sonuç Çıktılarını Kaydetme : Nesne ID, zaman damgası, pozisyon, hız bilgileri JSON veya CSV olarak dışa aktarılır.

Projeme yukarıdaki temel bileşenleri sırası ile eklemeye başlayacağım.
-QT 6.9.1 
-QT Creator 17.0

>>>>>>> 482ce8d (Terminal Dosya yükleme)
# Geliştirme Yol Haritası
1. Ana pencere, sinyal/slot omurgası
2. Video Seçici widget ve OpenCV entegrasyonu
3. Model Export/Import, doğrulama
4. Model ve Nesne Seçme panelleri
5. Tracker entegrasyonu
6. MotionAnalyzer modülü, hız eşiği testleri
7. RT Ön İzleme optimizasyonu : real-time görüntü akışını ekranda mümkün olduğunca akıcı ve gecikmesiz gösterebilmek için
8. Sonuç istatistikleri ve çıktı
9. Genel Test ve test sonrası hataları giderme

Uygulama yerel çalışır, video akışları ve analiz sonuçları sunucuya gönderilmez. 

# Örnek Kullanım Senaryosu
Alışveriş merkezi otoparkından alınmış güvenlik kamerası kaydını uygulamaya yükledik. Ön izleme penceresi videonun seçildiğini onaylar. Ardından "Model Ekle" düğmesiyle yolov8n.onnx dosyasını içe aktarıp yalnızca car, truck ve bus sınıflarını işaretledik, böylece yayalar ve bisikletliler filtrelenir. Hareket analiz penceresinde minimum hız eşiğini 0.5 m/s, park edilmiş sayılacak minimum bekleme süresini ise 15 dakika olarak kaydedebiliriz. Başlat tuşuna bastığımızda uygulama kareleri çözüp GPU’da nesne tespiti yapar, Tracker algoritması ile araçları izler ve her kimlik için hız hesaplayarak gerçek-zamanlı ön izlemede duran araçları mavi, hareket halindekileri kırmızı box ile vurgular. Sağ panelde toplam araç sayısı, ortalama hız ve 15 dakikadan uzun park eden araç sayısı canlı olarak güncellenirken, akışı istediğimiz anda durdurulup devam ettirilebilir. İşlem tamamlandığında stats.csv ve json dosyaları dışa aktarılır. ilki araç başına giriş-çıkış zamanları ve toplam park süresini, ikincisi ise zaman damgalı konum ve hız dizilerini içerir. Örneğin iki saatlik kayıtta 127 araç algılanmış, bunlardan 14’ü 15 dakikadan uzun süre park halinde kalmış gibi bilgileri içerir.
İstenilen kullanım senaryosu bu şekilde
