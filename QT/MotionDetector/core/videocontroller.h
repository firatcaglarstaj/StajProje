#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include "core/FrameData.h"
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <opencv2/opencv.hpp>

// Video oynatma durumları
enum PlaybackState {
    Stopped,
    Playing,
    Paused,
    Finished,
    Error
};

class VideoController : public QObject
{
    Q_OBJECT

public:
    explicit VideoController(QObject *parent = nullptr);
    ~VideoController();

    bool openVideo(const QString &filePath);
    void closeVideo();

    void play();
    void pause();
    void stop();

    void seekToFrame(int frameNumber);
    void seekToTime(double timeInSeconds);

    void setPlaybackSpeed(double speed);
    double getPlaybackSpeed() const { return playbackSpeed; }

    FrameData getNextFrame();

    FrameData getLastFrame() const;
    FrameData getCurrentFrame();

    bool isVideoOpen() const { return videoCapture.isOpened(); }
    PlaybackState getState() const { return currentState; }

signals:
    void frameReady(const FrameData& frame);
    void videoOpened(const VideoInfo& info);
    void videoClosed();
    void playbackStateChanged(PlaybackState newState);
    void progressChanged(double progress);  // progress: 0.0 - 1.0

private slots:
    void onPlaybackTimer();
    void onPerformanceTimer();

private:
    QTimer* playbackTimer;
    QTimer* fpsCalculationTimer;

    cv::VideoCapture videoCapture;

    PlaybackState currentState;
    VideoInfo currentVideoInfo;
    PerformanceStats performanceStats;

    FrameData lastFrameData;

    int nextFrameId;
    double targetFPS;
    double playbackSpeed;

    QElapsedTimer performanceTimer;

    void handleError(const QString& errorMessage);
    void setState(PlaybackState newState);

    void startTimers();
    void stopTimers();

    bool updateVideoInfo();
    void resetVideoInfo();
    FrameData createFrameData(const cv::Mat& frame);
    void updatePerformanceStats();

    bool isValidVideoFile(const QString &filePath);

    double getProgress() const {
        if (currentVideoInfo.totalFrames <= 0) return 0.0;
        return static_cast<double>(currentVideoInfo.currentFrameNumber) / currentVideoInfo.totalFrames;
    }
};

#endif // VIDEOCONTROLLER_H
