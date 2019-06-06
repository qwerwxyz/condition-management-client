#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::exitbtnSlot()
{
    this->close();
    exit(0);
}

void Dialog::loginbtnSlot()
{
    if(ui->userline->text().isEmpty())
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����Ų���Ϊ��"));
    else{
        this->hide();
        emit LoginData(ui->userline->text());
    }

}
