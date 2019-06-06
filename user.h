#ifndef USER_H
#define USER_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCloseEvent>

class QTcpSocket;

namespace Ui {
class User;
}

class User : public QMainWindow
{
    Q_OBJECT

public:
    explicit User(QWidget *parent = 0);
    ~User();

private:
    Ui::User *ui;
    QTcpSocket *tcpSocket;
    QString message;
    quint16 blocksize;  //����������ݵĴ�С��Ϣ

    int roomid; //�����
    int speed;  //�趨����,0123,�޵��и�
    int speed_now;  //��ǰ����,0123,�޵��и�
    double tem_now; //��ǰ�¶�
    double tem_init;    //��ʼ�¶�
    double tem_set; //�趨�¶�
    int mode;   //0���䣬1����
    int state = 0;  //0�ػ���1����
    int wind_state; //0ͣ�磬1�ͷ�
    double temp;
    QTimer *timer = new QTimer(this);
    QJsonObject cache;

    void init(QJsonObject respond);    //�յ���ʼ��

signals:
    void SendData(QJsonObject info);

private slots:
    void Login(QString data);   //���մ��ݹ��������ݵĲ�
    void newConnect();
    void readMessage();
    void sendMessage(QJsonObject info);
    void displayError(QAbstractSocket::SocketError);

    void tem_up();  //�¶�����
    void tem_down();    //�¶��½�
    void speed_change();    //�ı����
    void state_change();  //����
    void mode_change(); //ģʽ�ı�
    void checkout();

    void refresh(); //ˢ��ʵʱ�¶�
    void monitor(); //��ʱ���͵�ǰ�¶�
    void TemSend();

    void closeEvent(QCloseEvent * event);
};

#endif // USER_H
