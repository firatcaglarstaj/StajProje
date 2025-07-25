#include "motioncontroller.h"
#include <QDebug>

MotionController::MotionController(FrameQueue* inputQueue, MotionResultQueue* outputQueue, QObject *parent)
    : QObject(parent),
    m_inputQueue(inputQueue),
    m_outputQueue(outputQueue),
    m_isRunning(false)
{
    m_motionHandler = new MoveDetect::Handler();
    m_motionHandler->mask_enabled = true; // Maskenin oluşturulmasını aktif et
    qDebug() << "MotionController: Worker oluşturuldu.";
}

MotionController::~MotionController()
{
    delete m_motionHandler;
    qDebug() << "MotionController: Worker silindi.";
}

void MotionController::stopProcessing()
{
    m_isRunning = false;
}

void MotionController::startProcessing()
{
    m_isRunning = true;
    qDebug() << "Motion Thread: İşlem döngüsü başladı.";

    while (m_isRunning) {
        // Giriş kuyruğundan ham kareyi al
        FrameData frameData = m_inputQueue->pop();
        if (!m_isRunning || !frameData.isValid()) continue;

        // --- ÖN İŞLEME (PRE-PROCESSING) ADIMLARI ---
        cv::Mat grayFrame, blurredFrame;

        // 1. Kareyi gri tonlamaya çevir. Bu, renk değişimlerinden kaynaklanan gürültüyü azaltır.
        cv::cvtColor(frameData.frame, grayFrame, cv::COLOR_BGR2GRAY);

        // 2. Gürültüyü ve küçük detayları yumuşatmak için Gaussian Blur uygula.
        // Çekirdek boyutu (21, 21) ne kadar büyük olursa o kadar bulanıklaşır.
        // Bu değeri (15,15) veya (31,31) gibi değiştirerek test edebilirsiniz.
        cv::GaussianBlur(grayFrame, blurredFrame, cv::Size(21, 21), 0);
        // --- ÖN İŞLEME SONU ---


        // 3. Algoritmaya ham kare yerine bulanıklaştırılmış kareyi ver
        m_motionHandler->detect(blurredFrame);

        // Hareket tespit edildiyse...
        if (m_motionHandler->movement_detected) {
            // Hareket maskesini al
            cv::Mat motionMask = m_motionHandler->mask.clone();

            // Sonucu hazırla ve gönder
            MotionResult result;
            result.frameId = frameData.frameId;
            result.motionMask = motionMask;

            m_outputQueue->push(result);
            emit resultReady();
        }
    }
    qDebug() << "Motion Thread: İşlem döngüsü durdu.";
}
