#include <queue>
#include <mutex>
#include <condition_variable>
#include <QDebug>
#include "FrameData.h"

/**
Farklı thread'ler arasında güvenli veri paylaşımı için kullanılır.
Video frame lerini bir thread den diğerine geçirmek için
 */
template<typename T>
class ThreadQueue {
private:
    std::queue<T> queue;                    // Veri kuyrugu
    mutable std::mutex mutex;               // Thread güvenliği için
    std::condition_variable condition;       // Thread leri bekletmek için
    size_t maxSize;                         // Maximum kaç element olabilir

public:

    ThreadQueue(size_t maxSize = 50) : maxSize(maxSize) {
        qDebug() << "ThreadSafeQueue oluşturuldu, max size:" << maxSize;
    }

    ~ThreadQueue() {
        qDebug() << "ThreadSafeQueue silindi, son boyut:" << queue.size();
    }

    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex);

        // Eğer queue doldu ise eski elementleri at
        while (queue.size() >= maxSize) {
            queue.pop();
            qDebug() << "Queue dolu, eski element silindi";
        }

        // Yeni elementi ekle
        queue.push(item);

        // Bekleyen thread'leri uyandır
        condition.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex);

        // Queue boş değil olana kadar bekle
        condition.wait(lock, [this] { return !queue.empty(); });

        // Element'i çıkar
        T result = queue.front();
        queue.pop();

        return result;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }


    size_t size() const { //Queue'da kaç element var?
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }


    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        while (!queue.empty()) {
            queue.pop();
        }
        qDebug() << "Queue temizlendi";
    }

    QString getInfo() const {
        std::lock_guard<std::mutex> lock(mutex);
        return QString("Queue[Size:%1/%2, Empty:%3]")
            .arg(queue.size())
            .arg(maxSize)
            .arg(queue.empty() ? "Yes" : "No");
    }
};

// Video frame leri için kullanımı kolaylaştırmak için
using FrameQueue = ThreadQueue<FrameData>;
