#ifndef MDXGI_H
#define MDXGI_H

#include <QObject>
#include<QThread>
#include<QDebug>
//typedef unsigned int UINT;
class DxGI : public QObject
{
    Q_OBJECT
public:
    explicit DxGI(QObject *parent = nullptr);
    ~DxGI();

    bool DxGiInit();
    bool DxGetOneFrame();
    void DxCaptureScreen();

private:
    void* device = nullptr;
    void* context = nullptr;
    void* duplication = nullptr;
    void* staging = nullptr;

    unsigned int texWidth = 0;
    unsigned int texHeight = 0;
signals:
    void CaptureIsOk(const QImage &image);
};
#endif // MDXGI_H
