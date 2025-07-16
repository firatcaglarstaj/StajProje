#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QtMultimediaWidgets/qvideowidget.h>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QDebug>
#include <QTimer>
#include <QShortcut>
#include <QMenu>
#include <QInputDialog>
#include <QKeySequence>
#include <QThread>
#include<opencv2/opencv.hpp>
#include "core/FrameData.h"
#include "core/ThreadQueue.h"
#include "core/VideoController.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_AddVideo_clicked();

    void on_pushButton_selectVideo_clicked();

    void on_horizontalSlider_sliderMoved(int position);

    void on_listWidget_Videos_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_PlayPause_clicked();

    void on_doubleSpinBox_PlaybackSpeed_valueChanged(double arg1);

    void on_pushButton_SystemStatus_clicked();


    // VIDEO CONTROLLER SLOTLARI

    void onFrameReady(const FrameData& frameData); //VideoController dan yeni frame geldiğinde


    void onVideoOpened(const VideoInfo& videoInfo); // Video açıldığında


    void onVideoClosed(); // Video Kapandığında


    void onPlaybackStateChanged(VideoController::PlaybackState state); //Playback state değiştiğinde


    void onProgressChanged(double progress); //Video progress değiştiğinde


    void onPerformanceUpdated(const PerformanceStats& stats); //Performance istatistikleri güncellendiğinde





    // TIMER SLOTLARI

    void onUIUpdateTimer(); //UI güncelleme timer ı



private:
    Ui::MainWindow *ui;

    VideoController* videoController;       // Video yönetimi
    FrameQueue* testFrameQueue;            // Test amaçlı frame queue
    QStringList videoFilesList;            // Eklenen video dosyaları listesi
    QString currentVideoPath;              // Şu an açık olan video
    bool isVideoLoaded;                    // Video yüklü mü?
    bool isPlaying;                        // Oynatılıyor mu?
    QLabel* statusLabel;                   // Genel durum mesajı
    QLabel* videoInfoLabel;                // Video bilgileri
    QLabel* performanceLabel;              // Performance istatistikleri
    QLabel* frameInfoLabel;                // Mevcut frame bilgileri
    QProgressBar* memoryUsageBar;          // Memory kullanım göstergesi
    QTimer* uiUpdateTimer;                 // UI güncelleme
    QTimer* queueMonitorTimer;             // Queue monitoring
    QElapsedTimer fpsTimer;                // FPS hesaplama için
    int frameCounter;                      // İşlenen frame sayısı
    double currentDisplayFPS;              // Mevcut display FPS

    void initializeUI(); //UI bileşenlerini initialize et
    void setupVideoController(); //VideoController ı setup et
    void setupStatusBar();
    void setupTimers();
    void connectSignals(); //Signal-slot bağlantılarını kur
    void initializeTestQueue();

    void displayFrame(const FrameData& frameData); //Frame i video display widget ında göster
    QPixmap matToQPixmap(const cv::Mat& frame); // QT arayüzünden göstermek için cv::Mat → QPixmap dönüşümü zorunlu
    void updateVideoControls(VideoController::PlaybackState state); //Video kontrol butonlarını güncelle
    void updateVideoInfo(const VideoInfo& videoInfo); //Video bilgilerini UI da göster
    void updatePerformanceInfo(const PerformanceStats& stats); //Performance bilgilerini UI da göster
    void updateStatusBar(const QString& message);
    void updateSeekSlider(double progress);

     QString selectVideoFile();
    void addVideoToList(const QString& filePath);
    void removeVideoFromList(int index);
    void clearVideoList();

    void testThreadSafeQueue();
    void logSystemStatus();
    void showDebugInfo();
    double calculateMemoryUsage();

    QString formatTime(double seconds); //Zaman formatını okunabilir yap
    QString formatFileSize(qint64 bytes); // Dosya boyutunu okunabilir yap
    void showErrorMessage(const QString& title, const QString& message);
};
#endif // MAINWINDOW_H
