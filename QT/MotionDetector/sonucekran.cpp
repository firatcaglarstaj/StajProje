#include "sonucekran.h"
#include "ui_sonucekran.h"

SonucEkran::SonucEkran(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SonucEkran)
{
    ui->setupUi(this);
}

SonucEkran::~SonucEkran()
{
    delete ui;
}
