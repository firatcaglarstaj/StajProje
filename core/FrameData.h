#ifndef FRAMEDATA_H
#define FRAMEDATA_H
#include <opencv2/opencv.hpp>
#include <QString>
#include <QDebug>
#include <QDateTime>
/*
Video frame'lerini takip etmek için temel veri yapısı
Bu struct her video frame i için gerekli bilgileri tutar.
Frame ID sistemi ile hangi frame in ne zaman işlendiğini takip edebiliriz ve frame kontrolü bizde olur.
 */
struct FrameData {
    int frameId = -1; // Kendi atayacağımız benzersiz frame kimliği
    double timeStamp = 0.0; // Video içindeki saniye cinsinden zaman
    int frameNumber = -1; //Video dosyasındaki frame numarası
    cv::Mat frame; // Görüntü verisi
    bool processed = false; //Bu frame işlendi mi?
    bool isValid() const {
        return frameId >= 0 && !frame.empty();
    }
    // Okuma kolaylığı için bir yapı
    QString toString() const {
        return QString("Frame[ID:%1, Time:%2s, Number:%3, Size:%4x%5, Processed:%6]")
        .arg(frameId)
            .arg(timeStamp, 0, 'f', 3)
            .arg(frameNumber)
            .arg(frame.cols)
            .arg(frame.rows)
            .arg(processed ? "Yes" : "No");
    }
    double getSizeMB() const {
        if (frame.empty()) return 0.0;
        size_t bytes = frame.total() * frame.elemSize();
        return static_cast<double>(bytes) / (1024.0 * 1024.0); //MB DÖNÜŞÜMÜ
    }
};
/*
 Video dosyası hakkında kapsamlı bilgiler
 VideoController tarafından doldurulur UI tarafından gösterilir.
 */
struct VideoInfo {
    QString filePath;           // Video dosyasının tam yolu
    QString fileName;           // Sadece dosya adı
    int totalFrames = 0;        // Toplam frame sayısı
    double fps = 0.0;           // Video FPS değeri
    double duration = 0.0;      // Toplam süre (saniye)
    int width = 0;              // Video genişliği
    int height = 0;             // Video yüksekliği
    double currentTime = 0.0;   // Mevcut oynatma zamanı
    int currentFrameNumber = 0; // Mevcut frame numarası
    size_t fileSize = 0;        // Dosya boyutu (byte)
    // Video bilgilerini aynı şekilde okunabilir formatta döndür
    QString toString() const {
        return QString("Video[%1, %2x%3, %4FPS, %5s, %6 frames, %7MB]")
        .arg(fileName)
            .arg(width)
            .arg(height)
            .arg(fps, 0, 'f', 1)
            .arg(duration, 0, 'f', 1)
            .arg(totalFrames)
            .arg(static_cast<double>(fileSize) / (1024.0 * 1024.0), 0, 'f', 1);
    }
    // Video geçerli mi?
    bool isValid() const{
        return totalFrames > 0 && fps > 0 && width > 0 && height > 0;
    }
    // Progress hesapla (0.0 - 1.0)
    double getProgress() const {
        if (totalFrames <= 0) return 0.0;
        return static_cast<double>(currentFrameNumber) / totalFrames;
    }
};
/*
 Performance takip için bilgiler
 istem performansını takip etmek için kullanılır
 */
struct PerformanceStats {
    int framesProcessed = 0;    // İşlenen toplam frame sayısı
    double averageFrameTime = 0.0;  // Ortalama frame işleme süresi ms cinsinden
    double currentFPS = 0.0;    // Mevcut FPS değeri
    double memoryUsage = 0.0;   // Memory kullanımı mb cinsinden
    QDateTime startTime;        // Başlangıç zamanı
    QString toString() const {
        return QString("Perf[Frames:%1, FPS:%.1f, AvgTime:%.2fms, Memory:%.1fMB]")
        .arg(framesProcessed).arg(currentFPS, 0, 'f', 1).arg(averageFrameTime, 0, 'f', 2).arg(memoryUsage, 0, 'f', 1);
    }
};
#endif //FRAMEDATA_H
