#include "videocontroller.h"
#include <QFileInfo>
#include <QDir>
#include <QApplication>

VideoController::VideoController(QObject *parent)
    : QObject(parent)
    , currentState(Stopped)
    , nextFrameId(0)
    , targetFPS(30)
    , playbackSpeed(1.0)
    , playbackTimer(nullptr)
    , fpsCalculationTimer(nullptr)
{
     // Performance timer ını başlat
    performanceTimer.start();
    performanceStats.startTime = QDateTime::currentDateTime();

    playbackTimer = new QTimer(this);
    fpsCalculationTimer = new QTimer(this);

    // Timer connections
    connect(playbackTimer, &QTimer::timeout, this, &VideoController::onPlaybackTimer);
    connect(fpsCalculationTimer, &QTimer::timeout, this, &VideoController::onPerformanceTimer);

    // FPS calculation her saniye çalışsın
    fpsCalculationTimer->start(1000);
    // Video info yu sıfırla
    resetVideoInfo();
}
VideoController::~VideoController()
{
    // Video yu kapat
    closeVideo();

    // Timer ları temizle
    if (playbackTimer) {
        playbackTimer->stop();
    }
    if (fpsCalculationTimer) {
        fpsCalculationTimer->stop();
    }

    qDebug() << "Final durumlar:" << performanceStats.toString();
}

void VideoController::handleError(const QString& errorMessage)
{
    qDebug() << "VideoController: Error occurred:" << errorMessage;

    setState(Error);
}
bool VideoController::openVideo(const QString &filePath)
{
    if (isVideoOpen()){
        closeVideo();
    }

    if(!isValidVideoFile(filePath)){
        QString error = QString("Video Dosyası Yok: %1").arg(filePath);
        return false;
    }
    // OpenCV ile video yu aç
    qDebug() << "OpenCV ile videocapture açılıyor...";
    bool openResult = videoCapture.open(filePath.toStdString());
    if (!openResult) {
        QString error = QString("Video Dosyası Açılamadı: %1").arg(filePath);
        return false;
    }
    // Video bilgilerini güncelle
    currentVideoInfo.filePath = filePath;
    currentVideoInfo.fileName = QFileInfo(filePath).fileName();
    // Frame tracking i sıfırla
    nextFrameId = 0;
    // Performance stats ı sıfırla
    performanceStats = PerformanceStats();
    performanceStats.startTime = QDateTime::currentDateTime();
    performanceTimer.restart();
    // Video Pause state ile başlasın
    setState(Paused);
    // Signal emit ile ilgili sinyali tetikle
    emit videoOpened(currentVideoInfo);
    return true;
}

void VideoController::closeVideo()
{
    stopTimers();
    if (videoCapture.isOpened()){
        videoCapture.release();
    }
    resetVideoInfo();
    setState(Stopped);
    emit videoClosed();
}

void VideoController::play()
{
    if (!isVideoOpen()) {
        qDebug() << "Başlatılamadı";
        return;
    }
    double videoFPS = currentVideoInfo.fps;
    if (videoFPS <= 0) {
        videoFPS = 30.0; // Eğer fps çekilemezse yedek plan, 30 ile başlat default
        qDebug() << videoFPS << "- using 30 FPS";
    }

    double effectiveFPS = videoFPS * playbackSpeed; //Kullanıcı videoyu hızlandırmak veya yavaşlatmak isterse FPS çarpanı uyguladım
    double intervalMs = (1000.0 / effectiveFPS); // Her kareyi kaç milisaniyede göstereceğini hesapla
    intervalMs = qMax(1.0, intervalMs - 5.0);

    playbackTimer->start(intervalMs); //her X ms de yeni frame çek, işle, ekrana bas
    setState(Playing);
    int finalInterval = static_cast<int>(intervalMs);
    qDebug() << "VideoController: Video FPS:" << videoFPS
             << "Playback speed:" << playbackSpeed
             << "Effective FPS:" << effectiveFPS
             << "Timer interval:" << finalInterval << "ms";
    if (playbackTimer) {
        playbackTimer->start(finalInterval);
    }

    setState(Playing);
}

void VideoController::pause()
{
    playbackTimer->stop();
    setState(Paused);
}

void VideoController::stop()
{
    stopTimers();
    if (isVideoOpen()) {
        seekToFrame(0);
        setState(Stopped);
    }
}

FrameData VideoController::getNextFrame()
{
    try {
        qDebug() << "VideoController: getNextFrame çağrıldı";

        if (!isVideoOpen()) {
            qDebug() << "VideoController: Video açık değil!";
            return FrameData(); // Invalid frame
        }

        cv::Mat frame;
        bool success = videoCapture.read(frame);

        if (!success) {
            qDebug() << "VideoController: Frame okuma başarısız - video bitti";
            if (currentState == Playing) {
                setState(Finished);
                stopTimers();
            }
            return FrameData(); // Invalid frame
        }

        if (frame.empty()) {
            qDebug() << "VideoController: Frame boş";
            return FrameData();
        }

        qDebug() << "VideoController: Frame okundu - Size:" << frame.cols << "x" << frame.rows;

        // Video pozisyon bilgilerini güncelle
        if (!updateVideoInfo()) {
            qDebug() << "VideoController: Video info güncelleme başarısız";
        }

        // FrameData oluştur
        FrameData frameData = createFrameData(frame);

        if (!frameData.isValid()) {
            qDebug() << "VideoController: FrameData oluşturulamadı";
            return FrameData();
        }

        // Performance stats ı güncelle
        updatePerformanceStats();

        qDebug() << "VideoController: Frame başarıyla oluşturuldu:" << frameData.toString();

        return frameData;

    } catch (const cv::Exception& e) {
        qDebug() << "VideoController: OpenCV exception in getNextFrame:" << e.what();
        handleError(QString("OpenCV error: %1").arg(e.what()));
        return FrameData();
    } catch (const std::exception& e) {
        qDebug() << "VideoController: Exception in getNextFrame:" << e.what();
        handleError(QString("Error: %1").arg(e.what()));
        return FrameData();
    } catch (...) {
        qDebug() << "VideoController: Unknown exception in getNextFrame!";
        handleError("Unknown error in getNextFrame");
        return FrameData();
    }
}

FrameData VideoController::getLastFrame() const
{
    return lastFrameData;
}

void VideoController::seekToFrame(int frameNumber)
{
    frameNumber = qBound(0, frameNumber, currentVideoInfo.totalFrames - 1);  // Kullanıcının girebileceği Frame numarasını sınırla

    bool seekResult = videoCapture.set(cv::CAP_PROP_POS_FRAMES, frameNumber); // OpenCV ile pozisyonu ayarla

    if (seekResult) {
        // Video bilgilerini güncelle
        updateVideoInfo();

        qDebug() << "Arama başarılı. Zaman:" << currentVideoInfo.currentTime;


        emit progressChanged(getProgress()); // Progress signal ı emit et
    } else {
        qDebug() << "VideoController: Arama başarısız" << frameNumber;
    }
}

void VideoController::seekToTime(double timeInSeconds)
{

    int targetFrame = static_cast<int>(timeInSeconds * currentVideoInfo.fps); // Zamanı frame numarasına çevir
    seekToFrame(targetFrame);
}

void VideoController::setPlaybackSpeed(double speed)
{
    playbackSpeed = speed;

    if (currentState == Playing) {
        pause();
        play(); // Timer ı yeni hızla yeniden başlat
    }

}
    void VideoController::onPlaybackTimer()
    {
        // Bir sonraki frame i al
        FrameData frameData = getNextFrame();

        if (frameData.isValid()) {
            // Frame hazır signal ını emit et
            emit frameReady(frameData);

            // Progress güncellemesi
            emit progressChanged(getProgress());
        } else {
            // Video bitti veya hata oluştu
            qDebug() << "VideoController: Playback timer - daha fazla frame yok";
            pause();
        }
    }

    void VideoController::onPerformanceTimer()// @TODO
    {

    }

    bool VideoController::updateVideoInfo()
    {
        if (!videoCapture.isOpened()) {
            return false;
        }
        try {
            // OpenCV den video bilgilerini al
            currentVideoInfo.totalFrames = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
            currentVideoInfo.fps = videoCapture.get(cv::CAP_PROP_FPS);
            currentVideoInfo.width = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
            currentVideoInfo.height = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
            currentVideoInfo.currentFrameNumber = static_cast<int>(videoCapture.get(cv::CAP_PROP_POS_FRAMES));
            currentVideoInfo.currentTime = videoCapture.get(cv::CAP_PROP_POS_MSEC) / 1000.0;

            // Duration hesapla
            if (currentVideoInfo.fps > 0) {
                currentVideoInfo.duration = currentVideoInfo.totalFrames / currentVideoInfo.fps;
            }


            // Dosya boyutunu al
            QFileInfo fileInfo(currentVideoInfo.filePath);
            currentVideoInfo.fileSize = fileInfo.size();

            return true;

        } catch (const cv::Exception& e) {
            qDebug() << "VideoController: OpenCV exception in updateVideoInfo:" << e.what();
            return false;
        }
    }

    void VideoController::resetVideoInfo()
    {
        currentVideoInfo = VideoInfo();
        nextFrameId = 0;
    }

    FrameData VideoController::createFrameData(const cv::Mat& frame)
    {
        try {
            qDebug() << "VideoController: FrameData oluşturuluyor";

            FrameData frameData;
            frameData.frameId = nextFrameId++;
            frameData.timeStamp = currentVideoInfo.currentTime;
            frameData.frameNumber = currentVideoInfo.currentFrameNumber;
            frameData.frame = frame.clone();
            frameData.processed = false;

            qDebug() << "VideoController: FrameData oluşturuldu - ID:" << frameData.frameId;

            return frameData;

        } catch (const cv::Exception& e) {
            qDebug() << "VideoController: OpenCV exception in createFrameData:" << e.what();
            return FrameData();
        } catch (const std::exception& e) {
            qDebug() << "VideoController: Exception in createFrameData:" << e.what();
            return FrameData();
        } catch (...) {
            qDebug() << "VideoController: Unknown exception in createFrameData!";
            return FrameData();
        }
    }

    void VideoController::setState(PlaybackState newState)
    {
        if (currentState != newState) {
            PlaybackState oldState = currentState;
            currentState = newState;

            qDebug() << "VideoController: Video durumu" << oldState << "iken" << newState<< "oldu";

            // Signal emit et
            emit playbackStateChanged(newState);
        }
    }

    void VideoController::startTimers()
    {

    }

    void VideoController::stopTimers()
    {
        if (playbackTimer) {
            playbackTimer->stop();
        }

    }

    void VideoController::updatePerformanceStats()
    {

    }

    bool VideoController::isValidVideoFile(const QString &filePath)
    {
        QFileInfo fileInfo(filePath);

        if(!fileInfo.exists()) {
            qDebug() << "VideoController: Dosya yok:" << filePath;
            return false;
            }
        else return true;
    }




