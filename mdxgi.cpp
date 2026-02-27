#include<QImage>
#include "mdxgi.h"
#include<dxgi.h>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <QDebug>
#include <QThread>

DxGI::DxGI(QObject *parent)
    : QObject(parent), device(nullptr), context(nullptr), duplication(nullptr), staging(nullptr),
    texWidth(0), texHeight(0)
{
}

DxGI::~DxGI()
{
    // 安全释放COM对象，避免泄漏
    if (staging) {
        ((ID3D11Texture2D*)staging)->Release();
        staging = nullptr;
    }
    if (duplication) {
        ((IDXGIOutputDuplication*)duplication)->Release();
        duplication = nullptr;
    }
    if (context) {
        ((ID3D11DeviceContext*)context)->Release();
        context = nullptr;
    }
    if (device) {
        ((ID3D11Device*)device)->Release();
        device = nullptr;
    }
}

bool DxGI::DxGiInit()
{
    qDebug() << "[DxGiInit] 开始初始化D3D11设备";
    ID3D11Device* pdevice = nullptr;
    ID3D11DeviceContext* pcontext = nullptr;

    // 修复：添加BGRA支持+降级WARP渲染，提高初始化成功率
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, // 关键：支持BGRA格式
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &pdevice,
        &featureLevel,
        &pcontext
        );

    // 硬件渲染失败，降级到软件渲染（WARP）
    if (FAILED(hr)) {
        qWarning() << "[DxGiInit] 硬件渲染失败，尝试WARP软件渲染";
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &pdevice,
            &featureLevel,
            &pcontext
            );
        if (FAILED(hr)) {
            qCritical() << "[DxGiInit] D3D11CreateDevice失败，HR=" << hr;
            return false;
        }
    }

    device = pdevice;
    context = pcontext;
    qDebug() << "[DxGiInit] D3D11设备创建成功，FeatureLevel=" << featureLevel;

    // 步骤1：获取DXGI Device
    IDXGIDevice* dxgiDevice = nullptr;
    hr = pdevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] QueryInterface DXGI Device失败，HR=" << hr;
        pcontext->Release();
        pdevice->Release();
        device = context = nullptr;
        return false;
    }

    // 步骤2：获取Adapter
    IDXGIAdapter* adapter = nullptr;
    hr = dxgiDevice->GetAdapter(&adapter);
    dxgiDevice->Release(); // 用完立即释放
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] GetAdapter失败，HR=" << hr;
        pcontext->Release();
        pdevice->Release();
        device = context = nullptr;
        return false;
    }

    // 步骤3：获取Output
    IDXGIOutput* output = nullptr;
    hr = adapter->EnumOutputs(0, &output);
    adapter->Release(); // 用完立即释放
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] EnumOutputs失败，HR=" << hr;
        pcontext->Release();
        pdevice->Release();
        device = context = nullptr;
        return false;
    }

    // 步骤4：获取Output1（支持DuplicateOutput）
    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release(); // 用完立即释放
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] QueryInterface Output1失败，HR=" << hr;
        pcontext->Release();
        pdevice->Release();
        device = context = nullptr;
        return false;
    }

    // 步骤5：获取桌面复制句柄（最关键，需要管理员权限）
    IDXGIOutputDuplication* pduplication = nullptr;
    hr = output1->DuplicateOutput(pdevice, &pduplication);
    output1->Release(); // 用完立即释放
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] DuplicateOutput失败！HR=" << hr;
        qCritical() << "必看原因：1. 未以管理员身份运行 2. 其他录屏软件占用 3. 显卡不支持DxGI 1.2";
        pcontext->Release();
        pdevice->Release();
        device = context = nullptr;
        return false;
    }
    duplication = pduplication;
    qDebug() << "[DxGiInit] 桌面复制句柄创建成功";

    // 步骤6：获取第一帧，初始化Staging纹理
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    hr = pduplication->AcquireNextFrame(1000, &frameInfo, &desktopResource); // 延长超时到1秒
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] AcquireNextFrame初始化失败，HR=" << hr;
        pduplication->Release();
        pcontext->Release();
        pdevice->Release();
        duplication = device = context = nullptr;
        return false;
    }

    // 步骤7：转换为Texture2D，获取分辨率
    ID3D11Texture2D* desktopImage = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopImage);
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] QueryInterface Texture2D失败，HR=" << hr;
        pduplication->ReleaseFrame();
        desktopResource->Release();
        pduplication->Release();
        pcontext->Release();
        pdevice->Release();
        duplication = device = context = nullptr;
        return false;
    }

    D3D11_TEXTURE2D_DESC desc;
    desktopImage->GetDesc(&desc);
    texWidth = desc.Width;
    texHeight = desc.Height;
    qDebug() << "[DxGiInit] 截屏分辨率：" << texWidth << "x" << texHeight;

    // 步骤8：创建Staging纹理（CPU可读取）
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    ID3D11Texture2D* pstaging = nullptr;
    hr = pdevice->CreateTexture2D(&desc, nullptr, &pstaging);
    if (FAILED(hr)) {
        qCritical() << "[DxGiInit] CreateTexture2D Staging失败，HR=" << hr;
        pduplication->ReleaseFrame();
        desktopImage->Release();
        desktopResource->Release();
        pduplication->Release();
        pcontext->Release();
        pdevice->Release();
        duplication = device = context = nullptr;
        return false;
    }
    staging = pstaging;

    // 释放临时资源
    pduplication->ReleaseFrame();
    desktopImage->Release();
    desktopResource->Release();

    qDebug() << "[DxGiInit] 所有初始化步骤完成！";
    return true;
}

bool DxGI::DxGetOneFrame()
{
    // 修复1：严格检查所有核心指针，为空直接返回（消除空指针警告）
    if (!duplication || !context || !staging) {
        qWarning() << "[DxGetOneFrame] 核心COM对象为空，跳过截屏";
        return false;
    }

    ID3D11DeviceContext* pcontext = (ID3D11DeviceContext*)context;
    IDXGIOutputDuplication* pduplication = (IDXGIOutputDuplication*)duplication;
    ID3D11Texture2D* pstaging = (ID3D11Texture2D*)staging;

    // 步骤1：获取下一帧
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = pduplication->AcquireNextFrame(50, &frameInfo, &desktopResource); // 延长超时到50ms

    // 修复2：超时正常返回（不报错）
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        // qDebug() << "[DxGetOneFrame] 无新帧（超时）";
        return false;
    }

    // 修复3：其他失败直接返回，释放资源
    if (FAILED(hr)) {
        qCritical() << "[DxGetOneFrame] AcquireNextFrame失败，HR=" << hr;
        return false;
    }

    // 步骤2：转换为Texture2D
    ID3D11Texture2D* desktopImage = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopImage);
    if (FAILED(hr)) {
        qCritical() << "[DxGetOneFrame] QueryInterface Texture2D失败，HR=" << hr;
        pduplication->ReleaseFrame();
        desktopResource->Release();
        return false;
    }

    // 步骤3：复制到Staging纹理
    pcontext->CopyResource(pstaging, desktopImage);

    // 步骤4：映射到CPU内存
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = pcontext->Map(pstaging, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        qCritical() << "[DxGetOneFrame] Map失败，HR=" << hr;
        pduplication->ReleaseFrame();
        desktopImage->Release();
        desktopResource->Release();
        return false;
    }

    // 步骤5：转换为QImage（深拷贝避免内存失效）
    QImage image((uchar*)mapped.pData, texWidth, texHeight, mapped.RowPitch, QImage::Format_RGBA8888);
    QImage fixedImage = image.copy().rgbSwapped(); // 先拷贝再转格式
    QImage finalImage = fixedImage.copy(); // 深拷贝，确保独立内存

    // 步骤6：解锁资源
    pcontext->Unmap(pstaging, 0);

    // 步骤7：释放所有临时资源
    pduplication->ReleaseFrame();
    desktopImage->Release();
    desktopResource->Release();

    // 步骤8：发射信号（此时所有步骤成功，必走到这一步）
    emit CaptureIsOk(finalImage);
    //qDebug() << "[DxGetOneFrame] 成功获取帧，发射CaptureIsOk信号";
    return true;
}

void DxGI::DxCaptureScreen()
{
    qDebug() << "[DxCaptureScreen] 启动截屏线程，线程ID=" << QThread::currentThreadId();

    // 先初始化DxGI（子线程内）
    if (!DxGiInit()) {
        qCritical() << "[DxCaptureScreen] DxGiInit初始化失败，退出截屏循环";
        return;
    }

    // 修复：可中断循环+休眠，降低CPU占用
    while (!QThread::currentThread()->isInterruptionRequested()) {
        DxGetOneFrame();
        QThread::msleep(5); // 5ms休眠，避免高频调用
    }

    qDebug() << "[DxCaptureScreen] 截屏循环正常退出";
}
