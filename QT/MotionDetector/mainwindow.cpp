#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ai/yolocommunicator.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoController(nullptr)
    , testFrameQueue(nullptr)
    , yoloCommunicator(nullptr)
    , isVideoLoaded(false)
    , isYOLOConnected(false)
    , isYOLOEnabled(false)
    , isPlaying(false)
    , frameCounter(0)
    , currentDisplayFPS(0.0)
    , totalDetectionsCount(0)
{
    ui->setupUi(this);

    //initializeUI();
    setupVideoController();
    setupYOLOCommunicator();
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
    yoloStatusLabel = new QLabel("YOLO: Bağlı değil");

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
    ui->statusbar->addWidget(yoloStatusLabel);

    qDebug() << "MainWindow: Status bar setup tamamlandı";
}

void MainWindow::setupTimers()
{

    // UI güncelleme timer ı
    uiUpdateTimer = new QTimer(this);
    connect(uiUpdateTimer, &QTimer::timeout, this, &MainWindow::onUIUpdateTimer);
    uiUpdateTimer->start(100);

    // FPS hesaplama timerını başlat
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
    connect(yoloCommunicator, &YOLOCommunicator::detectionReceived,
            this, &MainWindow::onDetectionReceived);
    connect(yoloCommunicator, &YOLOCommunicator::connectionStatusChanged,
            this, &MainWindow::onYOLOConnectionChanged);
    connect(yoloCommunicator, &YOLOCommunicator::errorOccurred,
            this, &MainWindow::onYOLOError);

    qDebug() << "MainWindow: Signal-slot bağlantıları tamamlandı";
}

void MainWindow::initializeTestQueue()
{
    qDebug() << "MainWindow: Test frame queue initialize ediliyor...";

    // Test için frame queue oluştur
    //testFrameQueue = new FrameQueue(30); // 30 frame buffer

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
    //int totalFrames = videoController->getTotalFrames();
    //int targetFrame = (position * totalFrames) / 100;

    // Frame e git
    //videoController->seekToFrame(targetFrame);
}

void MainWindow::on_pushButton_SystemStatus_clicked()
{
    // showDebugInfo();
}

// VİDEO CONTROLLER SLOTLARI
void MainWindow::onFrameReady(const FrameData& frameData) {
    try {
        if (!frameData.isValid()) {
            return;
        }

        // YOLO'ya gönder - DÜZELTME: Her 5. frame (daha az yük)
        if (isYOLOConnected && isYOLOEnabled) {
            if (frameData.frameId % 5 == 0) {
                qDebug() << "Frame YOLO'ya gönderiliyor:" << frameData.frameId;
                yoloCommunicator->sendFrameForDetection(frameData);
            }
        }

        // Frame i display et
        displayFrame(frameData);

        /*  Test queue'ya ekle
        if (testFrameQueue) {
            testFrameQueue->push(frameData);
        }*/

        frameCounter++;

        // Cache temizliği
        if (frameCounter % 50 == 0) {
            cleanupDetectionCache();
        }

    } catch (const std::exception& e) {
        qDebug() << "onFrameReady exception:" << e.what();
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

void MainWindow::onPlaybackStateChanged(PlaybackState state)
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

void MainWindow::displayFrame(const FrameData& frameData) {
    try {
        if (frameData.frame.empty()) {
            qDebug() << "Frame boş - skip";
            return;
        }

        cv::Mat displayMat = frameData.frame; // original frame
        DetectionResult detectionToShow;
        bool shouldShowDetection = false;

        // Detection kontrolü
        bool hasDetections = detectionResults.contains(frameData.frameId);

        //  Bu frame için direkt detection var mı?
        if (detectionResults.contains(frameData.frameId)) {
            detectionToShow = detectionResults[frameData.frameId];
            shouldShowDetection = true;
            qDebug() << "detection bulundu frame" << frameData.frameId;
        }
        //  Son detection ı kullanabilir miyiz?
        else if (lastValidDetection.isValid() && lastDetectionFrameId >= 0) {
            int frameDiff = frameData.frameId - lastDetectionFrameId;

            if (frameDiff >= 0 && frameDiff <= DETECTION_PERSISTENCE) {
                detectionToShow = lastValidDetection;
                shouldShowDetection = true;
                qDebug() << "Persistent detection kullanılıyor (age:" << frameDiff << ")";
            } else {
                qDebug() << " Detection çok eski (age:" << frameDiff << ")";
            }
        }

        // Detection ı çiz
        if (shouldShowDetection) {
            try {
                displayMat = frameData.frame.clone();
                drawDetections(displayMat, detectionToShow);
                qDebug()  << detectionToShow.detections.size() << "detection çizildi";
            } catch (const cv::Exception& e) {
                qDebug() << " Detection çizim hatası:" << e.what();
                displayMat = frameData.frame;
            }
        }
        // QPixmap conversion ve display
        QPixmap pixmap = matToQPixmap(displayMat);

        if (!pixmap.isNull()) {
            ui->label_VideoDisplay->setPixmap(
                pixmap.scaled(ui->label_VideoDisplay->size(),
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation)
                );
        } else {
            qDebug() << "QPixmap conversion başarısız!";
        }

        // Frame info güncelle
        updateFrameInfo(frameData, shouldShowDetection, detectionToShow);

    } catch (const cv::Exception& e) {
        qDebug() << "displayFrame OpenCV exception:" << e.what();
    } catch (const std::exception& e) {
        qDebug() << "displayFrame std exception:" << e.what();
    } catch (...) {
        qDebug() << "displayFrame unknown exception!";
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
                          rgbFrame.step[0],    // step
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

/* void MainWindow::showDebugInfo()
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
}*/

double MainWindow::calculateMemoryUsage()
{
    // Basit memory usage hesaplama
    return 1;

}

void MainWindow::updateVideoControls(PlaybackState state)
{
    switch (state) {
    case Playing:
        ui->pushButton_PlayPause->setText("Pause");
        isPlaying = true;
        break;
    case Paused:
    case Stopped:
        ui->pushButton_PlayPause->setText("Play");
        isPlaying = false;
        break;
    case Finished:
        ui->pushButton_PlayPause->setText("Play");
        isPlaying = false;
        updateStatusBar("Video tamamlandı");
        break;
    case Error:
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
                .arg(videoInfo.fps, 0, 'f', 1);

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

void MainWindow::setupYOLOCommunicator()
{
    yoloCommunicator = new YOLOCommunicator(this);
}
void MainWindow::on_pushButton_ChooseModel_clicked()
{
    qDebug() << "MainWindow: YOLO bağlantı butonu tıklandı";

    if (isYOLOConnected) {
        // Disconnect
        yoloCommunicator->disconnectFromYOLO();
        qDebug() << "MainWindow: YOLO bağlantısı kesiliyor...";
    } else {
        // Connect
        qDebug() << "MainWindow: YOLO'ya bağlanıyor...";
        bool success = yoloCommunicator->connectToYOLO();

        if (success) {
            qDebug() << "MainWindow: YOLO servisine bağlandı";
        } else {

            qDebug() << "MainWindow: YOLO servisine bağlanılmadı";
        }
    }
}
void MainWindow::onDetectionReceived(const DetectionResult& result) {
    try {
        qDebug() << "=== DETECTION RECEIVED ===";
        qDebug() << "Detection frameId:" << result.frameId;
        qDebug() << "Detection count:" << result.detections.size();

        // Detection ı cache e sakla
        detectionResults[result.frameId] = result;

        // Son geçerli detection ı güncelle
        lastValidDetection = result;
        lastDetectionFrameId = result.frameId;

        qDebug() << "Son detection güncellendi";

        updateDetectionStats();

    } catch (const std::exception& e) {
        qDebug() << "Detection handling hatası:" << e.what();
    }
}

void MainWindow::onYOLOConnectionChanged(bool connected)
{
    qDebug() << "MainWindow: YOLO bağlantı durumu:" << (connected ? "Bağlı" : "Bağlı değil");

    isYOLOConnected = connected;
    isYOLOEnabled = connected;

    // UI güncelle
    updateYOLOStatus();

    // Button text güncelle
    if (ui->pushButton_ChooseModel) {
        ui->pushButton_ChooseModel->setText(connected ? "YOLO Disconnect" : "YOLO Connect");
    }
}

void MainWindow::onYOLOError(const QString& error)
{
    qDebug() << "MainWindow: YOLO hatası:" << error;

    // Status güncelle
    updateYOLOStatus();
}



void MainWindow::drawDetections(cv::Mat& frame, const DetectionResult& result) {
    try {
        for (const Detection& detection : result.detections) {
            if (!detection.isValid()) {
                continue;
            }

            // Confidence e göre renk belirleme
            cv::Scalar boxColor;
            if (detection.confidence > 0.8) {
                boxColor = cv::Scalar(0, 255, 0);      // Yüksek güven: Yeşil
            } else if (detection.confidence > 0.5) {
                boxColor = cv::Scalar(0, 255, 255);    // Orta güven: Sarı
            } else {
                boxColor = cv::Scalar(0, 165, 255);    // Düşük güven: Turuncu
            }

            // Kalınlık confidence'e göre
            int thickness = detection.confidence > 0.7 ? 3 : 2;

            // Bounding box çiz
            cv::rectangle(frame, detection.bbox, boxColor, thickness);

            // Label hazırla
            QString labelText = QString("%1 %2%")
                                    .arg(detection.className)
                                    .arg(static_cast<int>(detection.confidence * 100));

            // Text background
            cv::Size textSize = cv::getTextSize(labelText.toStdString(),
                                                cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, nullptr);

            cv::Point labelPos(detection.bbox.x, detection.bbox.y - 8);
            cv::Rect labelRect(labelPos.x, labelPos.y - textSize.height - 8,
                               textSize.width + 16, textSize.height + 16);

            // Semi-transparent background
            cv::Mat overlay;
            frame.copyTo(overlay);
            cv::rectangle(overlay, labelRect, cv::Scalar(0, 0, 0), -1);
            cv::addWeighted(frame, 0.7, overlay, 0.3, 0, frame);

            // Label text
            cv::putText(frame, labelText.toStdString(),
                        cv::Point(labelPos.x + 8, labelPos.y),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
        }

    } catch (const cv::Exception& e) {
        qDebug() << "drawDetections OpenCV exception:" << e.what();
    }
}

void MainWindow::updateYOLOStatus()
{
    if (yoloStatusLabel) {
        QString status;
        if (isYOLOConnected) {
            status = QString("YOLO:Connected (%1)")
                         .arg(yoloCommunicator->getStatus());
        } else {
            status = "YOLO: Disconnected";
        }
        yoloStatusLabel->setText(status);
    }
}

void MainWindow::updateDetectionStats()
{
    // Toplam detection sayısını hesapla
    totalDetectionsCount = 0;
    for (const auto& result : detectionResults) {
        totalDetectionsCount += result.detections.size();
    }

    qDebug() << "MainWindow: Toplam detection:" << totalDetectionsCount;
}

void MainWindow::updateFrameInfo(const FrameData& frameData, bool hasDetection, const DetectionResult& detection) {
    if (frameInfoLabel) {
        QString frameInfo = QString("Frame: %1 (%2s)")
        .arg(frameData.frameId)
            .arg(frameData.timeStamp, 0, 'f', 3);

        if (hasDetection) {
            int objectCount = detection.detections.size();
            int age = frameData.frameId - detection.frameId;

            if (age == 0) {
                frameInfo += QString(" - %1 objects [LIVE]").arg(objectCount);
            } else {
                frameInfo += QString(" - %1 objects [AGE:%2]").arg(objectCount).arg(age);
            }
        }

        frameInfoLabel->setText(frameInfo);
    }
}
void MainWindow::cleanupDetectionCache() {
    const int MAX_CACHE_SIZE = 100; // Son 100 frame i sakla

    if (detectionResults.size() > MAX_CACHE_SIZE) {
        // En eski detection'ları sil
        auto it = detectionResults.begin();
        int toRemove = detectionResults.size() - MAX_CACHE_SIZE;

        for (int i = 0; i < toRemove && it != detectionResults.end(); ++i) {
            it = detectionResults.erase(it);
        }

        qDebug() << "Detection cache temizlendi, yeni boyut:" << detectionResults.size();
    }
}
