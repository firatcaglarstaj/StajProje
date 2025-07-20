#include "yolocommunicator.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QNetworkProxy>

YOLOCommunicator::YOLOCommunicator(QObject *parent)
    : QObject(parent)
    , socket(nullptr)
    , timeoutTimer(nullptr)
    , connected(false)
    , serverHost("localhost")
    , serverPort(8888)
    , lastSentFrameId(-1)
    , waitingForResponse(false)
    , framesSent(0)
    , resultsReceived(0)
    , errors(0)
{
    qDebug() << "YOLOCommunicator: Oluşturuluyor...";

    // TCP socket oluştur
    socket = new QTcpSocket(this);

    // Timeout timer oluştur
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->setInterval(5000); // 5 saniye timeout

    // Signal bağlantıları
    connect(socket, &QTcpSocket::connected, this, &YOLOCommunicator::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &YOLOCommunicator::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &YOLOCommunicator::onDataReceived);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &YOLOCommunicator::onSocketError);
    connect(timeoutTimer, &QTimer::timeout, this, &YOLOCommunicator::onResponseTimeout);

    qDebug() << "YOLOCommunicator: Hazır";
}

YOLOCommunicator::~YOLOCommunicator()
{
    qDebug() << "YOLOCommunicator: Temizleniyor...";
    disconnectFromYOLO();
    qDebug() << "YOLOCommunicator: Son istatistikler - Gönderilen:" << framesSent
             << "Alınan:" << resultsReceived << "Hata:" << errors;
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
void YOLOCommunicator::disconnectFromYOLO()
{
    qDebug() << "YOLOCommunicator: Bağlantı kesiliyor...";

    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }

    resetConnection();
}

bool YOLOCommunicator::isConnected() const
{
    return connected && socket && socket->state() == QTcpSocket::ConnectedState;
}

void YOLOCommunicator::sendFrameForDetection(const FrameData& frameData) {
    if (!isConnected() || waitingForResponse) {
        return;
    }

    try {
        qDebug() << "Frame gönderiliyor:" << frameData.frameId;

        // JSON mesaj hazırla
        QJsonObject message;
        message["type"] = "frame_request";
        message["timestamp"] = QDateTime::currentMSecsSinceEpoch() / 1000.0;

        QJsonObject payload;
        payload["frame_id"] = frameData.frameId;
        payload["width"] = frameData.frame.cols;
        payload["height"] = frameData.frame.rows;

        // image encoding VE detection parametreleri
        payload["data"] = frameToBase64(frameData.frame);

        // YOLO detection parametreleri ekle
        QJsonObject detectionParams;
        detectionParams["confidence_threshold"] = 0.3;  // Düşük threshold (daha fazla detection)
        detectionParams["nms_threshold"] = 0.4;         // Non-max suppression
        detectionParams["max_detections"] = 50;         // Maksimum detection sayısı
        detectionParams["target_classes"] = QJsonArray::fromStringList(
            QStringList() << "person" << "car" //<< "bicycle" << "car" << "motorcycle"
            // << "bus" << "truck" << "backpack" << "handbag"
            ); // Sadece ilgili class ları detect et. Bu ilerde sadece istenen obje kontrolü için kullanılabilir

        payload["detection_params"] = detectionParams;
        message["payload"] = payload;

        // Gönder
        sendMessage(message);

        // State güncelle
        lastSentFrameId = frameData.frameId;
        waitingForResponse = true;
        framesSent++;
        timeoutTimer->start();

    } catch (const std::exception& e) {
        handleError(QString("Frame gönderme hatası: %1").arg(e.what()));
    }
}

void YOLOCommunicator::onConnected()
{
    qDebug() << "YOLOCommunicator: Python YOLO'ya bağlandı!";
    connected = true;
    emit connectionStatusChanged(true);
}

void YOLOCommunicator::onDisconnected()
{
    qDebug() << "YOLOCommunicator: Python YOLO bağlantısı kesildi";
    resetConnection();
    emit connectionStatusChanged(false);
}

void YOLOCommunicator::onDataReceived()
{
    try {
        qDebug() << "YOLOCommunicator: Python'dan veri alındı";

        // JSON mesajı al
        QJsonObject response = receiveMessage();

        if (response.isEmpty()) {
            qDebug() << "YOLOCommunicator: Boş mesaj alındı";
            return;
        }

        QString messageType = response["type"].toString();

        if (messageType == "detection_result") {
            // Detection sonucunu parse et
            DetectionResult result = parseDetectionResult(response["payload"].toObject());

            if (result.isValid()) {
                qDebug() << "YOLOCommunicator: Detection alındı:" << result.toString();

                // State güncelle
                waitingForResponse = false;
                resultsReceived++;
                timeoutTimer->stop();

                // Signal emit et
                emit detectionReceived(result);
            } else {
                handleError("Geçersiz detection sonucu");
            }
        } else {
            qDebug() << "YOLOCommunicator: Bilinmeyen mesaj tipi:" << messageType;
        }

    } catch (const std::exception& e) {
        handleError(QString("Veri alma hatası: %1").arg(e.what()));
    } catch (...) {
        handleError("Veri alma bilinmeyen hatası");
    }
}

void YOLOCommunicator::onSocketError()
{
    QString error = QString("Socket hatası: %1").arg(socket->errorString());
    qDebug() << "YOLOCommunicator:" << error;
    handleError(error);
}

void YOLOCommunicator::onResponseTimeout()
{
    qDebug() << "YOLOCommunicator: Python cevap timeout!";
    waitingForResponse = false;
    handleError("Python YOLO cevap vermiyor (timeout)");
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
    // İlk 4 byte = mesaj boyutu
    if (socket->bytesAvailable() < 4) {
        return QJsonObject();
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
    waitingForResponse = false;
    timeoutTimer->stop();
    emit errorOccurred(errorMessage);
}

void YOLOCommunicator::resetConnection()
{
    connected = false;
    waitingForResponse = false;
    lastSentFrameId = -1;
    if (timeoutTimer) {
        timeoutTimer->stop();
    }
}
