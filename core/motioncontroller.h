#ifndef MOTIONCONTROLLER_H
#define MOTIONCONTROLLER_H

#include <QObject>
#include <QThread>
#include "MoveDetect.hpp"
#include "ThreadQueue.h" // FrameQueue'nun olduğu dosya
#include "MoveDetect.hpp"        // MoveDetect kütüphanesi

// Sonuçları  taşımak için yeni bir struct
struct MotionResult {
    int frameId = -1;
    cv::Mat motionMask;
};

// Bu struct için bir kuyruk tipi tanımla
using MotionResultQueue = ThreadQueue<MotionResult>;

class MotionController : public QObject
{
    Q_OBJECT
public:
    explicit MotionController(FrameQueue* inputQueue, MotionResultQueue* outputQueue, QObject *parent = nullptr);
    ~MotionController();

public slots:
    void startProcessing();
    void stopProcessing();

signals:
    void resultReady(); // Yeni bir sonuç hazır olduğunda MainWindowa sinyal gönderecek

private:
    FrameQueue* m_inputQueue;
    MotionResultQueue* m_outputQueue;
    MoveDetect::Handler* m_motionHandler;
    bool m_isRunning;
};

#endif // MOTIONCONTROLLER_H
