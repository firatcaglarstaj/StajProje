#ifndef YOLOCOMMUNICATOR_H
#define YOLOCOMMUNICATOR_H

#include "ai/DetectionData.h"
#include "core/FrameData.h"
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonObject>
#include <opencv2/opencv.hpp>


class YOLOCommunicator : public QObject
{
    Q_OBJECT

public:
    explicit YOLOCommunicator(QObject *parent = nullptr);
    ~YOLOCommunicator();

    bool connectToYOLO(const QString& host = "localhost", int port = 8888);
    void disconnectFromYOLO();
    bool isConnected() const;

    void sendFrameForDetection(const FrameData& frameData);
    QString getStatus() const;

signals:
    void detectionReceived(const DetectionResult& result);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& errorMessage);

private slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived();
    void onSocketError();
    void onResponseTimeout();

private:
    QTcpSocket* socket;
    QTimer* timeoutTimer;
    bool connected;
    QString serverHost;
    int serverPort;

    int lastSentFrameId;
    bool waitingForResponse;

    int framesSent;
    int resultsReceived;
    int errors;

    void sendMessage(const QJsonObject& message);
    QJsonObject receiveMessage();
    QString frameToBase64(const cv::Mat& frame);
    DetectionResult parseDetectionResult(const QJsonObject& json);
    void handleError(const QString& errorMessage);
    void resetConnection();
};

#endif // YOLOCOMMUNICATOR_H
