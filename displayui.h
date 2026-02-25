#ifndef DISPLAYUI_H
#define DISPLAYUI_H

#include <QMainWindow>
#include"server.h"
#include<QResizeEvent>
#include<QSize>
namespace Ui {
class DisplayUI;
}

class DisplayUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit DisplayUI(QWidget *parent = nullptr);
    void DisplayToLabel(const QImage&image);
    ~DisplayUI();

protected:
    void resizeEvent(QResizeEvent*event) override;
private:

    Ui::DisplayUI *ui;
};

#endif // DISPLAYUI_H
