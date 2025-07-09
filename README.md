# Motion Detection Projesi


---

##  Projenin Amacı

- Videodan nesne tespiti yapılır (YOLOv9, YOLOv10, YOLOv11 gibi modellerle).
- Algılanan nesneler video boyunca kare kare izlenir.
- İzlenen nesnelerin hareket analizi gerçekleştirilir.
- Hareket olayları gerçek zamanlı olarak takip edilir.
- Sonuçlar ve istatistikler dışa aktarılır.

---

##  Temel Özellikler

- **Ana Pencere İskeleti**
- **Video Seçici**
- **Model Export / Import**
- **Model Seçme Paneli**
- **Nesne Seçme Paneli** *(Checkbox ile hangi nesnelerin izleneceği belirlenir)*
- **Hareket Analizi Ayarları** *(Min/max hız eşiği vb.)*
- **Process** *(Parametreleri onaylayarak analizi başlatır)*
- **Başlat/Durdur** *(Analizi kontrol eder; duraklatma ve devam ettirme sağlar)*
- **Real-Time Ön İzleme**
- **Sonuç İstatistiklerini Kaydetme** *(Ortalama hız, toplam hareket süresi, FPS gibi metrikleri CSV veya JSON olarak dışa aktarır)*
- **Sonuç Çıktılarını Kaydetme** *(Nesne ID, zaman damgası, pozisyon, hız bilgileri JSON veya CSV olarak dışa aktarılır)*

Projenin geliştirilmesi yukarıdaki bileşenlerin sırasıyla eklenmesiyle yapılacaktır.

---

## Kullanılan Teknolojiler

- **QT 6.9.1**
- **QT Creator 17.0**
- **OpenCV**
- **YOLO Modelleri (YOLOv9, YOLOv10, YOLOv11)**

---

##  Geliştirme Yol Haritası

Projenin geliştirilmesi aşağıdaki aşamalarda gerçekleştirilecektir:
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


## Örnek Kullanım Senaryosu
Alışveriş merkezi otoparkından alınmış güvenlik kamerası kaydını uygulamaya yükledik. Ön izleme penceresi videonun seçildiğini onaylar. Ardından "Model Ekle" düğmesiyle yolov8n.onnx dosyasını içe aktarıp yalnızca car, truck ve bus sınıflarını işaretledik, böylece yayalar ve bisikletliler filtrelenir. Hareket analiz penceresinde minimum hız eşiğini 0.5 m/s, park edilmiş sayılacak minimum bekleme süresini ise 15 dakika olarak kaydedebiliriz. Başlat tuşuna bastığımızda uygulama kareleri çözüp GPU’da nesne tespiti yapar, Tracker algoritması ile araçları izler ve her kimlik için hız hesaplayarak gerçek-zamanlı ön izlemede duran araçları mavi, hareket halindekileri kırmızı box ile vurgular. Sağ panelde toplam araç sayısı, ortalama hız ve 15 dakikadan uzun park eden araç sayısı canlı olarak güncellenirken, akışı istediğimiz anda durdurulup devam ettirilebilir. İşlem tamamlandığında stats.csv ve json dosyaları dışa aktarılır. ilki araç başına giriş-çıkış zamanları ve toplam park süresini, ikincisi ise zaman damgalı konum ve hız dizilerini içerir. Örneğin iki saatlik kayıtta 127 araç algılanmış, bunlardan 14’ü 15 dakikadan uzun süre park halinde kalmış gibi bilgileri içerir.
İstenilen kullanım senaryosu bu şekilde
