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

## Python ile Hibrit Yaklaşım
### Python Rolü
 - **Model Tipini Belirler**
 - **Parametreleri Ayarlar**
 - **Konfigürasyonu Döndürür**
 ### C++ Rolü
 - **Real-time frame processing**
 - **OpenCV optimizasyonları**
 - **Performans kontrolü**

 ## Model Seçme Sisteminin Çalışma Akışı
1. Model Seçimi
2. Python Config: Python script konfigürasyonu döndürür
3. C++ tarafında OpenCV detectorları başlatılır
4. Her frame C++ ile işlenir
5. Motion detection sonuçları gerçek zamanlı

## Asenkron Mimari 
- Proje python ve C++ ile beraber çalışacak. Bu yüzden hız farkından dolayı görüntünün akıcı bir şekilde işlenmesi için asenkron, katmanlı bir mimari tasarlandı
- DATA LAYER -> VIDEO PROCESSING (C++)  │  AI PROCESSING (Python)  -> COMMUNICATION LAYER -> UI LAYER (Qt C++)   
### DATA LAYER 
• OpenCV (Video I/O)  • Ultralytics YOLO                 
• Qt Framework        • NumPy/OpenCV       
### VIDEO PROCESSING (C++)  │  AI PROCESSING (Python) 
• C++ tarafında VideoController, Frame Management , Threading
• Python Tarafında • YOLO Inference • Result Formatting • Model Management 
### COMMUNICATION LAYER
• TCP Socket + JSON Protocol (Qt TCP) 
### UI LAYER (Qt C++) 
• Video Display • Controls • Status • Results Visualization 

### Thread Mimarisi Veri Akışı
1. **Video Thread:** Video frame i okur 
2. **Display Thread:** Frame i anında UI da gösterir 
3. **TCP Thread:** Her 6. frame i Python'a gönderir 
4. **Python Process:** YOLO inference yapar 
5. **TCP Thread:** Sonucu alır ve cache ler
6. **Display Thread:** En güncel AI sonucunu frame üzerine çizer
   
#### **Mesaj Tipleri:**
- **frame_request:** C++ → Python (frame analizi için)
- **detection_result:** Python → C++ (AI sonuçları)
- **control_command:** C++ → Python (model değiştirme vb.)
- **status_update:** Python → C++ (durum bilgileri)
- **error:** Her iki yön (hata bildirimleri)
- **heartbeat:** Bağlantı canlılık kontrolü
