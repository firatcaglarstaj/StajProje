#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>
#include <QMap>
#include <QList>
#include <QCloseEvent>
#include <qfiledialog.h>

#include "ai/yolocommunicator.h"
#include "core/FrameData.h"      // Temel veri yapıları için
#include "core/ThreadQueue.h"      // Thread-safe kuyruk için
#include "core/motioncontroller.h"
#include "core/videocontroller.h" // Video işçisi sınıfı için
#include "ai/yolocommunicator.h"  // YOLO işçisi sınıfı için

#include "motionwindow.h"
#include "MoveDetect.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QLabel;
class QProgressBar;
class QListWidgetItem;
class QCloseEvent;


/*

Uygulamanın ana penceresini ve iş akışını yöneten kısım

Bu sınıf, VideoController ve YOLOCommunicator işçilerini ayrı thread lerde çalıştırır.
UI güncellemelerini yapar, kullanıcı etkileşimlerini yönetir ve worker thread lerden
gelen verileri (görüntülenecek kareler, tespit sonuçları) işler.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /*
     Pencere kapatıldığında çağrılır.
     Arka plan thread lerinin güvenli bir şekilde sonlandırılmasını sağlar.
     */
    void closeEvent(QCloseEvent *event) override;

private slots:

    //  UI Eleman Slotları
    void on_pushButton_AddVideo_clicked();
    void on_pushButton_selectVideo_clicked();
    void on_listWidget_Videos_itemDoubleClicked(QListWidgetItem *item);
    void on_pushButton_PlayPause_clicked();
    void on_doubleSpinBox_PlaybackSpeed_valueChanged(double value);
    void on_horizontalSlider_sliderMoved(int position);
    void on_pushButton_SystemStatus_clicked();
    void on_pushButton_ChooseModel_clicked();
    void on_pushButton_MotionDetect_clicked(); //

    //  Worker Thread lerden Gelen Sinyaller İçin Slotlar
    void onVideoOpened(const VideoInfo& videoInfo);
    void onVideoFinished();
    void onProgressChanged(double progress);
    void onDetectionReceived(const DetectionResult& result);
    void onYOLOConnectionChanged(bool connected);
    void onYOLOError(const QString& error);


    //  Ana Thread Zamanlayıcı (Timer) Slotları
    void onDisplayTimer();//Görüntüleme kuyruğunu periyodik olarak kontrol eder ve yeni kare varsa ekrana basar.
    void onUIUpdateTimer();//FPS gibi anlık durum bilgilerini UI da günceller

    void onMotionResultReady();


private:

    //  Kurulum  ve Kontrol Fonksiyonları

    void setupStatusBar();
    void setupTimers();
    void setupThreads();
    void startVideoProcessing(const QString& videoPath);
    void stopVideoProcessing();


    //  Yardımcı ve UI Güncelleme Fonksiyonları

    void displayFrame(const FrameData& frameData);
    void drawDetections(cv::Mat& frame, const DetectionResult& result);
    QPixmap matToQPixmap(const cv::Mat& frame);
    QString selectVideoFile();
    void addVideoToList(const QString& filePath);
    void updateStatusBar(const QString& message);
    void showDebugInfo();
    double calculateMemoryUsage();
    void updateVideoInfo(const VideoInfo& videoInfo);
    void updatePerformanceInfo(const PerformanceStats& stats);
    void updateSeekSlider(double progress);
    void updateYOLOStatus();
    void updateDetectionStats();
    void updateFrameInfo(const FrameData& frameData, bool hasDetection, const DetectionResult& detection);
    void cleanupDetectionCache();


    Ui::MainWindow *ui;                      // Qt Designer ile oluşturulan UI elemanlarına erişim pointer ı.

    //  Thread ler arası iletişim için Kuyruklar
    FrameQueue displayQueue;                 // Video->UI: Görüntülenecek tüm kareleri tutan kuyruk.
    FrameQueue detectionQueue;               // Video->YOLO: Tespit edilecek kareleri (örn. her 6. kare) tutan kuyruk.
    // motion için
    FrameQueue motionQueue;
    MotionResultQueue motionResultQueue;

    //  Worker Sınıfları ve Onları Çalıştıran Thread ler
    VideoController *videoController;        // Video okuma işçisi.
    YOLOCommunicator *yoloCommunicator;      // YOLO/TCP iletişim işçisi.
    QThread *videoThread;                    // videoController ı çalıştıran thread.
    QThread *yoloThread;                     // yoloCommunicator ı çalıştıran thread.
    QThread *motionThread;


    //  Ana Thread Zamanlayıcıları
    QTimer* displayTimer = nullptr;                   // Görüntüleme kuyruğunu kontrol eden zamanlayıcı.
    QTimer* uiUpdateTimer = nullptr;                   // FPS gibi UI bileşenlerini güncelleyen zamanlayıcı.
    QElapsedTimer fpsTimer;                  // FPS hesaplaması için geçen süreyi ölçer.

    //  Status Bar Elemanları
    QLabel *statusLabel;
    QLabel *videoInfoLabel;
    QLabel *performanceLabel;
    QLabel *frameInfoLabel;
    QLabel *yoloStatusLabel;
    QProgressBar *memoryUsageBar;

    //  Durum Değişkenleri ve Veri Saklama
    QStringList videoFilesList;              // Playlist e eklenen video dosyalarının yolları.
    QString currentVideoPath;                // O an işlenen video dosyasının yolu.
    bool isVideoLoaded = false;
    bool isYOLOConnected = false;
    bool isYOLOEnabled = false;
    bool isPlaying = false;
    int frameCounter = 0;
    double currentDisplayFPS = 0.0;
    int totalDetectionsCount = 0;
    FrameData currentFrameData;              // UI da en son gösterilen kare verisi.

    //  Tespit Sonuçları Önbelleği (Cache)
    QMap<int, DetectionResult> detectionResults; // Frame ID sine göre tespit sonuçlarını saklar.
    DetectionResult lastValidDetection;      // Ekranda gösterilen son geçerli tespit.
    int lastDetectionFrameId = -1;
    const int DETECTION_PERSISTENCE = 15;     // Bir tespitin ekranda kalma süresi (kare sayısı).
    void setupSignalConnections();
    void cleanupThreads();

    bool isSliderDragging = false;


    //  Yeni eklediğim pencere için
    MotionWindow *motionWindow;
    MoveDetect::Handler *motionHandler;
    bool isMotionDetectActive;

    MotionController *motionController;
};

#endif // MAINWINDOW_H
