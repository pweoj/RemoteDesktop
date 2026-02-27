#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    MainUI();
    //TestDisplay();
}

void Widget::MainUI()
{
    qDebug()<<"........";
    this->setMinimumSize(300,200);

    QPushButton*serverBtn=new QPushButton(this);
    serverBtn->setText("启动服务");

    QPushButton*ClientBtn=new QPushButton(this);
    ClientBtn->setText("启动客户端");
    ClientBtn->move(0,50);

    connect(serverBtn,&QPushButton::clicked,this,&Widget::SerThreadCreat);
    connect(ClientBtn,&QPushButton::clicked,this,&Widget::ClientThreadCreat);


}

void Widget::SerThreadCreat()
{
    SerTh=new QThread(this);
    SerWork=new Server;

    connect(this,&Widget::SerThStartSignal,SerWork,&Server::ServerRun);//服务端启动


    SerWork->moveToThread(SerTh);
    SerTh->start();

    TestDisplay();//测试屏幕捕捉
    emit SerThStartSignal();
}

void Widget::ClientThreadCreat()
{
    CliTh=new QThread(this);
    ClientWork=new Client;

    connect(this,&Widget::ClientThStartSignal,ClientWork,&Client::ClientRun);//客户端启动

    loginui=new LoginUI(this);loginui->setWindowFlags(Qt::Window);
    loginui->show();
    connect(loginui,&LoginUI::SendData,this,[=](QHostAddress ip,int port){
        ClientWork->SerIp=ip;ClientWork->SerPort=port;
        emit ClientThStartSignal();
    });//设置连接参数
    connect(ClientWork,&Client::ConnectIsOk,loginui,&LoginUI::setOk);

    ClientWork->moveToThread(CliTh);
    CliTh->start();


}

void Widget::TestDisplay()
{
    DisplayUI *displayui=new DisplayUI(this);
    connect(SerWork,&Server::sendFrame,displayui,&DisplayUI::DisplayToLabel);

}

Widget::~Widget() {}
