#ifndef MDXGI_H
#define MDXGI_H

#include <QObject>

#include<QDebug>
//typedef unsigned int UINT;
class DxGI : public QObject
{
    Q_OBJECT
public:
    explicit DxGI(QObject *parent = nullptr);
    ~DxGI();

    bool DxGiInit();
    QImage DxGetOneFrame();

private:
    void* device = nullptr;
    void* context = nullptr;
    void* duplication = nullptr;
    void* staging = nullptr;

    unsigned int texWidth = 0;
    unsigned int texHeight = 0;
};
#endif // MDXGI_H
