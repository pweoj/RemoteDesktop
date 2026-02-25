#include "displayui.h"
#include "ui_displayui.h"

DisplayUI::DisplayUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DisplayUI)
{
    ui->setupUi(this);
    ui->horizontalLayout->setStretchFactor(ui->label,1);


}
void DisplayUI::DisplayToLabel(const QImage&image)
{

    this->show();
    if (image.isNull()) return;


    QPixmap map = QPixmap::fromImage(image);
    ui->label->setMaximumSize(1200, 750);
    ui->label->setMinimumSize(800,500);
    ui->label->setScaledContents(1);

    ui->label->setPixmap(map);

}

DisplayUI::~DisplayUI()
{

    delete ui;
}

void DisplayUI::resizeEvent(QResizeEvent *event)//外部被自动调用
{
    QWidget::resizeEvent(event);
    qDebug()<<"event->size()"<<event->size();
    qDebug()<<"this->label->size()"<<this->ui->label->size();

}
