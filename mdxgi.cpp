#include<QImage>
#include "mdxgi.h"
#include<dxgi.h>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>

DxGI::DxGI(QObject *parent)
    : QObject(parent)
{
}

DxGI::~DxGI()
{
    if (staging)
        ((ID3D11Texture2D*)staging)->Release();

    if (duplication)
        ((IDXGIOutputDuplication*)duplication)->Release();

    if (context)
        ((ID3D11DeviceContext*)context)->Release();

    if (device)
        ((ID3D11Device*)device)->Release();
}
bool DxGI::DxGiInit()
{
    ID3D11Device* pdevice = nullptr;
    ID3D11DeviceContext* pcontext = nullptr;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &pdevice,
        nullptr,
        &pcontext
        );

    if (FAILED(hr))
        return false;

    device = pdevice;
    context = pcontext;

    IDXGIDevice* dxgiDevice = nullptr;
    IDXGIAdapter* adapter = nullptr;
    IDXGIOutput* output = nullptr;
    IDXGIOutput1* output1 = nullptr;
    IDXGIOutputDuplication* pduplication = nullptr;

    pdevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    dxgiDevice->GetAdapter(&adapter);
    adapter->EnumOutputs(0, &output);
    output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);

    hr = output1->DuplicateOutput(pdevice, &pduplication);

    output1->Release();
    output->Release();
    adapter->Release();
    dxgiDevice->Release();

    if (FAILED(hr))
        return false;

    duplication = pduplication;

    // 先抓一帧用于创建 staging
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;

    hr = pduplication->AcquireNextFrame(100, &frameInfo, &desktopResource);
    if (FAILED(hr))
        return false;

    ID3D11Texture2D* desktopImage = nullptr;
    desktopResource->QueryInterface(
        __uuidof(ID3D11Texture2D),
        (void**)&desktopImage
        );

    D3D11_TEXTURE2D_DESC desc;
    desktopImage->GetDesc(&desc);

    texWidth = desc.Width;
    texHeight = desc.Height;

    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    ID3D11Texture2D* pstaging = nullptr;
    pdevice->CreateTexture2D(&desc, nullptr, &pstaging);

    staging = pstaging;

    pduplication->ReleaseFrame();
    desktopImage->Release();
    desktopResource->Release();

    return true;
}
QImage DxGI::DxGetOneFrame()
{
    if (!duplication)
        return QImage();

    ID3D11DeviceContext* pcontext =
        (ID3D11DeviceContext*)context;

    IDXGIOutputDuplication* pduplication =
        (IDXGIOutputDuplication*)duplication;

    ID3D11Texture2D* pstaging =
        (ID3D11Texture2D*)staging;

    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;

    HRESULT hr = pduplication->AcquireNextFrame(
        16,
        &frameInfo,
        &desktopResource
        );

    if (hr == DXGI_ERROR_WAIT_TIMEOUT)
        return QImage();

    if (FAILED(hr))
        return QImage();

    ID3D11Texture2D* desktopImage = nullptr;
    desktopResource->QueryInterface(
        __uuidof(ID3D11Texture2D),
        (void**)&desktopImage
        );

    pcontext->CopyResource(pstaging, desktopImage);

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = pcontext->Map(pstaging, 0, D3D11_MAP_READ, 0, &mapped);

    if (FAILED(hr))
    {
        pduplication->ReleaseFrame();
        desktopImage->Release();
        desktopResource->Release();
        return QImage();
    }

    QImage image(
        (uchar*)mapped.pData,
        texWidth,
        texHeight,
        mapped.RowPitch,
        QImage::Format_RGBA8888
        );

    QImage fixedImage = image.copy(0, 0, texWidth, texHeight).rgbSwapped();
    QImage copy = fixedImage.copy();   // 必须 copy

    pcontext->Unmap(pstaging, 0);

    pduplication->ReleaseFrame();
    desktopImage->Release();
    desktopResource->Release();

    return copy;
}
