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
    //�����źźͲۺ���
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
    //tem_now  = qrand()%11 + 20; //�����ǰ�¶�
    tem_now = 25;
    emit SendData(rectJson);
}

void User::newConnect()
{
    blocksize = 0;  // ��ʼ�����ݴ�С��ϢΪ0
    tcpSocket->abort(); // ȡ�����е�����
    tcpSocket->connectToHost("10.28.191.237", 5555);    // ����������
    tcpSocket->waitForConnected(-1);
}

void User::readMessage()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_9); //�����������İ汾���ͻ��˺ͷ�������ʹ�õİ汾Ҫ��ͬ
    //if(blocksize == 0){
        //if(tcpSocket->bytesAvailable()<(int)sizeof(quint16)) return;    //�жϽ��յ������Ƿ�������ֽڣ�Ҳ�����ļ��Ĵ�С��Ϣ��ռ�Ŀռ�
        in >> blocksize;    //������򱣴浽blocksize�����У�����ֱ�ӷ��أ�������������
    //}
    //if(tcpSocket->bytesAvailable()<blocksize) return;   //���û�еõ�ȫ�������ݣ��򷵻أ�������������
    in >> message;  //�����յ������ݴ�ŵ�������

    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toLocal8Bit().data());
    QJsonObject respond = jsonDoc.object();

    qDebug()<<"Recv:    "<<respond;

    if(respond.value("Action").toString()=="Login_S"){  //��½�ɹ�
        this->show();
        ui->set_mode->setEnabled(false);
        ui->set_temp->setEnabled(false);
        ui->set_temp_2->setEnabled(false);
        ui->set_wind->setEnabled(false);
        ui->exit->setEnabled(false);
    }
    else if(respond.value("Action").toString()=="Login_F"){  //��½ʧ��
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���������"));

    }
    else if(respond.value("Action").toString()=="Turnon_S"){    //�����ɹ�
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
    else if(respond.value("Action").toString()=="Turnon_F"){    //����ʧ��
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����յ�δ����"));
    }
    else if(respond.value("Action").toString()=="Turnoff_S"){   //�ػ�
        ui->lcd_goaltemp->display("");
        ui->lcd_currenttemp->display("");
        ui->wind_speed->setText(NULL);
        state=0;
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        tem_now  = qrand()%11 + 20; //�����ǰ�¶�

        ui->set_mode->setEnabled(false);
        ui->set_temp->setEnabled(false);
        ui->set_temp_2->setEnabled(false);
        ui->set_wind->setEnabled(false);
        ui->exit->setEnabled(false);
    }
    else if(respond.value("Action").toString()=="Changemode_S"){    //�л�ģʽ
        if(mode == 0) mode = 1;
        else if(mode == 1) mode = 0;

        if(mode == 0)
            ui->model_of_main->setText("COOL");
        else if(mode == 1)
            ui->model_of_main->setText("HOT");
    }
    else if(respond.value("Action").toString()=="Changewind_S"){    //�л�����
        speed = respond.value("requiredwindspeed").toVariant().toInt();
        ui->wind_speed->setText(QString::number(speed));
        speed_now = speed;
        ui->wind_speed_2->setText(QString::number(speed_now));
    }
    else if(respond.value("Action").toString()=="Changetemp_S"){    //�л��¶ȳɹ�
        tem_set = respond.value("settem").toVariant().toInt();
        temp = tem_set;
        ui->lcd_goaltemp->display(tem_set);
    }
    else if(respond.value("Action").toString()=="Changetemp_F"){    //�л��¶�ʧ��
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�¶ȳ�����Χ"));
    }
    else if(respond.value("Action").toString()=="Sendtemp_S"){  //��ʱ�����¶�

    }
    else if(respond.value("Action").toString()=="Stopwind_S" || respond.value("Action").toString()=="STOP"){  //ֹͣ�ͷ�
        wind_state = 0;
        speed_now = 0;
        ui->wind_speed_2->setText(QString::number(speed_now));
    }
    else if(respond.value("Action").toString()=="Startwind_S" || respond.value("Action").toString()=="START"){ //�����ͷ�
        wind_state = 1;
        speed_now = speed;
        ui->wind_speed_2->setText(QString::number(speed_now));
    }
    else if(respond.value("Action").toString()=="Checkout_S"){  //�˷�
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

    QByteArray block;   //�����ݴ�Ҫ���͵�����
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_6);    //�����������İ汾���ͻ��˺ͷ�������ʹ�õİ汾Ҫ��ͬ
    out << (quint16)0;  //Ԥ�����ֽ�
    out << request;

    out.device()->seek(0);  //  ��ת�����ݿ�Ŀ�ͷ
    out << (quint16)(block.size() - sizeof(quint16));   //��д��С��Ϣ

    qDebug()<<"Send:    "<<block;
    tcpSocket->write(block);
    tcpSocket->flush();
}

void User::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpSocket->errorString();
    QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ӷ�����������������"));
    exit(0);
}

void User::tem_up() //�¶�����
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

void User::tem_down()   //�¶��½�
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

void User::speed_change()   //���ķ���
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

void User::state_change()   //����
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

void User::mode_change()    //����ģʽ
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

void User::init(QJsonObject respond)   //�յ���ʼ��
{
    speed = respond.value("windspeed").toVariant().toInt(); //Ĭ�Ϸ���
    state = 0;  //Ĭ�Ϲػ�
    if(respond.value("mode").toString()=="cold") //Ĭ��ģʽ
        mode = 0;
    else if(respond.value("mode").toString()=="hot")
        mode = 1;
    tem_set = respond.value("starttemp").toVariant().toInt();    //Ĭ���¶�
    temp = tem_set;
    tem_init = tem_now;
    wind_state = 0;
    speed_now = 0;

    int ww;
    if(mode==0 && tem_now<=tem_set)    //�����ҵ�ǰС���趨
        ww = 0;
    else if(mode==1 && tem_now>=tem_set)   //�����ҵ�ǰ�����趨
        ww = 0;
    else if(mode==0 && tem_now>tem_set)    //�����ҵ�ǰ�����趨
        ww = 1;
    else if(mode==1 && tem_now<tem_set)    //�����ҵ�ǰС���趨
        ww = 1;

    if(ww==1){
        QJsonObject rectJson;
        rectJson.insert("Action", "Startwind");
        rectJson.insert("roomID", roomid);
        rectJson.insert("requiredwindspeed", speed);

        emit SendData(rectJson);
    }
}

void User::refresh()    //ˢ��ʵʱ�¶�
{
    if(state == 1){ //����״̬
        if(mode==0 && speed_now==0){    //�����ҵ�ǰС���趨
            //if(tem_now<tem_init)
                tem_now = tem_now + 0.5/60;
        }
        else if(mode==1 && speed_now==0){   //�����ҵ�ǰ�����趨
            //if(tem_now>tem_init)
                tem_now = tem_now - 0.5/60;
        }
        else if(mode==0 && wind_state==1){    //�����ҵ�ǰС���趨
            if(speed_now == 1)
                tem_now = tem_now - 0.4/60;
            else if(speed_now == 2)
                tem_now = tem_now - 0.5/60;
            else
                tem_now = tem_now - 0.6/60;
        }
        else if(mode==1 && wind_state==1){    //�����ҵ�ǰС���趨
            if(speed_now == 1)
                tem_now = tem_now + 0.4/60;
            else if(speed_now == 2)
                tem_now = tem_now + 0.5/60;
            else
                tem_now = tem_now + 0.6/60;
        }

        if(qAbs(tem_now - tem_set)<=0.01 && wind_state==1){  //�¶ȵ���Ŀ���������ͷ磬Ҫ��ֹͣ
            //wind_state = 0;
            QJsonObject rectJson;
            rectJson.insert("Action", "Stopwind");
            rectJson.insert("roomID", roomid);

            emit SendData(rectJson);
        }
        if(qAbs(tem_now - tem_set)>=1 && wind_state==0){ //��ǰ����Ŀ��1�ȣ�Ҫ���ͷ�
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

void User::monitor()    //��ʱ���͵�ǰ�¶�
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

void User::checkout()   //�˷�
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
