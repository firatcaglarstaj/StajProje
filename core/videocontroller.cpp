#include "videocontroller.h"
#include <QFileInfo>
#include <QDebug>
#include <qthread.h>
#include <qcoreapplication.h>

VideoController::VideoController(FrameQueue* displayQueue, QObject *parent)
    : QObject(parent),
    displayQueue(displayQueue),
    nextFrameId(0),
    isRunning(false)
{
    qDebug() << "VideoController: Worker oluşturuldu.";
    resetVideoInfo();
}

VideoController::~VideoController()
{
    closeVideo();
    qDebug() << "VideoController: Worker silindi.";
}

bool VideoController::openVideo(const QString& filePath) {
    return openVideoDirectly(filePath);
}

void VideoController::closeVideo()
{
    stopProcessing();
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }
    resetVideoInfo();
}

void VideoController::stopProcessing()
{
    isRunning = false;
}

void VideoController::startProcessing() {
    if (!videoCapture.isOpened() || isRunning) {
        return;
    }

    qDebug() << "VideoController: Processing başlatılıyor...";
    isRunning = true;

    while (isRunning && videoCapture.isOpened()) {
        cv::Mat frame;
        if (!videoCapture.read(frame) || frame.empty()) {
            emit videoFinished();
            break;
        }

        updateVideoInfo();
        FrameData frameData = createFrameData(frame);

        if (displayQueue) {
            displayQueue->push(frameData);
        }

        // Progress emit et
        emit progressChanged(currentVideoInfo.getProgress());

        QThread::msleep(33);

        // Thread responsive tut
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }

    qDebug() << "VideoController: Processing durdu";
    isRunning = false;
}


// Yardımcı Fonksiyonlar

FrameData VideoController::createFrameData(const cv::Mat& frame)
{
    FrameData frameData;
    frameData.frameId = nextFrameId++;
    frameData.timeStamp = currentVideoInfo.currentTime;
    frameData.frameNumber = currentVideoInfo.currentFrameNumber;
    frameData.frame = frame.clone(); // Veri bütünlüğü için klonla
    return frameData;
}

bool VideoController::updateVideoInfo()
{
    if (!videoCapture.isOpened()) return false;
    currentVideoInfo.totalFrames = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
    currentVideoInfo.fps = videoCapture.get(cv::CAP_PROP_FPS);
    currentVideoInfo.width = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
    currentVideoInfo.height = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
    currentVideoInfo.currentFrameNumber = static_cast<int>(videoCapture.get(cv::CAP_PROP_POS_FRAMES));
    currentVideoInfo.currentTime = videoCapture.get(cv::CAP_PROP_POS_MSEC) / 1000.0;
    if (currentVideoInfo.fps > 0) {
        currentVideoInfo.duration = currentVideoInfo.totalFrames / currentVideoInfo.fps;
    }
    return true;
}

void VideoController::resetVideoInfo()
{
    currentVideoInfo = VideoInfo();
    nextFrameId = 0;
}
bool VideoController::openVideoDirectly(const QString& filePath) {
    // Mevcut video'yu kapat
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }

    // Basit açma
    bool success = videoCapture.open(filePath.toStdString());

    if (success && videoCapture.isOpened()) {
        // Video bilgilerini güncelle
        currentVideoInfo.filePath = filePath;
        currentVideoInfo.fileName = QFileInfo(filePath).fileName();
        currentVideoInfo.width = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
        currentVideoInfo.height = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
        currentVideoInfo.fps = videoCapture.get(cv::CAP_PROP_FPS);
        currentVideoInfo.totalFrames = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
        currentVideoInfo.duration = currentVideoInfo.totalFrames / currentVideoInfo.fps;

        nextFrameId = 0;

        // Signal emit et
        emit videoOpened(currentVideoInfo);
        return true;
    }

    return false;
}


bool VideoController::seekToPercentage(double percentage) {
    if (!videoCapture.isOpened()) return false;

    // Frame numarasını hesapla
    int totalFrames = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
    int targetFrame = static_cast<int>((percentage / 100.0) * totalFrames);

    // Seek yap
    bool success = videoCapture.set(cv::CAP_PROP_POS_FRAMES, targetFrame);

    if (success) {
        // Video info güncelle
        updateVideoInfo();

        // Display queue'yu temizle
        if (displayQueue) {
            displayQueue->clear();
        }
    }

    return success;
}
