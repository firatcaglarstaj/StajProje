#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pythonScriptModel.h"
#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QtMultimediaWidgets/qvideowidget.h>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QDebug>
#include <QTimer>
#include<opencv2/opencv.hpp>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_AddVideo_clicked();

    void on_pushButton_selectVideo_clicked();

    void on_doubleSpinBox_valueChanged(double arg1);

    void on_horizontalSlider_sliderMoved(int position);

    void on_pushButton_ChooseModel_clicked();

    void on_pushButton_addModel_clicked();

    void on_comboBox_selectModel_currentIndexChanged(int index);

    void on_pushButton_ChooseObject_clicked();

    void on_pushButton_AddObject_clicked();

    void on_pushButton_Process_clicked();

    void on_pushButton_Results_clicked();

    void showNextFrame();

    void setupMotionDetectionModels();

    void pytonScript(const QString &modelName);
private:
    Ui::MainWindow *ui;

    // Video işleme için gerekli değişkenler
    cv::VideoCapture capture;
    cv::Mat currentFrame;
    QTimer *frameTimer;
    QString currentVideoPath;
    QStringList videoPaths;

    // Model yönetimi için gerekli değişkenler
    PythonScriptModel *pythonModel; // Python motion detection için
    QStringList modelPaths;
    QStringList modelNames;
    QString selectedModelPath;
    QString currentSelectedModelKey;
    QString currentSelectedModelName;
};
#endif // MAINWINDOW_H
