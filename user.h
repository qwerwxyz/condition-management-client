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
    quint16 blocksize;  //用来存放数据的大小信息

    int roomid; //房间号
    int speed;  //设定风速,0123,无低中高
    int speed_now;  //当前风速,0123,无低中高
    double tem_now; //当前温度
    double tem_init;    //初始温度
    double tem_set; //设定温度
    int mode;   //0制冷，1制热
    int state = 0;  //0关机，1开机
    int wind_state; //0停风，1送风
    double temp;
    QTimer *timer = new QTimer(this);
    QJsonObject cache;

    void init(QJsonObject respond);    //空调初始化

signals:
    void SendData(QJsonObject info);

private slots:
    void Login(QString data);   //接收传递过来的数据的槽
    void newConnect();
    void readMessage();
    void sendMessage(QJsonObject info);
    void displayError(QAbstractSocket::SocketError);

    void tem_up();  //温度上升
    void tem_down();    //温度下降
    void speed_change();    //改变风速
    void state_change();  //开关
    void mode_change(); //模式改变
    void checkout();

    void refresh(); //刷新实时温度
    void monitor(); //定时发送当前温度
    void TemSend();

    void closeEvent(QCloseEvent * event);
};

#endif // USER_H
