#include "user.h"
#include "ui_user.h"
#include "dialog.h"
#include <QtNetwork>

User::User(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::User)
{
    ui->setupUi(this);

    Dialog *dlg = new Dialog;
    //关联信号和槽函数
    connect(dlg, SIGNAL(LoginData(QString)), this, SLOT(Login(QString)));
    dlg->show();

    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &User::readMessage);
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(this,SIGNAL(SendData(QJsonObject)),this,SLOT(sendMessage(QJsonObject)));

    QTimer *second_timer = new QTimer(this);
    connect(second_timer,SIGNAL(timeout()),this,SLOT(refresh()));
    second_timer->start(1000);

    QTimer *minute_timer = new QTimer(this);
    connect(minute_timer,SIGNAL(timeout()),this,SLOT(monitor()));
    minute_timer->start(60000);

    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(TemSend()));

    ui->lcd_currenttemp->setSmallDecimalPoint(true);
    ui->lcd_goaltemp->setSmallDecimalPoint(true);
    }

User::~User()
{
    delete ui;
}

void User::Login(QString data)
{
    newConnect();
    QJsonObject rectJson;
    rectJson.insert("Action", "Login");
    rectJson.insert("roomID", data);
    roomid = data.toInt();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    //tem_now  = qrand()%11 + 20; //随机当前温度
    tem_now = 25;
    emit SendData(rectJson);
}

void User::newConnect()
{
    blocksize = 0;  // 初始化数据大小信息为0
    tcpSocket->abort(); // 取消已有的连接
    tcpSocket->connectToHost("10.28.191.237", 5555);    // 建立新连接
    tcpSocket->waitForConnected(-1);
}

void User::readMessage()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_9); //设置数据流的版本，客户端和服务器端使用的版本要相同
    //if(blocksize == 0){
        //if(tcpSocket->bytesAvailable()<(int)sizeof(quint16)) return;    //判断接收的数据是否大于两字节，也就是文件的大小信息所占的空间
        in >> blocksize;    //如果是则保存到blocksize变量中，否则直接返回，继续接收数据
    //}
    //if(tcpSocket->bytesAvailable()<blocksize) return;   //如果没有得到全部的数据，则返回，继续接收数据
    in >> message;  //将接收到的数据存放到变量中

    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toLocal8Bit().data());
    QJsonObject respond = jsonDoc.object();

    qDebug()<<"Recv:    "<<respond;

    if(respond.value("Action").toString()=="Login_S"){  //登陆成功
        this->show();
        ui->set_mode->setEnabled(false);
        ui->set_temp->setEnabled(false);
        ui->set_temp_2->setEnabled(false);
        ui->set_wind->setEnabled(false);
        ui->exit->setEnabled(false);
    }
    else if(respond.value("Action").toString()=="Login_F"){  //登陆失败
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("房间号有误"));

    }
    else if(respond.value("Action").toString()=="Turnon_S"){    //开机成功
        init(respond);

        ui->lcd_goaltemp->display(tem_set);
        ui->lcd_currenttemp->display(tem_now);
        ui->wind_speed->setText(QString::number(speed));
        state=1;
        if(mode == 0)
            ui->model_of_main->setText("COOL");
        else if(mode == 1)
            ui->model_of_main->setText("HOT");

        ui->set_mode->setEnabled(true);
        ui->set_temp->setEnabled(true);
        ui->set_temp_2->setEnabled(true);
        ui->set_wind->setEnabled(true);
        ui->exit->setEnabled(true);
    }
    else if(respond.value("Action").toString()=="Turnon_F"){    //开机失败
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("中央空调未开启"));
    }
    else if(respond.value("Action").toString()=="Turnoff_S"){   //关机
        ui->lcd_goaltemp->display("");
        ui->lcd_currenttemp->display("");
        ui->wind_speed->setText(NULL);
        state=0;
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        tem_now  = qrand()%11 + 20; //随机当前温度

        ui->set_mode->setEnabled(false);
        ui->set_temp->setEnabled(false);
        ui->set_temp_2->setEnabled(false);
        ui->set_wind->setEnabled(false);
        ui->exit->setEnabled(false);
    }
    else if(respond.value("Action").toString()=="Changemode_S"){    //切换模式
        if(mode == 0) mode = 1;
        else if(mode == 1) mode = 0;

        if(mode == 0)
            ui->model_of_main->setText("COOL");
        else if(mode == 1)
            ui->model_of_main->setText("HOT");
    }
    else if(respond.value("Action").toString()=="Changewind_S"){    //切换风速
        speed = respond.value("requiredwindspeed").toVariant().toInt();
        ui->wind_speed->setText(QString::number(speed));
        speed_now = speed;
        ui->wind_speed_2->setText(QString::number(speed_now));
    }
    else if(respond.value("Action").toString()=="Changetemp_S"){    //切换温度成功
        tem_set = respond.value("settem").toVariant().toInt();
        temp = tem_set;
        ui->lcd_goaltemp->display(tem_set);
    }
    else if(respond.value("Action").toString()=="Changetemp_F"){    //切换温度失败
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("温度超出范围"));
    }
    else if(respond.value("Action").toString()=="Sendtemp_S"){  //定时发送温度

    }
    else if(respond.value("Action").toString()=="Stopwind_S" || respond.value("Action").toString()=="STOP"){  //停止送风
        wind_state = 0;
        speed_now = 0;
        ui->wind_speed_2->setText(QString::number(speed_now));
    }
    else if(respond.value("Action").toString()=="Startwind_S" || respond.value("Action").toString()=="START"){ //请求送风
        wind_state = 1;
        speed_now = speed;
        ui->wind_speed_2->setText(QString::number(speed_now));
    }
    else if(respond.value("Action").toString()=="Checkout_S"){  //退房
        double cost = respond.value("money").toDouble();
        ui->cost->setText(QString::number(cost,'f',2));
        tcpSocket->disconnectFromHost();
    }
}

void User::sendMessage(QJsonObject info)
{
    QJsonDocument rectJsonDoc;
    rectJsonDoc.setObject(info);
    QString request(rectJsonDoc.toJson(QJsonDocument::Compact));
    qDebug()<<request;

    QByteArray block;   //用于暂存要发送的数据
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_6);    //设置数据流的版本，客户端和服务器端使用的版本要相同
    out << (quint16)0;  //预留两字节
    out << request;

    out.device()->seek(0);  //  跳转到数据块的开头
    out << (quint16)(block.size() - sizeof(quint16));   //填写大小信息

    qDebug()<<"Send:    "<<block;
    tcpSocket->write(block);
    tcpSocket->flush();
}

void User::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpSocket->errorString();
    QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("连接发生错误！请重新连接"));
    exit(0);
}

void User::tem_up() //温度上升
{
    if(timer->remainingTime()>0){
        timer->stop();
        timer->start(1000);
    }
    else
        timer->start(1000);

    temp = temp + 1;
    QJsonObject rectJson;
    rectJson.insert("Action", "Changetemp");
    rectJson.insert("roomID", roomid);
    rectJson.insert("requiredtemp", temp);

    cache = rectJson;
}

void User::tem_down()   //温度下降
{
    if(timer->remainingTime()>0){
        timer->stop();
        timer->start(1000);
    }
    else
        timer->start(1000);

    temp = temp - 1;
    QJsonObject rectJson;
    rectJson.insert("Action", "Changetemp");
    rectJson.insert("roomID", roomid);
    rectJson.insert("requiredtemp", temp);

    cache = rectJson;
}

void User::speed_change()   //更改风速
{
    QJsonObject rectJson;
    rectJson.insert("Action", "Changewind");
    rectJson.insert("roomID", roomid);

    if(speed_now==0){
        if(speed==1)
            speed=2;
        else if(speed==2)
            speed=3;
        else if(speed==3)
            speed=1;
        ui->wind_speed->setText(QString::number(speed));
    }
    else if(speed_now!=0){
        if(speed==1)
            rectJson.insert("requiredwindspeed", 2);
        else if(speed==2)
            rectJson.insert("requiredwindspeed", 3);
        else if(speed==3)
            rectJson.insert("requiredwindspeed", 1);

        emit SendData(rectJson);
    }
}

void User::state_change()   //开关
{
    QJsonObject rectJson;
    if(state==0){
        rectJson.insert("Action", "Turnon");
        rectJson.insert("roomID", roomid);
    }
    else if(state==1){
        rectJson.insert("Action", "Turnoff");
        rectJson.insert("roomID", roomid);
    }
    emit SendData(rectJson);
}

void User::mode_change()    //更改模式
{
    QJsonObject rectJson;
    rectJson.insert("Action", "Changemode");
    rectJson.insert("roomID", roomid);

    if(mode==0)
        rectJson.insert("mode", "hot");
    else if(mode==1)
        rectJson.insert("mode", "cold");

    emit SendData(rectJson);
}

void User::init(QJsonObject respond)   //空调初始化
{
    speed = respond.value("windspeed").toVariant().toInt(); //默认风速
    state = 0;  //默认关机
    if(respond.value("mode").toString()=="cold") //默认模式
        mode = 0;
    else if(respond.value("mode").toString()=="hot")
        mode = 1;
    tem_set = respond.value("starttemp").toVariant().toInt();    //默认温度
    temp = tem_set;
    tem_init = tem_now;
    wind_state = 0;
    speed_now = 0;

    int ww;
    if(mode==0 && tem_now<=tem_set)    //制冷且当前小于设定
        ww = 0;
    else if(mode==1 && tem_now>=tem_set)   //制热且当前大于设定
        ww = 0;
    else if(mode==0 && tem_now>tem_set)    //制冷且当前大于设定
        ww = 1;
    else if(mode==1 && tem_now<tem_set)    //制热且当前小于设定
        ww = 1;

    if(ww==1){
        QJsonObject rectJson;
        rectJson.insert("Action", "Startwind");
        rectJson.insert("roomID", roomid);
        rectJson.insert("requiredwindspeed", speed);

        emit SendData(rectJson);
    }
}

void User::refresh()    //刷新实时温度
{
    if(state == 1){ //开机状态
        if(mode==0 && speed_now==0){    //制冷且当前小于设定
            //if(tem_now<tem_init)
                tem_now = tem_now + 0.5/60;
        }
        else if(mode==1 && speed_now==0){   //制热且当前大于设定
            //if(tem_now>tem_init)
                tem_now = tem_now - 0.5/60;
        }
        else if(mode==0 && wind_state==1){    //制冷且当前小于设定
            if(speed_now == 1)
                tem_now = tem_now - 0.4/60;
            else if(speed_now == 2)
                tem_now = tem_now - 0.5/60;
            else
                tem_now = tem_now - 0.6/60;
        }
        else if(mode==1 && wind_state==1){    //制热且当前小于设定
            if(speed_now == 1)
                tem_now = tem_now + 0.4/60;
            else if(speed_now == 2)
                tem_now = tem_now + 0.5/60;
            else
                tem_now = tem_now + 0.6/60;
        }

        if(qAbs(tem_now - tem_set)<=0.01 && wind_state==1){  //温度到达目标且正则送风，要求停止
            //wind_state = 0;
            QJsonObject rectJson;
            rectJson.insert("Action", "Stopwind");
            rectJson.insert("roomID", roomid);

            emit SendData(rectJson);
        }
        if(qAbs(tem_now - tem_set)>=1 && wind_state==0){ //当前超出目标1度，要求送风
            //wind_state = 1;
            QJsonObject rectJson;
            rectJson.insert("Action", "Startwind");
            rectJson.insert("roomID", roomid);
            rectJson.insert("requiredwindspeed", speed);

            emit SendData(rectJson);
        }

        ui->lcd_currenttemp->display(QString::number(tem_now,'f',2));
    }

}

void User::monitor()    //定时发送当前温度
{
    if(state == 1){
        QJsonObject rectJson;
        rectJson.insert("Action", "Sendtemp");
        rectJson.insert("roomID", roomid);

        QString str = QString::number(tem_now, 'f', 2);
        rectJson.insert("currenttemp", str);

        emit SendData(rectJson);
    }
}

void User::checkout()   //退房
{
    QJsonObject rectJson;
    rectJson.insert("Action", "Checkout");
    rectJson.insert("roomID", roomid);

    state = 0;
    ui->on->setEnabled(false);
    ui->set_mode->setEnabled(false);
    ui->set_temp->setEnabled(false);
    ui->set_temp_2->setEnabled(false);
    ui->set_wind->setEnabled(false);
    ui->exit->setEnabled(false);
    emit SendData(rectJson);
}

void User::closeEvent(QCloseEvent * event)
{
    QJsonObject rectJson;
    rectJson.insert("Action", "Checkout");
    rectJson.insert("roomID", roomid);

    emit SendData(rectJson);
    exit(0);
}

void User::TemSend()
{
    emit SendData(cache);
}
