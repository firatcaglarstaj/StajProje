/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton_AddVideo;
    QPushButton *pushButton_selectVideo;
    QComboBox *comboBox_selectModel;
    QLabel *label;
    QLabel *label_2;
    QComboBox *comboBox_selectObject;
    QLabel *label_3;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QPushButton *pushButton_addModel;
    QPushButton *pushButton_ChooseModel;
    QWidget *formLayoutWidget_2;
    QFormLayout *formLayout_2;
    QPushButton *pushButton_AddObject;
    QPushButton *pushButton_ChooseObject;
    QPushButton *pushButton_Process;
    QProgressBar *progressBar;
    QSlider *horizontalSlider;
    QPushButton *pushButton_Results;
    QDoubleSpinBox *doubleSpinBox_PlaybackSpeed;
    QLabel *label_4;
    QLabel *label_CurrentTime;
    QListWidget *listWidget_Videos;
    QLabel *label_VideoDisplay;
    QPushButton *pushButton_PlayPause;
    QPushButton *pushButton_SystemStatus;
    QPushButton *pushButton_MotionDetect;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1298, 600);
        QFont font;
        font.setPointSize(9);
        font.setBold(false);
        MainWindow->setFont(font);
        MainWindow->setAutoFillBackground(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayoutWidget = new QWidget(centralwidget);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(50, 300, 88, 71));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        pushButton_AddVideo = new QPushButton(verticalLayoutWidget);
        pushButton_AddVideo->setObjectName("pushButton_AddVideo");

        verticalLayout->addWidget(pushButton_AddVideo);

        pushButton_selectVideo = new QPushButton(verticalLayoutWidget);
        pushButton_selectVideo->setObjectName("pushButton_selectVideo");

        verticalLayout->addWidget(pushButton_selectVideo);

        comboBox_selectModel = new QComboBox(centralwidget);
        comboBox_selectModel->setObjectName("comboBox_selectModel");
        comboBox_selectModel->setGeometry(QRect(1120, 50, 151, 28));
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(1130, 30, 131, 20));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(70, 10, 61, 20));
        comboBox_selectObject = new QComboBox(centralwidget);
        comboBox_selectObject->setObjectName("comboBox_selectObject");
        comboBox_selectObject->setGeometry(QRect(1120, 260, 151, 28));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(1130, 240, 131, 20));
        formLayoutWidget = new QWidget(centralwidget);
        formLayoutWidget->setObjectName("formLayoutWidget");
        formLayoutWidget->setGeometry(QRect(1020, 40, 108, 78));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::ExpandingFieldsGrow);
        formLayout->setRowWrapPolicy(QFormLayout::RowWrapPolicy::WrapLongRows);
        formLayout->setContentsMargins(0, 0, 0, 0);
        pushButton_addModel = new QPushButton(formLayoutWidget);
        pushButton_addModel->setObjectName("pushButton_addModel");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, pushButton_addModel);

        pushButton_ChooseModel = new QPushButton(formLayoutWidget);
        pushButton_ChooseModel->setObjectName("pushButton_ChooseModel");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, pushButton_ChooseModel);

        formLayoutWidget_2 = new QWidget(centralwidget);
        formLayoutWidget_2->setObjectName("formLayoutWidget_2");
        formLayoutWidget_2->setGeometry(QRect(1020, 240, 108, 74));
        formLayout_2 = new QFormLayout(formLayoutWidget_2);
        formLayout_2->setObjectName("formLayout_2");
        formLayout_2->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
        formLayout_2->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::ExpandingFieldsGrow);
        formLayout_2->setRowWrapPolicy(QFormLayout::RowWrapPolicy::WrapLongRows);
        formLayout_2->setContentsMargins(0, 0, 0, 0);
        pushButton_AddObject = new QPushButton(formLayoutWidget_2);
        pushButton_AddObject->setObjectName("pushButton_AddObject");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::FieldRole, pushButton_AddObject);

        pushButton_ChooseObject = new QPushButton(formLayoutWidget_2);
        pushButton_ChooseObject->setObjectName("pushButton_ChooseObject");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, pushButton_ChooseObject);

        pushButton_Process = new QPushButton(centralwidget);
        pushButton_Process->setObjectName("pushButton_Process");
        pushButton_Process->setGeometry(QRect(1020, 370, 251, 29));
        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName("progressBar");
        progressBar->setGeometry(QRect(1070, 410, 191, 23));
        progressBar->setValue(24);
        horizontalSlider = new QSlider(centralwidget);
        horizontalSlider->setObjectName("horizontalSlider");
        horizontalSlider->setGeometry(QRect(350, 470, 591, 20));
        horizontalSlider->setOrientation(Qt::Orientation::Horizontal);
        pushButton_Results = new QPushButton(centralwidget);
        pushButton_Results->setObjectName("pushButton_Results");
        pushButton_Results->setGeometry(QRect(1070, 450, 151, 29));
        doubleSpinBox_PlaybackSpeed = new QDoubleSpinBox(centralwidget);
        doubleSpinBox_PlaybackSpeed->setObjectName("doubleSpinBox_PlaybackSpeed");
        doubleSpinBox_PlaybackSpeed->setGeometry(QRect(50, 480, 111, 29));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(10, 460, 201, 21));
        label_CurrentTime = new QLabel(centralwidget);
        label_CurrentTime->setObjectName("label_CurrentTime");
        label_CurrentTime->setGeometry(QRect(260, 470, 71, 20));
        QFont font1;
        font1.setPointSize(11);
        font1.setBold(true);
        label_CurrentTime->setFont(font1);
        listWidget_Videos = new QListWidget(centralwidget);
        listWidget_Videos->setObjectName("listWidget_Videos");
        listWidget_Videos->setGeometry(QRect(40, 40, 111, 251));
        label_VideoDisplay = new QLabel(centralwidget);
        label_VideoDisplay->setObjectName("label_VideoDisplay");
        label_VideoDisplay->setGeometry(QRect(170, 20, 841, 431));
        pushButton_PlayPause = new QPushButton(centralwidget);
        pushButton_PlayPause->setObjectName("pushButton_PlayPause");
        pushButton_PlayPause->setGeometry(QRect(580, 500, 90, 29));
        pushButton_SystemStatus = new QPushButton(centralwidget);
        pushButton_SystemStatus->setObjectName("pushButton_SystemStatus");
        pushButton_SystemStatus->setGeometry(QRect(1080, 490, 131, 29));
        pushButton_MotionDetect = new QPushButton(centralwidget);
        pushButton_MotionDetect->setObjectName("pushButton_MotionDetect");
        pushButton_MotionDetect->setGeometry(QRect(30, 390, 121, 29));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1298, 25));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton_AddVideo->setText(QCoreApplication::translate("MainWindow", "Video Ekle", nullptr));
        pushButton_selectVideo->setText(QCoreApplication::translate("MainWindow", "Video Se\303\247", nullptr));
        comboBox_selectModel->setCurrentText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "Model Se\303\247im Paneli", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Videolar", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Nesne Se\303\247im Paneli", nullptr));
        pushButton_addModel->setText(QCoreApplication::translate("MainWindow", "Model Ekle", nullptr));
        pushButton_ChooseModel->setText(QCoreApplication::translate("MainWindow", "Modeli Se\303\247", nullptr));
        pushButton_AddObject->setText(QCoreApplication::translate("MainWindow", "Nesne Ekle", nullptr));
        pushButton_ChooseObject->setText(QCoreApplication::translate("MainWindow", "Nesne Se\303\247", nullptr));
        pushButton_Process->setText(QCoreApplication::translate("MainWindow", "\304\260\305\236LE", nullptr));
        pushButton_Results->setText(QCoreApplication::translate("MainWindow", "Sonu\303\247lar\304\261 Getir", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Nesne Hareket H\304\261z\304\261 E\305\237i\304\237i (m/s)", nullptr));
        label_CurrentTime->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        label_VideoDisplay->setText(QString());
        pushButton_PlayPause->setText(QCoreApplication::translate("MainWindow", "Play/Pause", nullptr));
        pushButton_SystemStatus->setText(QCoreApplication::translate("MainWindow", "Sistem Durumu", nullptr));
        pushButton_MotionDetect->setText(QCoreApplication::translate("MainWindow", "motion detect", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
