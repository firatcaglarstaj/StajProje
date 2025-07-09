#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia>
#include <QtMultimediaWidgets/qvideowidget.h>
#include <QtMultimediaWidgets>
#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QtMultimedia>
#include <QtMultimediaWidgets/qvideowidget.h>
#include <QtMultimediaWidgets>
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

    void on_listView_videos_doubleClicked(const QModelIndex &index);

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

private:
    Ui::MainWindow *ui;

    // Video işleme için gerekli üye değişkenler
    cv::VideoCapture capture;
    cv::Mat currentFrame;
    QTimer *frameTimer;
    QString currentVideoPath;
    QStringList videoPaths;
};
#endif // MAINWINDOW_H
