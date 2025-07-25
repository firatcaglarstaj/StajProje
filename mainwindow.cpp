#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ai/yolocommunicator.h"
#include <qfileinfo.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    videoController(nullptr),
    yoloCommunicator(nullptr),
    videoThread(nullptr),
    yoloThread(nullptr),
    displayTimer(nullptr),
    uiUpdateTimer(nullptr),
    isPlaying(false),
    isVideoLoaded(false),
    isYOLOConnected(false),
    isYOLOEnabled(false),
    frameCounter(0),
    currentDisplayFPS(0.0),
    totalDetectionsCount(0),
    lastDetectionFrameId(-1),
    DETECTION_PERSISTENCE(15),
    isSliderDragging(false),
    isMotionDetectActive(false)

{
    ui->setupUi(this);
    setupStatusBar();
    setupTimers();
    setupThreads();

    displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, &MainWindow::onDisplayTimer);

    uiUpdateTimer = new QTimer(this);
    connect(uiUpdateTimer, &QTimer::timeout, this, &MainWindow::onUIUpdateTimer);
    uiUpdateTimer->start(100);

    motionHandler = new MoveDetect::Handler();
    motionHandler->mask_enabled = true;
    motionWindow = new MotionWindow();
    motionHandler->psnr_threshold = 5.0;
    motionHandler->contours_size = 5;
    motionHandler->contours_enabled = true;

}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow: Temizlik başlıyor...";

    // Tüm worker ların döngülerini durdurması için sinyal gönder
    if (videoController) {
        QMetaObject::invokeMethod(videoController, "stopProcessing", Qt::QueuedConnection);
    }
    if (yoloCommunicator) {
        QMetaObject::invokeMethod(yoloCommunicator, "stopProcessing", Qt::QueuedConnection);
    }
    if (motionController) {
        QMetaObject::invokeMethod(motionController, "stopProcessing", Qt::QueuedConnection);
    }

    // Kuyrukları temizleyerek bekleyen threadleri serbest bırak
    displayQueue.clear();
    detectionQueue.clear();
    motionQueue.clear();
    motionResultQueue.clear();


    // Threadleri güvenli bir şekilde kapat ve bitmelerini bekle
    if (videoThread && videoThread->isRunning()) {
        videoThread->quit();
        videoThread->wait(3000);
    }
    if (yoloThread && yoloThread->isRunning()) {
        yoloThread->quit();
        yoloThread->wait(3000);
    }
    if (motionThread && motionThread->isRunning()) {
        motionThread->quit();
        motionThread->wait(3000);
    }

    delete ui;
    qDebug() << "MainWindow: Temizlik tamamlandı.";
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    cleanupThreads();

    if (videoThread && videoThread->isRunning()) {
        videoThread->quit();
        if (!videoThread->wait(3000)) {
            qDebug() << "Video thread timeout - zorla sonlandırılıyor";
            videoThread->terminate();
            videoThread->wait(1000);
        }
    }

    if (yoloThread && yoloThread->isRunning()) {
        yoloThread->quit();
        if (!yoloThread->wait(3000)) {
            qDebug() << "YOLO thread timeout - zorla sonlandırılıyor";
            yoloThread->terminate();
            yoloThread->wait(1000);
        }
    }

    event->accept();
    qDebug() << "MainWindow: Close event tamamlandı";
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
void MainWindow::setupThreads() {
    qDebug() << "MainWindow: Thread'ler kuruluyor...";

    try {
        // Thread nesnelerini oluştur
        videoThread = new QThread(this);
        videoThread->setObjectName("VideoThread");
        yoloThread = new QThread(this);
        yoloThread->setObjectName("YOLOThread");
        // YENİ MOTION THREAD KURULUMU
        motionThread = new QThread(this);
        motionThread->setObjectName("MotionThread");
        motionController = new MotionController(&motionQueue, &motionResultQueue);
        motionController->moveToThread(motionThread);


        // Worker nesnelerini oluştur
        videoController = new VideoController(&displayQueue);
        yoloCommunicator = new YOLOCommunicator(&detectionQueue);

        // Worker'ları thread'lere taşı
        videoController->moveToThread(videoThread);
        yoloCommunicator->moveToThread(yoloThread);

        // Signal-slot bağlantıları
        setupSignalConnections();

        // Thread cleanup bağlantıları
        connect(videoThread, &QThread::finished, videoController, &QObject::deleteLater);
        connect(yoloThread, &QThread::finished, yoloCommunicator, &QObject::deleteLater);
        connect(motionController, &MotionController::resultReady, this, &MainWindow::onMotionResultReady, Qt::QueuedConnection);
        connect(motionThread, &QThread::finished, motionController, &QObject::deleteLater);

        // Thread'leri başlat
        videoThread->start();
        yoloThread->start();
        motionThread->start();

        // YOLO processing'i başlat (video açıldığında değil, hemen)
        QMetaObject::invokeMethod(yoloCommunicator, "startProcessing", Qt::QueuedConnection);

        qDebug() << "MainWindow: Thread'ler başarıyla kuruldu";

    } catch (const std::exception& e) {
        qDebug() << "Thread setup hatası:" << e.what();
        // Hata durumunda temizlik yap
        cleanupThreads();
    }
}

void MainWindow::setupSignalConnections() {
    // VideoController signals
    connect(videoController, &VideoController::videoOpened,
            this, &MainWindow::onVideoOpened, Qt::QueuedConnection);
    connect(videoController, &VideoController::videoFinished,
            this, &MainWindow::onVideoFinished, Qt::QueuedConnection);
    connect(videoController, &VideoController::progressChanged,
            this, &MainWindow::onProgressChanged, Qt::QueuedConnection);

    // seek signals
    // Slider başladığında video durdur
    connect(ui->horizontalSlider, &QSlider::sliderPressed, [this]() {
        isSliderDragging = true;

        // Video processing'i durdur
        displayTimer->stop();
        QMetaObject::invokeMethod(videoController, "stopProcessing", Qt::QueuedConnection);
    });

    // Slider bittiğinde video başlat
    connect(ui->horizontalSlider, &QSlider::sliderReleased, [this]() {
        isSliderDragging = false;

        // Video'yu yeni pozisyondan başlat
        if (isPlaying) {
            displayTimer->start(33);
            QMetaObject::invokeMethod(videoController, "startProcessing", Qt::QueuedConnection);
        }
    });


    // YOLOCommunicator signals
    connect(yoloCommunicator, &YOLOCommunicator::detectionReceived,
            this, &MainWindow::onDetectionReceived, Qt::QueuedConnection);
    connect(yoloCommunicator, &YOLOCommunicator::connectionStatusChanged,
            this, &MainWindow::onYOLOConnectionChanged, Qt::QueuedConnection);
    connect(yoloCommunicator, &YOLOCommunicator::errorOccurred,
            this, &MainWindow::onYOLOError, Qt::QueuedConnection);

}

void MainWindow::cleanupThreads() {
    qDebug() << "MainWindow: Thread cleanup başlıyor...";

    if (videoController) {
        QMetaObject::invokeMethod(videoController, "stopProcessing", Qt::QueuedConnection);
    }

    if (yoloCommunicator) {
        QMetaObject::invokeMethod(yoloCommunicator, "stopProcessing", Qt::QueuedConnection);
    }

    // Queue'ları temizle
    displayQueue.clear();
    detectionQueue.clear();
}

void MainWindow::startVideoProcessing(const QString& videoPath) {
    // Önce durdur
    stopVideoProcessing();

    updateStatusBar("Video açılıyor...");

    // Direkt main thread'de aç
    if (videoController->openVideoDirectly(videoPath)) {
        isVideoLoaded = true;
        isPlaying = true;
        ui->pushButton_PlayPause->setText("Pause");
        displayTimer->start(33);

        // Sonra processing'i thread'de başlat
        QMetaObject::invokeMethod(videoController, "startProcessing", Qt::QueuedConnection);

        updateStatusBar("Video oynatılıyor");
    } else {
        updateStatusBar("Hata: Video açılamadı");
    }
}
void MainWindow::stopVideoProcessing() {
    if (displayTimer) displayTimer->stop();
    if (videoController) {
        QMetaObject::invokeMethod(videoController, "stopProcessing", Qt::QueuedConnection);
    }

    displayQueue.clear();
    detectionQueue.clear();

    isPlaying = false;
    isVideoLoaded = false;
    ui->pushButton_PlayPause->setText("Play");
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

void MainWindow::onDisplayTimer()
{
    if (displayQueue.empty()) {
        return;
    }

    FrameData frameData = displayQueue.pop();
    if (!frameData.isValid()) return;

    currentFrameData = frameData;
    frameCounter++;

    // 1. Ana görüntüyü her zaman göster
    displayFrame(frameData);

    // 2. Hareket tespiti aktifse, kareyi motionQueue'ya gönder
    if (isMotionDetectActive) {
        motionQueue.push(frameData);
    }

    // 3. YOLO analizi için kare gönder
    if (isYOLOEnabled && isYOLOConnected && (frameCounter % 3 == 0)) {
        detectionQueue.push(frameData);
    }
}

void MainWindow::onVideoFinished()
{
    QMetaObject::invokeMethod(this, [this](){
        updateStatusBar("Video tamamlandı");
        stopVideoProcessing();
    }, Qt::QueuedConnection);
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
    startVideoProcessing(selectedVideoPath);
    isVideoLoaded = true;
    currentVideoPath = selectedVideoPath;
}

void MainWindow::on_listWidget_Videos_itemDoubleClicked(QListWidgetItem *item)
{

    // Double click ile video seç
    on_pushButton_selectVideo_clicked();
}


void MainWindow::on_pushButton_PlayPause_clicked() {
    if (!isVideoLoaded) {
        updateStatusBar("Önce video seçin");
        return;
    }

    if (isPlaying) {
        // Pause
        displayTimer->stop();
        QMetaObject::invokeMethod(videoController, "stopProcessing", Qt::QueuedConnection);
        isPlaying = false;
        ui->pushButton_PlayPause->setText("Play");
        updateStatusBar("Video duraklatıldı");
    } else {
        // Play
        displayTimer->start(33);
        QMetaObject::invokeMethod(videoController, "startProcessing", Qt::QueuedConnection);
        isPlaying = true;
        ui->pushButton_PlayPause->setText("Pause");
        updateStatusBar("Video oynatılıyor");
    }
}



void MainWindow::on_doubleSpinBox_PlaybackSpeed_valueChanged(double value)
{
    if (value <= 0) return; // Sıfıra bölme hatasını önle

    const double baseInterval = 33.0;
    int newInterval = static_cast<int>(baseInterval / value);

    if (displayTimer) {
        displayTimer->setInterval(qMax(1, newInterval)); // Minimum 1ms
    }
}

void MainWindow::on_horizontalSlider_sliderMoved(int position) {
    if (!isVideoLoaded || !videoController) {
        return;
    }

    // Pozisyonu yüzdeye çevir
    double percentage = static_cast<double>(position);

    // VideoController'a seek komutunu gönder
    QMetaObject::invokeMethod(videoController, "seekToPercentage",
                              Qt::QueuedConnection,
                              Q_ARG(double, percentage));
}


void MainWindow::on_pushButton_SystemStatus_clicked()
{
    // showDebugInfo();
}

// VİDEO CONTROLLER SLOTLARI

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


void MainWindow::onProgressChanged(double progress) {
    if (!isSliderDragging) {
        updateSeekSlider(progress);
    }
}

////////////////////////////////

void MainWindow::onUIUpdateTimer()
{
    try {
        // FPS hesapla
        static int lastFrameCounter = 0;
        int frameDiff = frameCounter - lastFrameCounter;
        lastFrameCounter = frameCounter;

        currentDisplayFPS = frameDiff * 10.0;

        // Performance label'ı güvenli güncelle
        if (performanceLabel) {
            performanceLabel->setText(QString("FPS: %1").arg(currentDisplayFPS, 0, 'f', 1));
        }

        // Memory usage güncelle
        if (memoryUsageBar) {
            double memoryUsage = calculateMemoryUsage();
            memoryUsageBar->setValue(static_cast<int>(memoryUsage));
        }

        // Cache temizliği
        if (frameCounter % 500 == 0) { // Her 500 frame'de bir
            cleanupDetectionCache();
        }

    } catch (const std::exception& e) {
        qDebug() << "UI update hatası:" << e.what();
    }
}

void MainWindow::displayFrame(const FrameData& frameData) {
    try {
        if (frameData.frame.empty()) return;

        cv::Mat displayMat = frameData.frame;
        DetectionResult detectionToShow;
        bool shouldShowDetection = false;

        // Detection kontrolü
        if (detectionResults.contains(frameData.frameId)) {
            detectionToShow = detectionResults[frameData.frameId];
            shouldShowDetection = true;
        }
        else if (lastValidDetection.isValid() && lastDetectionFrameId >= 0) {
            int frameDiff = frameData.frameId - lastDetectionFrameId;
            if (frameDiff >= 0 && frameDiff <= DETECTION_PERSISTENCE) {
                detectionToShow = lastValidDetection;
                shouldShowDetection = true;
            }
        }

        // Detection çiz
        if (shouldShowDetection) {
            try {
                displayMat = frameData.frame.clone();
                drawDetections(displayMat, detectionToShow);
            } catch (const cv::Exception& e) {
                displayMat = frameData.frame;
            }
        }

        // Display - debug'sız
        QPixmap pixmap = matToQPixmap(displayMat);
        if (!pixmap.isNull()) {
            ui->label_VideoDisplay->setPixmap(
                pixmap.scaled(ui->label_VideoDisplay->size(),
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation)
                );
        }

        updateFrameInfo(frameData, shouldShowDetection, detectionToShow);

    } catch (...) {
        // Silent catch
    }
}

QPixmap MainWindow::matToQPixmap(const cv::Mat& frame) {
    try {
        if (frame.empty() || frame.cols <= 0 || frame.rows <= 0) {
            return QPixmap();
        }

        QImage qimg;

        if (frame.channels() == 3) {
            cv::Mat rgbFrame;
            cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

            if (rgbFrame.type() != CV_8UC3) {
                rgbFrame.convertTo(rgbFrame, CV_8UC3);
            }

            qimg = QImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                          rgbFrame.step[0], QImage::Format_RGB888);
            qimg = qimg.copy();

        } else if (frame.channels() == 1) {
            cv::Mat grayFrame = frame;
            if (grayFrame.type() != CV_8UC1) {
                grayFrame.convertTo(grayFrame, CV_8UC1);
            }
            qimg = QImage(grayFrame.data, grayFrame.cols, grayFrame.rows,
                          grayFrame.step[0], QImage::Format_Grayscale8);
            qimg = qimg.copy();
        } else {
            return QPixmap();
        }

        if (qimg.isNull()) return QPixmap();

        return QPixmap::fromImage(qimg);

    } catch (...) {
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

double MainWindow::calculateMemoryUsage()
{
    // Basit memory usage hesaplama
    return 1;

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


void MainWindow::on_pushButton_ChooseModel_clicked()
{
    isYOLOEnabled = !isYOLOEnabled; // Sadece bayrağı tersine çevir

    if (isYOLOEnabled) {
        qDebug() << "MainWindow: YOLO analizi AKTİF.";
        ui->pushButton_ChooseModel->setText("YOLO Analizi: Aktif");
        updateYOLOStatus();
    } else {
        qDebug() << "MainWindow: YOLO analizi PASİF.";
        ui->pushButton_ChooseModel->setText("YOLO Analizi: Pasif");
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

            int thickness = 3;

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
                frameInfo += QString(" - %1 drone [LIVE]").arg(objectCount);
            } else {
                frameInfo += QString(" - %1 drone [AGE:%2]").arg(objectCount).arg(age);
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


void MainWindow::updateSeekSlider(double progress) {
    if (isSliderDragging) return;

    int sliderValue = static_cast<int>(progress * 100);
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(sliderValue);
    ui->horizontalSlider->blockSignals(false);
}


void MainWindow::on_pushButton_MotionDetect_clicked()
{
    isMotionDetectActive = !isMotionDetectActive;

    if (isMotionDetectActive) {
        ui->pushButton_MotionDetect->setText("Hareket Tespiti: Aktif");
        // Worker'ı kendi thread inde başlat
        QMetaObject::invokeMethod(motionController, "startProcessing", Qt::QueuedConnection);
        motionWindow->show();
    } else {
        ui->pushButton_MotionDetect->setText("Hareket Tespiti: Kapalı");
        // eğer aktif değilse Worker ı kendi thread inde durdur
        QMetaObject::invokeMethod(motionController, "stopProcessing", Qt::QueuedConnection);
        motionQueue.clear(); // Bekleyen işleri temizle
        motionResultQueue.clear();
        motionWindow->hide();
    }
}

void MainWindow::onMotionResultReady()
{
    if (motionResultQueue.empty()) {
        return;
    }

    MotionResult result;

    while (!motionResultQueue.empty()) {
        result = motionResultQueue.pop();
    }

    if (isMotionDetectActive && motionWindow && !result.motionMask.empty()) {
        QPixmap pixmap = matToQPixmap(result.motionMask);
        motionWindow->updateFrame(pixmap);
    }
}

