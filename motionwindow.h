#ifndef MOTIONWINDOW_H
#define MOTIONWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

class MotionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MotionWindow(QWidget *parent = nullptr);
    ~MotionWindow();

public slots:
    void updateFrame(const QPixmap &pixmap); // Dışarıdan yeni kare almak için slot

private:
    QLabel *displayLabel;
    QVBoxLayout *layout;
};

#endif // MOTIONWINDOW_H
