#include "motionwindow.h"

MotionWindow::MotionWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Motion Detection Pre-process");
    setMinimumSize(400, 300);

    displayLabel = new QLabel(this);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setStyleSheet("background-color: black;");

    layout = new QVBoxLayout(this);
    layout->addWidget(displayLabel);
    setLayout(layout);
}

MotionWindow::~MotionWindow()
{
}

void MotionWindow::updateFrame(const QPixmap &pixmap)
{
    if (pixmap.isNull()) return;

    // Gelen pixmap'i label'da göster, pencere boyutuna sığdır
    displayLabel->setPixmap(pixmap.scaled(displayLabel->size(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
}
