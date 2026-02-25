#ifndef LOGINUI_H
#define LOGINUI_H

#include <QWidget>
#include<QHostAddress>
#include<QString>
namespace Ui {
class LoginUI;
}

class LoginUI : public QWidget
{
    Q_OBJECT

public:
    explicit LoginUI(QWidget *parent = nullptr);
    QHostAddress getSerAddress();
    void setOk();
    void getdata();
    ~LoginUI();

private:
    Ui::LoginUI *ui;
signals:
   void SendData(QHostAddress ip,int port);
};

#endif // LOGINUI_H
