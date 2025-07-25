#include "yolocommunicator.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QNetworkProxy>

YOLOCommunicator::YOLOCommunicator(FrameQueue* detectionQueue, QObject *parent)
    : QObject(parent),
    socket(nullptr),
    detectionQueue(detectionQueue),
    isRunning(false),
    connected(false),
    serverHost("localhost"),
    serverPort(8888),
    framesSent(0),
    resultsReceived(0),
    errors(0)
{
    qDebug() << "YOLOCommunicator: Worker oluşturuldu.";
}

YOLOCommunicator::~YOLOCommunicator()
{
    if (socket) {
        delete socket;
    }
    qDebug() << "YOLOCommunicator: Worker silindi.";
}
void YOLOCommunicator::stopProcessing()
{
    isRunning = false;
    // Bekleyen bir pop() işlemini sonlandırmak için kuyruğu temizleyebiliriz.
    // Bu, MainWindow da thread'i durdururken yapılır.
}


void YOLOCommunicator::startProcessing()
{
    socket = new QTcpSocket();
    connect(socket, &QTcpSocket::connected, this, &YOLOCommunicator::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &YOLOCommunicator::onDisconnected);

    isRunning = true;
    qDebug() << "YOLO Thread: İşlem döngüsü başladı.";

    while (isRunning) {
        if (!isConnected()) {
            if (!connectToYOLO(serverHost, serverPort)) {
                qDebug() << "YOLO Thread: Bağlantı kurulamadı, 5 saniye sonra tekrar denenecek.";
                QThread::msleep(5000); // 5 saniye bekle
                continue; // Döngünün başına dön
            }
        }

        // Tespit kuyruğundan bir kare al (kuyruk boşsa burada bekleyecek)
        FrameData frameData = detectionQueue->pop();
        if (!isRunning) break;

        try {
            qDebug() << "YOLO Thread: Frame gönderiliyor:" << frameData.frameId;

            QJsonObject payload;
            payload["frame_id"] = frameData.frameId;
            payload["data"] = frameToBase64(frameData.frame);
            // Diğer parametreler eklenebilir...

            QJsonObject message;
            message["type"] = "frame_request";
            message["payload"] = payload;

            sendMessage(message);
            framesSent++;

            // Cevabı al (receiveMessage blocking bir yapıda olmalı)
            QJsonObject response = receiveMessage();
            if (response.isEmpty()) {
                // Bağlantı kopmuş olabilir
                handleError("Python servisinden boş cevap alındı, bağlantı kopmuş olabilir.");
                disconnectFromYOLO();
                continue;
            }

            if (response["type"].toString() == "detection_result") {
                DetectionResult result = parseDetectionResult(response["payload"].toObject());
                if (result.isValid()) {
                    resultsReceived++;
                    emit detectionReceived(result); // Sonucu Ana Thread'e sinyal ile gönder
                }
            }
        }
        catch (const std::exception& e) {
            handleError(QString("YOLO Thread hatası: %1").arg(e.what()));
            disconnectFromYOLO();
        }
    }
    disconnectFromYOLO();
    qDebug() << "YOLO Thread: İşlem döngüsü durdu.";
}

bool YOLOCommunicator::connectToYOLO(const QString& host, int port) {
    qDebug() << "YOLOCommunicator: Python YOLO'ya bağlanıyor..." << host << ":" << port;

    if (isConnected()) {
        return true;
    }

    serverHost = host;
    serverPort = port;
    socket->setProxy(QNetworkProxy::NoProxy);

    // Asenkron bağlantı başlat
    socket->connectToHost(host, port);

    // Gerçek bağlantı kontrolü
    bool success = socket->waitForConnected(3000);

    if (!success) {
        QString error = QString("Bağlantı başarısız: %1").arg(socket->errorString());
        qDebug() << "YOLOCommunicator:" << error;
        handleError(error);
    }

    return success;
}


bool YOLOCommunicator::isConnected() const
{
    return connected && socket && socket->state() == QTcpSocket::ConnectedState;
}

void YOLOCommunicator::onDisconnected() {
    connected = false;
    qDebug() << "YOLOCommunicator: Python YOLO bağlantısı kesildi.";
    emit connectionStatusChanged(false);
}

void YOLOCommunicator::sendMessage(const QJsonObject& message)
{
    if (!isConnected()) {
        throw std::runtime_error("Bağlantı yok");
    }

    // JSON'a çevir
    QJsonDocument doc(message);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // Boyut prefix + JSON gönder
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(jsonData.size());
    packet.append(jsonData);

    // Gönder
    qint64 written = socket->write(packet);
    if (written != packet.size()) {
        throw std::runtime_error("Tüm veri gönderilemedi");
    }

    socket->flush();
}

QJsonObject YOLOCommunicator::receiveMessage()
{
    if (!socket->waitForReadyRead(5000)) { // 5 saniye timeout
        throw std::runtime_error("Cevap alma zaman aşımına uğradı.");
    }

    QByteArray sizeData = socket->read(4);
    QDataStream sizeStream(sizeData);
    sizeStream.setByteOrder(QDataStream::BigEndian);
    quint32 messageSize;
    sizeStream >> messageSize;

    // JSON mesajının tamamı geldi mi?
    if (socket->bytesAvailable() < messageSize) {
        return QJsonObject();
    }

    // JSON'u oku
    QByteArray jsonData = socket->read(messageSize);
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    return doc.object();
}

QString YOLOCommunicator::frameToBase64(const cv::Mat& frame)
{
    // Frame i JPEG e encode et
    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95}; // Yüksek kalite
    cv::imencode(".jpg", frame, buffer, params);

    // Base64'e çevir
    QByteArray byteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    return byteArray.toBase64();
}

DetectionResult YOLOCommunicator::parseDetectionResult(const QJsonObject& json)
{
    DetectionResult result;

    try {
        result.frameId = json["frame_id"].toInt();
        result.processingTimeMs = json["processing_time_ms"].toDouble();
        result.success = true;

        // Detections array'ini parse et
        QJsonArray detectionsArray = json["detections"].toArray();

        for (const auto& detectionValue : detectionsArray) {
            QJsonObject detectionObj = detectionValue.toObject();

            Detection detection;
            detection.classId = detectionObj["class_id"].toInt();
            detection.className = detectionObj["class_name"].toString();
            detection.confidence = detectionObj["confidence"].toDouble();

            // Bounding box parse et
            QJsonObject bboxObj = detectionObj["bbox"].toObject();
            int x1 = bboxObj["x1"].toInt();
            int y1 = bboxObj["y1"].toInt();
            int x2 = bboxObj["x2"].toInt();
            int y2 = bboxObj["y2"].toInt();

            detection.bbox = cv::Rect(x1, y1, x2 - x1, y2 - y1);

            if (detection.isValid()) {
                result.detections.append(detection);
            }
        }

    } catch (const std::exception& e) {
        qDebug() << "YOLOCommunicator: Parse hatası:" << e.what();
        result.success = false;
    }

    return result;
}

QString YOLOCommunicator::getStatus() const
{
    return QString("YOLO[Connected:%1, Sent:%2, Received:%3, Errors:%4]")
    .arg(isConnected() ? "Yes" : "No")
        .arg(framesSent)
        .arg(resultsReceived)
        .arg(errors);
}

void YOLOCommunicator::handleError(const QString& errorMessage)
{
    qDebug() << "YOLOCommunicator: Hata:" << errorMessage;
    errors++;
    emit errorOccurred(errorMessage);
}

void YOLOCommunicator::disconnectFromYOLO() {
    if (socket && socket->isOpen()) {
        socket->disconnectFromHost();
        // Beklemeli disconnect, thread'in blocklanmaması için isteğe bağlı
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
    }
    // Zaten onDisconnected sinyali connected bayrağını ve durumu yönetecek.
}

void YOLOCommunicator::onConnected() {
    connected = true;
    qDebug() << "YOLOCommunicator: Python YOLO'ya bağlandı!";
    emit connectionStatusChanged(true);
}
