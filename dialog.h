#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;

signals:
    void LoginData(QString);   //�����������ݵ��ź�

private slots:
    void exitbtnSlot();
    void loginbtnSlot();
};

#endif // DIALOG_H
