#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_AddVideo_clicked()
{
    //Tıklandığında Video Paneline Video olarak eklenmeli

}


void MainWindow::on_pushButton_selectVideo_clicked()
{
    // Tıklandığında video Ekranda Gösterilmek Üzere Seçilmeli
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

}


void MainWindow::on_pushButton_ChooseObject_clicked()
{
    //Karar mekanizması. Nesne Seçimi burdan yapılacak. Modele bu bilgi de yollanmalı.

}


void MainWindow::on_pushButton_AddObject_clicked()
{

}


void MainWindow::on_pushButton_Process_clicked()
{
    //Tüm metrikleri görüp , tıklanılınca işleyecek
}


void MainWindow::on_pushButton_Results_clicked()
{
    //videoda, model ile elde edilen loglar ve istenilen metrikler listelenecek.
}

