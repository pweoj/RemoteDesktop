#include "loginui.h"
#include "ui_loginui.h"

LoginUI::LoginUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginUI)
{
    ui->setupUi(this);
    getdata();
}

void LoginUI::setOk()
{
    ui->Note->setText("已连接");
}


void LoginUI::getdata()
{
    connect(ui->pushButton,&QPushButton::clicked,[=](){
        QString ip_s=ui->IPLineEditor->text();
        QHostAddress ip=QHostAddress(ip_s);

        int port=ui->PortLineEditor->text().toInt();
        emit SendData(ip,port);
    });
}

LoginUI::~LoginUI()
{
    delete ui;
}
