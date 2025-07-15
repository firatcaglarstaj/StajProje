#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "FrameData.h"

/*
Video dosyalarını kontrol eden ana sınıf
 Bu sınıf video dosyası açma, oynatma, durdurma, frame navigation
 gibi tüm video operasyonlarını yönetir. OpenCV VideoCapture ı çalıştırır
ve Qt signals/slots sistemi ile ui ile iletişim kurar
 */

class VideoController: public QObject
{
     Q_OBJECT
public:
     //Video Oynatma durumları
    enum PlaybackState {
        Stopped,    // Video durdurulmuş, başa alınmış
        Playing,    // Video oynatılıyor
        Paused,     // Video duraklatılmış
        Finished,   // Video sonuna ulaşmış
        Error       // Hata durumu
    };
    Q_ENUM(PlaybackState)  // Qt için enum registration

private:
    cv::VideoCapture videoCapture; // OpenCV video capture objesi
    VideoInfo currentVideoInfo;
    PlaybackState currentState;

    //Frame Tracking için
    int nextFrameId; //Bir sonraki frame e atanacak ID
    int targetFPS; //İstenen FPS (Performans ölçümü için değiştirilebilir)
    double playbackSpeed;// Oynatma hızı

    // Performance izleme

    QElapsedTimer performanceTimer; // Performance ölçümü için
    PerformanceStats performanceStats;
    QTimer* fpsCalculationTimer;    // FPS hesaplama timer ı
    QTimer* playbackTimer;          // Video oynatma timer ı

public:
    explicit VideoController(QObject *parent = nullptr);
      ~VideoController();

    /*
    Video dosyası aç
    filePath Video dosyasının tam yolu
    Başarılı ise true
     */
    bool openVideo(const QString& filePath);

    /*
     Video dosyasını kapat ve kaynakları temizle
     */
    void closeVideo();

    /*
    Video açık mı kontrol et
     */
    bool isVideoOpen() const { return videoCapture.isOpened(); }

    /*
    Video oynatmaya başla
    */
    void play();

    /*
    Videoyu duraklat
    */
    void pause();

    /*
     Videoyu durdur ve başa al
     */
    void stop();

    /*
     Sonraki frame i al
     Frame data, invalid ise boş FrameData döndür
     */
    FrameData getNextFrame();

    /*
    Mevcut frame i al (pozisyonu değiştirmeden)
    Mevcut frame data döndür
     */
    FrameData getCurrentFrame();

    // Navigasyon İşlemleri için

    /*
    Belirli frame numarasına git
    frameNumber Gidilecek frame numarası ile
     */
    void seekToFrame(int frameNumber);

    /*
    Belirli zamana git
    timeInSeconds Gidilecek zaman (saniye)
     */
    void seekToTime(double timeInSeconds);

    /*
     Oynatma hızını ayarla
    speed Hız çarpanı (1.0 = normal, 2.0 = 2x hızlı, 0.5 = yarı hız)
     */
    void setPlaybackSpeed(double speed);

    // Getter metodları

    VideoInfo getVideoInfo() const { return currentVideoInfo; }
    PlaybackState getPlaybackState() const { return currentState; }
    double getPlaybackSpeed() const { return playbackSpeed; }
    PerformanceStats getPerformanceStats() const { return performanceStats; }
    int getTotalFrames() const { return currentVideoInfo.totalFrames; }
    double getDuration() const { return currentVideoInfo.duration; }
    double getCurrentTime() const { return currentVideoInfo.currentTime; }
    double getFPS() const { return currentVideoInfo.fps; }

signals:

    void frameReady(const FrameData& frameData); //Yeni frame hazır olduğunda active edilir,  frameData = İşlenmiş frame verisi
    void videoOpened(const VideoInfo& videoInfo); // Video açıldığında active edilir, video bilgilerini parametre alır
    void videoClosed(); // video kapandığında active edilir
    void playbackStateChanged(PlaybackState newState);
    void performanceUpdated(const PerformanceStats& stats); // Perfomance istatistikleri güncelleğinde active olur, stats = güncellenen performans bilgileri
    void onPlaybackTimer(); //Playback timer ı tetiklendiğinde çağrılır
    void onPerformanceTimer(); // Performans hesaplama timer ı tetiklendiğinde çağrılır

private:
    /*
    Video bilgilerini OpenCV'den oku ve güncelle
    Başarılı ise true
    */
    bool updateVideoInfo();

    /*
    Video bilgilerini sıfırla
    */
    void resetVideoInfo();

    /*
    OpenCV Mat ten FrameData oluştur
    frame = OpenCV Mat objesi
    Doldurulmuş FrameData döndür
    */
    FrameData createFrameData(const cv::Mat& frame);

    void setState(PlaybackState newState);//Oynatma durumunu değiştir ve signal active et

    void startTimers(); //Timer ları başlat

    void stopTimers(); // Timer'ları durdur

    void updatePerformanceStats(); //Performance istatistiklerini güncelle


    /*
     Video dosyası geçerli mi kontrol et
     filePath Kontrol edilecek dosya yolu
     Geçerli ise true
     */
    bool isValidVideoFile(const QString& filePath);

};




#endif // VIDEOCONTROLLER_H
