#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include "core/FrameData.h"
#include "core/ThreadQueue.h"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <opencv2/opencv.hpp>
#include <atomic> // Döngüyü güvenli bir şekilde durdurmak için

class VideoController : public QObject
{
    Q_OBJECT

public:
    // Kurucu fonksiyon artık kuyrukları parametre olarak alıyor
    explicit VideoController(FrameQueue* displayQueue, QObject *parent = nullptr);
    ~VideoController();

    // Bu metodlar hala Ana Thread den çağrılabilir
    bool openVideo(const QString &filePath);
    void closeVideo();
    bool openVideoDirectly(const QString& filePath); ////////////
public slots:
    // Bu slotlar thread başladığında veya durdurulmak istendiğinde çağrılacak
    void startProcessing();
    void stopProcessing();

signals:
    // Görüntüleme kuyruğuna yeni bir kare eklendiğini Ana Thread e bildirir
    void frameAvailable();
    // Video hakkında bilgi ve durum sinyalleri
    void videoOpened(const VideoInfo& videoInfo);
    void videoFinished();
    void progressChanged(double progress);

private:
    FrameData createFrameData(const cv::Mat& frame);
    bool updateVideoInfo();
    void resetVideoInfo();

    cv::VideoCapture videoCapture;
    VideoInfo currentVideoInfo;
    int nextFrameId;

    // Worker ların iletişim kuracağı kuyruklar
    FrameQueue* displayQueue;
    FrameQueue* detectionQueue;

    // Döngünün çalışıp çalışmadığını kontrol eden thread-safe bayrak
    std::atomic<bool> isRunning;
};

#endif // VIDEOCONTROLLER_H
