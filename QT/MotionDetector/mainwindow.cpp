#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , frameTimer(nullptr)  // Başlangıçta null olarak ayarlandı yoksa uygulama crash oluyor
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{

    if (frameTimer) {
        frameTimer->stop();
        delete frameTimer;
    }
    if (capture.isOpened()) {
        capture.release();
    }
    delete ui;
}

void MainWindow::on_pushButton_AddVideo_clicked()
{
    //Tıklandığında Video Paneline Video olarak eklenmeli
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Video Dosyası Seç"),
                                                    QString(),
                                                    tr("Video dosyaları (*.mp4 *.avi *.mkv)"));

    // Dosya seçimi iptal edildiyse return et
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo info(filePath);
    QString fileName = info.fileName();
    ui->videoListWidget->addItem(fileName);
    videoPaths.append(filePath); // videoPath vektörüne atandı. İlerde Process başlatılırken bu listeden çekilecek.
}

void MainWindow::showNextFrame()
{
    if (!capture.isOpened()) return;

    capture >> currentFrame;
    if (currentFrame.empty()) {
        frameTimer->stop();
        return;
    }

    // Örnek olarak çalışıyor mu diye rastgele bounding box çizdirdim (sonra YOLO sonucuyla değiştirilecek)
    cv::rectangle(currentFrame, cv::Rect(50, 50, 100, 100), cv::Scalar(0,255,0), 2);

    // OpenCV Mat -> QImage -> QPixmap -> QLabel
    QImage qimg(currentFrame.data, currentFrame.cols, currentFrame.rows, currentFrame.step, QImage::Format_BGR888);

    // QLabelın üstünde video gösterildiği için geçerli olduğundan emin olmalıyım
    if (ui->videoLabel) {
        ui->videoLabel->setPixmap(QPixmap::fromImage(qimg).scaled(ui->videoLabel->size(), Qt::KeepAspectRatio));
    }
}

void MainWindow::on_pushButton_selectVideo_clicked()
{
    // Tıklandığında video Ekranda Gösterilmek Üzere Seçilmeli
    // Listedeki seçili videoyu al
    int selectedIndex = ui->videoListWidget->currentRow();
    if (selectedIndex < 0 || selectedIndex >= videoPaths.size()) {
        QMessageBox::warning(this, "Hata", "Lütfen bir video seçin!");
        return;
    }

    currentVideoPath = videoPaths[selectedIndex];

    // Önceki timerı durdur
    if (frameTimer) {
        frameTimer->stop();
    }

    // Önceki captureı kapat
    if (capture.isOpened()) {
        capture.release();
    }

    // Yeni video aç
    capture.open(currentVideoPath.toStdString());
    if (!capture.isOpened()) {
        QMessageBox::warning(this, "Hata", "Video açılamadı: " + currentVideoPath);
        return;
    }

    // Timerı oluştur (sadece bir kez)
    if (!frameTimer) {
        frameTimer = new QTimer(this);
        connect(frameTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);
    }

    frameTimer->start(30); // yaklaşık 30 ms'de bir frame (yaklaşık 30 FPS)
}

void MainWindow::on_listView_videos_doubleClicked(const QModelIndex &index)
{
    // Paneldeki videoya çift tıklandığında video seçilmeli

}

void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    //Nesne hız eşiği burdan ayarlanmalı ve seçilen model ile beraber İŞLE butonuna basınca bu eşiğe göre işlenmeli
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    //Video konum kontorlü buradan yapılmalı
}

void MainWindow::on_pushButton_ChooseModel_clicked()
{
    //Model Seçimi burdan yapılacak
}

void MainWindow::on_pushButton_addModel_clicked()
{
    //Bilgisayardan model import edilecek
}

void MainWindow::on_comboBox_selectModel_currentIndexChanged(int index)
{
    // Model seçim değişikliği
}

void MainWindow::on_pushButton_ChooseObject_clicked()
{
    //Karar mekanizması. Nesne Seçimi burdan yapılacak. Modele bu bilgi de yollanmalı.
}

void MainWindow::on_pushButton_AddObject_clicked()
{
    // Nesne ekleme
}

void MainWindow::on_pushButton_Process_clicked()
{
    //Tüm metrikleri görüp , tıklanılınca işleyecek
}

void MainWindow::on_pushButton_Results_clicked()
{
    //videoda, model ile elde edilen loglar ve istenilen metrikler listelenecek.
}
