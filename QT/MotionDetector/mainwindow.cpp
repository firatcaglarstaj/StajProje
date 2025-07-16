#include "MainWindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoController(nullptr)
    , testFrameQueue(nullptr)
    , isVideoLoaded(false)
    , isPlaying(false)
    , frameCounter(0)
    , currentDisplayFPS(0.0)
{
    ui->setupUi(this);

    //initializeUI();
    setupVideoController();
    setupStatusBar();
    setupTimers();
    connectSignals();
    initializeTestQueue();
    updateStatusBar("Sistem hazır - Video dosyası seçin");
}

MainWindow::~MainWindow()
{

    // Timer ları durdur
    if (uiUpdateTimer) {
        uiUpdateTimer->stop();
    }

    // VideoController ı temizle
    if (videoController) {
        videoController->closeVideo();
        delete videoController;
    }

    // Test queue sunu temizle
    if (testFrameQueue) {
        delete testFrameQueue;
    }

    delete ui;
    qDebug() << "MainWindow: Temizlik tamamlandı";
}

void MainWindow::setupVideoController()
{
    // VideoController oluştur
    videoController = new VideoController(this);
}

void MainWindow::setupStatusBar()
{
    qDebug() << "MainWindow: Status bar setup ediliyor...";

    // Status bar elemanlarını oluştur
    statusLabel = new QLabel("Sistem: Hazır");
    videoInfoLabel = new QLabel("Video: Yüklenmedi");
    performanceLabel = new QLabel("FPS: --");
    frameInfoLabel = new QLabel("Frame: --");

    // Memory usage bar
    memoryUsageBar = new QProgressBar();
    memoryUsageBar->setRange(0, 100);
    memoryUsageBar->setValue(0);
    memoryUsageBar->setTextVisible(true);
    memoryUsageBar->setFormat("Memory: %p%");
    memoryUsageBar->setMaximumWidth(120);

    // Status bar a ekle
    ui->statusbar->addWidget(statusLabel);
    ui->statusbar->addWidget(videoInfoLabel);
    ui->statusbar->addPermanentWidget(frameInfoLabel);
    ui->statusbar->addPermanentWidget(performanceLabel);
    ui->statusbar->addPermanentWidget(memoryUsageBar);

    qDebug() << "MainWindow: Status bar setup tamamlandı";
}

void MainWindow::setupTimers()
{

    // UI güncelleme timer ı (her 100ms)
    uiUpdateTimer = new QTimer(this);
    connect(uiUpdateTimer, &QTimer::timeout, this, &MainWindow::onUIUpdateTimer);
    uiUpdateTimer->start(100);

    // FPS hesaplama timer'ını başlat
    fpsTimer.start();

    qDebug() << "MainWindow: Timer'lar başlatıldı";
}

void MainWindow::connectSignals()
{
    qDebug() << "MainWindow: Signal-slot bağlantıları kuruluyor...";

    // VideoController signals
    connect(videoController, &VideoController::frameReady,
            this, &MainWindow::onFrameReady);
    connect(videoController, &VideoController::videoOpened,
            this, &MainWindow::onVideoOpened);
    connect(videoController, &VideoController::videoClosed,
            this, &MainWindow::onVideoClosed);
    connect(videoController, &VideoController::playbackStateChanged,
            this, &MainWindow::onPlaybackStateChanged);
    connect(videoController, &VideoController::progressChanged,
            this, &MainWindow::onProgressChanged);
    connect(videoController, &VideoController::performanceUpdated,
            this, &MainWindow::onPerformanceUpdated);

    qDebug() << "MainWindow: Signal-slot bağlantıları tamamlandı";
}

void MainWindow::initializeTestQueue()
{
    qDebug() << "MainWindow: Test frame queue initialize ediliyor...";

    // Test için frame queue oluştur
    testFrameQueue = new FrameQueue(30); // 30 frame buffer

    qDebug() << "MainWindow: Test frame queue oluşturuldu";
}

void MainWindow::on_pushButton_AddVideo_clicked()
{
       // Video dosyası seç
        QString filePath = selectVideoFile();

    if (!filePath.isEmpty()) {
        // Listeye ekle
        addVideoToList(filePath);

        QFileInfo fileInfo(filePath);


        qDebug() << "MainWindow: Video eklendi:" << filePath << "Video İsmi:"<< fileInfo.fileName();
    }
}


void MainWindow::on_pushButton_selectVideo_clicked()
 {

        // Listeden seçilen video yu al
        int selectedRow = ui->listWidget_Videos->currentRow();

        // Video dosyasını aç
        QString selectedVideoPath = videoFilesList[selectedRow];

        qDebug() << "MainWindow: Video açılıyor:" << selectedVideoPath;

        if (videoController->openVideo(selectedVideoPath)) {
            currentVideoPath = selectedVideoPath;
            isVideoLoaded = true;

            // UI güncellemelerini VideoController signals halledecek
            qDebug() << "MainWindow: Video başarıyla açıldı";
        } else {
            qDebug() << "Video açma başarısız";
        }

 }

    void MainWindow::on_listWidget_Videos_itemDoubleClicked(QListWidgetItem *item)
    {

        // Double click ile video seç
        on_pushButton_selectVideo_clicked();
    }


    void MainWindow::on_pushButton_PlayPause_clicked()
    {

        if (isPlaying) {
            // Pause
            videoController->pause();
            qDebug() << "Video duraklatıldı";
        } else {
            // Play
            videoController->play();
            qDebug() << "Video oynatılıyor";
        }
    }



    void MainWindow::on_doubleSpinBox_PlaybackSpeed_valueChanged(double value)
    {
        if (videoController) {
            videoController->setPlaybackSpeed(value);
        }
    }

    void MainWindow::on_horizontalSlider_sliderMoved(int position)
    {
        if (!isVideoLoaded) {
            return;
        }

        // Slider pozisyonunu frame numarasına çevir
        int totalFrames = videoController->getTotalFrames();
        int targetFrame = (position * totalFrames) / 100;

        // Frame e git
        videoController->seekToFrame(targetFrame);
    }

    void MainWindow::on_pushButton_SystemStatus_clicked()
    {
        showDebugInfo();
    }

    // VİDEO CONTROLLER SLOTLARI
    void MainWindow::onFrameReady(const FrameData& frameData)
    {
        try {
            qDebug() << "MainWindow: Frame alındı - ID:" << frameData.frameId;

            // Frame geçerli mi kontrol et
            if (!frameData.isValid()) {
                qDebug() << "MainWindow: Geçersiz frame - skip";
                return;
            }

            // Frame'i display et
            displayFrame(frameData);

            // Test queue'suna güvenli ekle
            if (testFrameQueue) {
                testFrameQueue->push(frameData);
            }

            // Frame counter güncelle
            frameCounter++;

            // Debug log (her 10 frame'de bir)
            if (frameCounter % 10 == 0) {
                qDebug() << "MainWindow: Frame sayısı:" << frameCounter;
            }

        } catch (const std::exception& e) {
            qDebug() << "MainWindow: onFrameReady exception:" << e.what();
            // Video'yu durdur
            if (videoController) {
                videoController->pause();
            }
        } catch (...) {
            qDebug() << "MainWindow: onFrameReady unknown exception!";
            if (videoController) {
                videoController->pause();
            }
        }
    }

    void MainWindow::onVideoOpened(const VideoInfo& videoInfo)
    {
        qDebug() << "Video açıldı siNYAL alındı";

        updateVideoInfo(videoInfo);  // Video bilgilerini UI da göster

        // Slider ayarla
        ui->horizontalSlider->setRange(0, 100);
        ui->horizontalSlider->setValue(0);

        // Status güncelle
        updateStatusBar(QString("Video yüklendi: %1").arg(videoInfo.fileName));
    }

    void MainWindow::onVideoClosed()
    {
        qDebug() << "Video kapatıldı sinyal alındı";

        // UI sıfırla
        isVideoLoaded = false;
        isPlaying = false;
        frameCounter = 0;

        // Video display ini temizle
        ui->label_VideoDisplay->clear();
        ui->label_VideoDisplay->setText("Video Display Area");

        updateStatusBar("Video kapatıldı");

    }

    void MainWindow::onPlaybackStateChanged(VideoController::PlaybackState state)
    {
        qDebug() << "Playback state değişti:" << state;

        updateVideoControls(state);
    }

    void MainWindow::onProgressChanged(double progress)
    {
        updateSeekSlider(progress); // Slider güncelle
    }

    void MainWindow::onPerformanceUpdated(const PerformanceStats& stats)
    {
        updatePerformanceInfo(stats);  // Performance bilgilerini UI da göster
    }
    ////////////////////////////////

    void MainWindow::onUIUpdateTimer()
    {
        // FPS hesapla
        static int lastFrameCounter = 0;
        int frameDiff = frameCounter - lastFrameCounter;
        lastFrameCounter = frameCounter;


        currentDisplayFPS = frameDiff * 10.0;

        // Memory usage güncelle
        double memoryUsage = calculateMemoryUsage();
        memoryUsageBar->setValue(static_cast<int>(memoryUsage));
    }

    void MainWindow::displayFrame(const FrameData& frameData)
    {
        try {
            qDebug() << "MainWindow: Frame display ediliyor - ID:" << frameData.frameId;

            // Frame boş mu kontrol et
            if (frameData.frame.empty()) {
                qDebug() << "MainWindow: Frame boş - skip";
                return;
            }

            // UI widget var mı kontrol et
            if (!ui->label_VideoDisplay) {
                qDebug() << "MainWindow: Video display widget yok!";
                return;
            }

            // Frame boyutunu kontrol et
            if (frameData.frame.cols <= 0 || frameData.frame.rows <= 0) {
                qDebug() << "MainWindow: Frame boyutu geçersiz:" << frameData.frame.cols << "x" << frameData.frame.rows;
                return;
            }

            qDebug() << "MainWindow: Frame boyutu:" << frameData.frame.cols << "x" << frameData.frame.rows;

            // OpenCV Mat'i QPixmap'e çevir
            QPixmap pixmap = matToQPixmap(frameData.frame);

            if (pixmap.isNull()) {
                qDebug() << "MainWindow: Pixmap conversion başarısız!";
                return;
            }

            // Video display label'ında güvenli göster
            ui->label_VideoDisplay->setPixmap(
                pixmap.scaled(ui->label_VideoDisplay->size(),
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation)
                );

            // Frame bilgilerini güvenli güncelle
            if (frameInfoLabel) {
                frameInfoLabel->setText(QString("Frame: %1 (%.3fs)")
                                            .arg(frameData.frameId)
                                            .arg(frameData.timeStamp, 0, 'f', 3));
            }

            qDebug() << "MainWindow: Frame display başarılı";

        } catch (const cv::Exception& e) {
            qDebug() << "MainWindow: OpenCV exception in displayFrame:" << e.what();
        } catch (const std::exception& e) {
            qDebug() << "MainWindow: Exception in displayFrame:" << e.what();
        } catch (...) {
            qDebug() << "MainWindow: Unknown exception in displayFrame!";
        }
    }

    QPixmap MainWindow::matToQPixmap(const cv::Mat& frame)
    {
        try {
            qDebug() << "MainWindow: Mat to QPixmap conversion başlıyor";
            qDebug() << "Frame info - Size:" << frame.cols << "x" << frame.rows
                     << "Channels:" << frame.channels()
                     << "Depth:" << frame.depth()
                     << "Type:" << frame.type();

            // Frame geçerli mi kontrol et
            if (frame.empty()) {
                qDebug() << "MainWindow: Frame boş!";
                return QPixmap();
            }

            // Frame boyutu kontrol et
            if (frame.cols <= 0 || frame.rows <= 0) {
                qDebug() << "MainWindow: Frame boyutu geçersiz!";
                return QPixmap();
            }

            QImage qimg;

            if (frame.channels() == 3) {
                qDebug() << "MainWindow: 3 channel BGR frame işleniyor";

                // BGR'yi RGB ye çevir
                cv::Mat rgbFrame;
                cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

                qDebug() << "BGR to RGB conversion tamam";

                // Veri tipini kontrol et
                if (rgbFrame.type() != CV_8UC3) {
                    qDebug() << "Frame tipi CV_8UC3 değil:" << rgbFrame.type();
                    rgbFrame.convertTo(rgbFrame, CV_8UC3);
                }

                // QImage oluştur - güvenli yöntem
                qimg = QImage(rgbFrame.data,
                              rgbFrame.cols,
                              rgbFrame.rows,
                              rgbFrame.step[0],    // step kullan
                              QImage::Format_RGB888);

                // QImage i kopyala (data corruption dan kaçınmak için)
                qimg = qimg.copy();

            } else if (frame.channels() == 1) {
                qDebug() << "MainWindow: 1 channel grayscale frame işleniyor";

                // Grayscale frame
                cv::Mat grayFrame = frame;

                // Veri tipini kontrol et
                if (grayFrame.type() != CV_8UC1) {
                    qDebug() << "Frame tipi CV_8UC1 değil:" << grayFrame.type();
                    grayFrame.convertTo(grayFrame, CV_8UC1);
                }

                qimg = QImage(grayFrame.data,
                              grayFrame.cols,
                              grayFrame.rows,
                              grayFrame.step[0],
                              QImage::Format_Grayscale8);

                // QImage i kopyala
                qimg = qimg.copy();

            } else {
                qDebug() << "MainWindow: Desteklenmeyen channel sayısı:" << frame.channels();
                return QPixmap();
            }

            // QImage geçerli mi kontrol et
            if (qimg.isNull()) {
                qDebug() << "MainWindow: QImage oluşturulamadı!";
                return QPixmap();
            }

            // QPixmap'e çevir
            QPixmap pixmap = QPixmap::fromImage(qimg);

            if (pixmap.isNull()) {
                qDebug() << "QPixmap oluşturulamadı!";
                return QPixmap();
            }

            qDebug() << "QPixmap conversion başarılı!";
            return pixmap;

        } catch (const cv::Exception& e) {
            qDebug() << "MainWindow: OpenCV exception in matToQPixmap:" << e.what();
            return QPixmap();
        } catch (const std::exception& e) {
            qDebug() << "MainWindow: Exception in matToQPixmap:" << e.what();
            return QPixmap();
        } catch (...) {
            qDebug() << "MainWindow: Unknown exception in matToQPixmap!";
            return QPixmap();
        }
    }
QString MainWindow::selectVideoFile()
{
    QString filter = "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv *.flv *.webm);;All Files (*.*)";

    return QFileDialog::getOpenFileName(this, "Video Dosyası Seç", QString(), filter);
}

void MainWindow::addVideoToList(const QString& filePath)
{
    QFileInfo fileInfo(filePath);

    // Listeye ekle
    ui->listWidget_Videos->addItem(fileInfo.fileName());
    videoFilesList.append(filePath);

    // İlk video ise select butonunu aktif et
    if (videoFilesList.size() == 1) {
        ui->pushButton_selectVideo->setEnabled(true);
    }
}

void MainWindow::updateStatusBar(const QString& message)
{
    statusLabel->setText(message);
}

void MainWindow::showDebugInfo()
{
    QString info = "=== DEBUG INFO ===\n\n";

    // Video durumu
    if (isVideoLoaded) {
        VideoInfo videoInfo = videoController->getVideoInfo();
        // Video bilgileri
        info += QString("Video: %1, %2x%3, %4FPS, %5s, %6 frames, %7MB\n")
                    .arg(videoInfo.fileName)
                    .arg(videoInfo.width)
                    .arg(videoInfo.height)
                    .arg(videoInfo.fps, 0, 'f', 1)
                    .arg(videoInfo.duration, 0, 'f', 1)
                    .arg(videoInfo.totalFrames)
                    .arg(static_cast<double>(videoInfo.fileSize) / (1024.0 * 1024.0), 0, 'f', 1);
        info += QString("Playing: %1\n").arg(isPlaying ? "Yes" : "No");
        info += QString("Display FPS: %1\n").arg(currentDisplayFPS, 0, 'f', 1);
    } else {
        info += "Video: Not loaded\n";
    }

    // Queue durumu
    if (testFrameQueue) {
        info += QString("\nQueue: %1\n").arg(testFrameQueue->getInfo());
    }

    // System durumu
    info += QString("\nFrame Counter: %1\n").arg(frameCounter);

    QMessageBox::information(this, "Debug Info", info);
}

double MainWindow::calculateMemoryUsage()
{
    // Basit memory usage hesaplama
    return 1;

}

void MainWindow::updateVideoControls(VideoController::PlaybackState state)
{
    switch (state) {
    case VideoController::Playing:
        ui->pushButton_PlayPause->setText("Pause");
        isPlaying = true;
        break;
    case VideoController::Paused:
    case VideoController::Stopped:
        ui->pushButton_PlayPause->setText("Play");
        isPlaying = false;
        break;
    case VideoController::Finished:
        ui->pushButton_PlayPause->setText("Play");
        isPlaying = false;
        updateStatusBar("Video tamamlandı");
        break;
    case VideoController::Error:
        ui->pushButton_PlayPause->setText("Play");
        isPlaying = false;
        updateStatusBar("Video hatası");
        break;
    }
}

void MainWindow::updateVideoInfo(const VideoInfo& videoInfo)
{
    try {
        qDebug() << "MainWindow: Video info güncelleniyor UI'da";

        if (videoInfoLabel) {
            QString infoText = QString("Video: %1x%2, %3 FPS")
            .arg(videoInfo.width)
                .arg(videoInfo.height)
                .arg(videoInfo.fps, 0, 'f', 1);  // %.1f yerine 0, 'f', 1

            videoInfoLabel->setText(infoText);
            qDebug() << "MainWindow: Video info label güncellendi:" << infoText;
        }

    } catch (const std::exception& e) {
        qDebug() << "MainWindow: Exception in updateVideoInfo:" << e.what();
    } catch (...) {
        qDebug() << "MainWindow: Unknown exception in updateVideoInfo!";
    }
}

void MainWindow::updatePerformanceInfo(const PerformanceStats& stats)
{
    performanceLabel->setText(QString("FPS: %.1f").arg(stats.currentFPS));
}

void MainWindow::updateSeekSlider(double progress)
{
    int sliderValue = static_cast<int>(progress * 100);
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(sliderValue);
    ui->horizontalSlider->blockSignals(false);
}
