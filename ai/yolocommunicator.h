#ifndef YOLOCOMMUNICATOR_H
#define YOLOCOMMUNICATOR_H

#include "ai/DetectionData.h"
#include "core/FrameData.h"
#include "core/ThreadQueue.h" // FrameQueue için
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <QThread>

class YOLOCommunicator : public QObject
{
    Q_OBJECT

public:
    explicit YOLOCommunicator(FrameQueue* detectionQueue, QObject *parent = nullptr);
    ~YOLOCommunicator();

    QString getStatus() const;

public slots:
    void startProcessing();
    void stopProcessing();


signals:
    void detectionReceived(const DetectionResult& result);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& errorMessage);

private slots:
    void onConnected();
    void onDisconnected();
    // onDataReceived ve onSocketError artık doğrudan döngü içinde yönetilecek

private:
    bool connectToYOLO(const QString& host = "localhost", int port = 8888);
    void disconnectFromYOLO();
    bool isConnected() const;

    void sendMessage(const QJsonObject& message);
    QJsonObject receiveMessage();
    QString frameToBase64(const cv::Mat& frame);
    DetectionResult parseDetectionResult(const QJsonObject& json);
    void handleError(const QString& errorMessage);

    QTcpSocket* socket;
    FrameQueue* detectionQueue;
    std::atomic<bool> isRunning;

    bool connected;
    QString serverHost;
    int serverPort;

    // İstatistikler
    int framesSent;
    int resultsReceived;
    int errors;
};
#endif // YOLOCOMMUNICATOR_H
