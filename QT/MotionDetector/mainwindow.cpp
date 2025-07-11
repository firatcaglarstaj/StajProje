#include <python3.12/Python.h>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "motiondetector.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , frameTimer(nullptr)  // Başlangıçta null olarak ayarlandı yoksa uygulama crash oluyor
    , pythonModel(nullptr)
{
    ui->setupUi(this);

    // config için Python model oluşturma
    pythonModel = new PythonScriptModel();

    // C++ Motion detector'ı oluştur (gerçek processing için)
    MotionDetector = new MotionDetector();

    // ComboBox'a motion detection modelleri ekleme
    setupMotionDetectionModels();

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

void MainWindow::showNextFrame() // Timer her tetiklendiğinde çalışır. OpenCV ile sonraki Frame i okur
{
    if (!capture.isOpened()) return;

    capture >> currentFrame;
    if (currentFrame.empty()) {
        frameTimer->stop();
        return;
    }

    // Örnek olarak çalışıyor mu diye rastgele bounding box çizdirdim (sonra YOLO sonucuyla değiştirilecek)
    cv::rectangle(currentFrame, cv::Rect(50, 50, 100, 100), cv::Scalar(0,255,0), 2);

    // OpenCV Mat formatını Qt nin Qimage formatına çevirme işlemi
    QImage qimg(currentFrame.data, currentFrame.cols, currentFrame.rows, currentFrame.step, QImage::Format_BGR888);

    // QLabelın üstünde video gösterildiği için geçerli olduğundan emin olmalıyım
    if (ui->videoLabel) {
        ui->videoLabel->setPixmap(QPixmap::fromImage(qimg).scaled(ui->videoLabel->size(), Qt::KeepAspectRatio)); //Sonuç videoLabel da gösterilir
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
    capture.open(currentVideoPath.toStdString()); //Video dosyasını okumak için capture
    if (!capture.isOpened()) {
        QMessageBox::warning(this, "Hata", "Video açılamadı: " + currentVideoPath);
        return;
    }

    // Timerı oluştur
    if (!frameTimer) {
        frameTimer = new QTimer(this); //Frame  gösterimi için zamanlayucu
        connect(frameTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);
    }

    frameTimer->start(30); // yaklaşık 30 ms de bir frame (yaklaşık 30 FPS)
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

    // Seçilen modeli al
    int selectedIndex = ui->comboBox_selectModel->currentIndex();

    if (selectedIndex <= 0) {
        QMessageBox::warning(this, "Hata", "Lütfen bir model seçin!");
        return;
    }

    // Seçilen model bilgilerini sakla
    currentSelectedModelKey = modelNames[selectedIndex];
    currentSelectedModelName = ui->comboBox_selectModel->currentText();

    // Kullanıcıya bilgi ver
    QMessageBox::information(this, "Model Seçildi",
                             QString("Model başarıyla seçildi: %1").arg(currentSelectedModelName));

    // Python scripti ile modeli çağır
    pytonScript(currentSelectedModelKey);

}

void MainWindow::on_pushButton_addModel_clicked()
{
    //Bilgisayardan model import edilecek
    QString modelPath = QFileDialog::getOpenFileName(this,
                                                     tr("Model Dosyası Seç"),
                                                     QString(),
                                                     tr("Model dosyaları (*.pt *.onnx *.weights *.pb *.tflite)"));
    if (modelPath.isEmpty()) {
        return;
    }
    // Dosya var mı kontrol etme işlemi
    QFileInfo modelInfo(modelPath);
    if (!modelInfo.exists()) {
        QMessageBox::warning(this, "Hata", "Seçilen model dosyası bulunamadı!");
        return;
    }

    // Model zaten ekli mi kontrol etme işlemi
    if (modelPaths.contains(modelPath)) {
        QMessageBox::information(this, "Bilgi", "Bu model zaten ekli!");
        return;
    }

    // Model ismini dosya adı ile alma
    QString modelName = modelInfo.baseName();
    QString displayName = QString("%1").arg(modelName);
    modelPaths.append(modelPath);
    modelNames.append(displayName);

    ui->comboBox_selectModel->addItem(displayName);

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
void MainWindow::setupMotionDetectionModels()
{
    // Hazır motion detection modelleri comboBoxa ekleme
    ui->comboBox_selectModel->clear();
    ui->comboBox_selectModel->addItem("Seçiniz..");

    // motion detection modelleri
    ui->comboBox_selectModel->addItem("Background Subtraction MOG2");
    ui->comboBox_selectModel->addItem("Background Subtraction GMM");
    ui->comboBox_selectModel->addItem("Frame Difference");
    ui->comboBox_selectModel->addItem("Optical Flow Lucas-Kanade");
    ui->comboBox_selectModel->addItem("Optical Flow Farneback");
    ui->comboBox_selectModel->addItem("YOLO Motion Detection");
    ui->comboBox_selectModel->addItem("Custom Deep Learning Model");

    // Model bilgilerini saklama
    modelNames.clear();
    modelNames.append("none");
    modelNames.append("mog2");
    modelNames.append("gmm");
    modelNames.append("frame_diff");
    modelNames.append("lucas_kanade");
    modelNames.append("farneback");
    modelNames.append("yolo_motion");
    modelNames.append("custom_dl");
}

// Python script fonksiyonu
void MainWindow::pytonScript(const QString &modelName) {
    // Python yorumlayıcısını başlat
    Py_Initialize();

    // Python script dizini
    QString scriptPath = QCoreApplication::applicationDirPath() + "/scripts";

    // Python kodunu çalıştır
    QString pythonCode = QString("import sys\n"
                                 "sys.path.append('%1')\n"
                                 "try:\n"
                                 "    import motion_detection_model\n"
                                 "    motion_detection_model.load_model('%2')\n"
                                 "    print('Model loaded: %2')\n"
                                 "except Exception as e:\n"
                                 "    print('Error:', e)\n")
                             .arg(scriptPath)
                             .arg(modelName);

    qDebug() << "Executing Python code:" << pythonCode;

    // Python kodunu çalıştır
    PyRun_SimpleString(pythonCode.toStdString().c_str());

    // Python yorumlayıcısını sonlandır
    Py_Finalize();

    qDebug() << "Python script executed for model:" << modelName;
}
