#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QPushButton>
#include<QThread>
#include"server.h"
#include"client.h"
#include<QDebug>
#include"loginui.h"
#include<QHostAddress>
#include"displayui.h"
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QThread *SerTh=0;
    QThread* CliTh=0;
    Server*SerWork;
    Client*ClientWork;

    LoginUI*loginui=0;
    //UiControl *MainUI;

    void MainUI();//两个按钮
    void SerThreadCreat();
    void ClientThreadCreat();
    void TestDisplay();
signals:
    void SerThStartSignal();
    void ClientThStartSignal();
};
#endif // WIDGET_H
