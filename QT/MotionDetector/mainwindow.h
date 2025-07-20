#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ai/DetectionData.h"
#include "core/videocontroller.h"
#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QMap>

#include <opencv2/opencv.hpp> // OpenCV nin ana başlık dosyası

// Qt UI sınıfının ileriye dönük bildirimi (forward declaration)
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Kendi özel sınıflarımızın ileriye dönük bildirimleri.
// Bu, başlık dosyasına tam sınıf tanımını dahil etme ihtiyacını ortadan kaldırır
// ve derleme süresini kısaltır.
class VideoController;
class FrameQueue;
class YOLOCommunicator;

// Sinyaller ve slotlar arasında veri taşımak için kullanılan struct ların
// ileriye dönük bildirimleri.
struct FrameData;
struct VideoInfo;
struct PerformanceStats;
struct DetectionResult;
struct Detection;

/**
 * @class MainWindow
 * @brief Uygulamanın ana penceresini yöneten sınıf.
 *
 * Bu sınıf, kullanıcı arayüzü (UI) elemanlarını oluşturur, video kontrolü,
 * YOLO servisi ile iletişim ve genel uygulama mantığını yönetir.
 * Video oynatma, duraklatma, hız ayarı gibi kontrolleri sağlar ve
 * gelen video frame lerini işleyerek ekranda gösterir.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT // Qt nin sinyal/slot mekanizması için zorunlu makro

public:
    /**
     * @brief MainWindow sınıfının kurucu fonksiyonu.
     * @param parent Üst QWidget nesnesi.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief MainWindow sınıfının yıkıcı fonksiyonu.
     *
     * Uygulama kapatıldığında bellek sızıntılarını önlemek için
     * oluşturulan tüm dinamik nesneleri (pointer lar) temizler.
     */
    ~MainWindow();

private slots:
    //================================================================
    //== UI Eleman Slotları (Qt Designer tarafından otomatik bağlanır)
    //================================================================
    void on_pushButton_AddVideo_clicked();
    void on_pushButton_selectVideo_clicked();
    void on_listWidget_Videos_itemDoubleClicked(QListWidgetItem *item);
    void on_pushButton_PlayPause_clicked();
    void on_doubleSpinBox_PlaybackSpeed_valueChanged(double value);
    void on_horizontalSlider_sliderMoved(int position);
    void on_pushButton_SystemStatus_clicked();
    void on_pushButton_ChooseModel_clicked();

    //================================================================
    //== VideoController Sinyalleri İçin Slotlar
    //================================================================
    void onFrameReady(const FrameData& frameData);
    void onVideoOpened(const VideoInfo& videoInfo);
    void onVideoClosed();
    void onPlaybackStateChanged(PlaybackState state);
    void onProgressChanged(double progress);
    void onPerformanceUpdated(const PerformanceStats& stats);

    //================================================================
    //== YOLOCommunicator Sinyalleri İçin Slotlar
    //================================================================
    void onDetectionReceived(const DetectionResult& result);
    void onYOLOConnectionChanged(bool connected);
    void onYOLOError(const QString& error);

    //================================================================
    //== Zamanlayıcı (Timer) Slotu
    //================================================================
    void onUIUpdateTimer();

private:
    //================================================================
    //== Kurulum (Setup) Fonksiyonları
    //================================================================
    void setupVideoController();
    void setupYOLOCommunicator();
    void setupStatusBar();
    void setupTimers();
    void connectSignals();
    void initializeTestQueue();

    //================================================================
    //== Yardımcı ve UI Güncelleme Fonksiyonları
    //================================================================
    void displayFrame(const FrameData& frameData);
    QPixmap matToQPixmap(const cv::Mat& frame);
    QString selectVideoFile();
    void addVideoToList(const QString& filePath);
    void updateStatusBar(const QString& message);
    void showDebugInfo();
    double calculateMemoryUsage();
    void updateVideoControls(PlaybackState state);
    void updateVideoInfo(const VideoInfo& videoInfo);
    void updatePerformanceInfo(const PerformanceStats& stats);
    void updateSeekSlider(double progress);
    void updateYOLOStatus();
    void updateDetectionStats();
    void updateFrameInfo(const FrameData& frameData, bool hasDetection, const DetectionResult& detection);
    void drawDetections(cv::Mat& frame, const DetectionResult& result);
    void cleanupDetectionCache();

    Ui::MainWindow *ui;

    // Status Bar Elemanları
    QLabel *statusLabel;                     // Genel sistem durumu mesajını gösterir.
    QLabel *videoInfoLabel;                  // Yüklenen video bilgilerini (çözünürlük, FPS) gösterir.
    QLabel *performanceLabel;                // Video işleme performansını (FPS) gösterir.
    QLabel *frameInfoLabel;                  // Mevcut frame numarasını, timestamp ini ve tespit bilgilerini gösterir.
    QLabel *yoloStatusLabel;                 // YOLO servis bağlantı durumunu gösterir.
    QProgressBar *memoryUsageBar;            // Uygulamanın bellek kullanımını gösterir.

    // Kontrolcüler ve İletişimciler
    VideoController *videoController;        // Video okuma, oynatma ve kontrol işlemlerini yönetir.
    FrameQueue *testFrameQueue;              // Test ve debug amaçlı frame leri saklayan kuyruk.
    YOLOCommunicator *yoloCommunicator;      // YOLOv8 servisi ile iletişimi yönetir.

    //   Zamanlayıcılar (Timers)
    QTimer *uiUpdateTimer;                    // UI elemanlarını periyodik olarak güncelleyen timer.
    QElapsedTimer fpsTimer;                   // FPS hesaplaması için geçen süreyi ölçer.

    //  Durum Değişkenleri (State Variables)
    bool isVideoLoaded;                       // Bir video dosyasının yüklenip yüklenmediğini belirtir.
    bool isYOLOConnected;                     // YOLO servisine bağlı olup olmadığını belirtir.
    bool isYOLOEnabled;                       // YOLO tespitlerinin aktif olup olmadığını belirtir.
    bool isPlaying;                           // Videonun anlık olarak oynatılıp oynatılmadığını belirtir.

    //  Veri Saklama ve Sayaçlar
    QStringList videoFilesList;               // Playlist e eklenen video dosyalarının tam yollarını tutar.
    QString currentVideoPath;                 // O an işlenen video dosyasının yolu.
    int frameCounter;                         // Başlangıçtan itibaren işlenen toplam frame sayısı.
    double currentDisplayFPS;                 // Anlık olarak ekranda gösterilen FPS değeri.
    QMap<int, DetectionResult> detectionResults;  // Frame ID sine göre tespit sonuçlarını saklayan önbellek (cache).
    DetectionResult lastValidDetection;       // Ekranda gösterilmek üzere kullanılan son geçerli tespit sonucu.
    int lastDetectionFrameId = -1;            // Son tespitin yapıldığı frame in ID si.
    long long totalDetectionsCount = 0;       // Oturum boyunca yapılan toplam tespit sayısı.

    //   Sabitler
    const int DETECTION_PERSISTENCE = 5;      // Bir tespit sonucunun, yeni bir tespit gelene kadar sonraki kaç frame boyunca ekranda kalacağını belirler.
};

#endif // MAINWINDOW_H
