#ifndef SONUCEKRAN_H
#define SONUCEKRAN_H

#include <QWidget>

namespace Ui {
class SonucEkran;
}

class SonucEkran : public QWidget
{
    Q_OBJECT

public:
    explicit SonucEkran(QWidget *parent = nullptr);
    ~SonucEkran();

private:
    Ui::SonucEkran *ui;
};

#endif // SONUCEKRAN_H
