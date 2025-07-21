#ifndef DETECTIONDATA_H
#define DETECTIONDATA_H
#include <QString>
#include <QVector>
#include <opencv2/opencv.hpp>
/*
    Tek bir nesne tespiti
    Python YOLO'dan gelen sonuç
    */
    struct Detection {
    int classId = -1;               // YOLO class ID (0=person, 1=bicycle vs)
    QString className;              // Class adı ("person", "car", vs)
    double confidence = 0.0;        // Güven skoru (0.0 - 1.0)
    cv::Rect bbox;                  // Bounding box (x, y, width, height)
    // Geçerli mi kontrol et
    bool isValid() const {
        return classId >= 0 && confidence > 0.0 &&
               bbox.width > 0 && bbox.height > 0;
    }

    QString toString() const {
        return QString("Detection[%1, %2, (%3,%4,%5,%6)]")
        .arg(className)
            .arg(confidence, 0, 'f', 2)
            .arg(bbox.x).arg(bbox.y)
            .arg(bbox.width).arg(bbox.height);
    }
};
/*
    Bir frame'deki tüm tespitler
    */
    struct DetectionResult {
    int frameId = -1;                   // Hangi frame
    double processingTimeMs = 0.0;      // Python da işleme süresi
    QVector<Detection> detections;      // Bulunan nesneler
    bool success = false;               // İşlem başarılı mı
    // Geçerli mi kontrol et
    bool isValid() const {
        return frameId >= 0 && success;
    }
    QString toString() const {
        return QString("DetectionResult[Frame:%1, Objects:%2, Time:%3ms, Success:%4]")
        .arg(frameId)
            .arg(detections.size())
            .arg(processingTimeMs, 0, 'f', 1)
            .arg(success ? "Yes" : "No");
    }
};
#endif // DETECTIONDATA_H
