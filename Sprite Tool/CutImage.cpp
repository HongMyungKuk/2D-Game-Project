#include "CutImage.h"
#include "GDIObject.h"

CutImage::CutImage()
{
    
}

CutImage::~CutImage()
{
    delete m_pImage;
    delete g;

    ReleaseDC(m_hwnd, m_hdc);
}

void CutImage::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;
    HDC hdc = GetDC(hwnd);
    m_hdc = CreateCompatibleDC(hdc);
    m_hBitmap = CreateCompatibleBitmap(hdc, 1280, 780);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(m_hdc, m_hBitmap);
    DeleteObject(hOldBitmap);

    g = new Gdiplus::Graphics(m_hdc);

    LoadPNGImage(m_hdc);
}

void CutImage::Render()
{
    g->DrawImage(m_pImage, 0, 0);
}

void CutImage::LoadPNGImage(HDC hdc)
{
    m_pImage = Gdiplus::Image::FromFile(L"kyo.png");
}

RECT CutImage::FindCutPosition(int left, int top, int right, int bottom)
{
    std::cout << left << std::endl;
    int newLeft = FindLeftPosition(left, top, right, bottom);
    int newTop = FindTopPosition(newLeft, top, right, bottom);
    int newRight = FindRightPosition(newLeft, newTop, right, bottom);
    int newBottom = FindBottomPosition(newLeft, newTop, newRight, bottom);

    return RECT{newLeft, newTop, newRight, newBottom};
}

int CutImage::FindLeftPosition(int left, int top, int right, int bottom)
{
    COLORREF startColor = GetPixel(m_hdc, left, top);
    std::cout << startColor << std::endl; 
    COLORREF currentColor = 0;
    for (int i = left; i <= right; ++i)
    {
        for (int j = top; j <= bottom; ++j)
        {
            currentColor = GetPixel(m_hdc, i, j);
            if (startColor != currentColor)
            {
                std::cout << "color: " << currentColor << std::endl;
                std::cout << i << std::endl;
                return i;
            }
        }
    }
    return 0;
}

int CutImage::FindTopPosition(int left, int top, int right, int bottom)
{
    COLORREF startColor = GetPixel(m_hdc, left, top);
    std::cout << startColor << std::endl;
    COLORREF currentColor = 0;
    for (int i = top; i <= bottom; ++i)
    {
        for (int j = left; j <= right; ++j)
        {
            currentColor = GetPixel(m_hdc, j, i);
            if (startColor != currentColor)
            {
                std::cout << "color: " << currentColor << std::endl;
                std::cout << i << std::endl;
                return i;
            }
        }
    }
    return 0;
}

int CutImage::FindRightPosition(int left, int top, int right, int bottom)
{
    COLORREF startColor = GetPixel(m_hdc, right, bottom);
    std::cout << startColor << std::endl;
    std::cout << right << std::endl;
    COLORREF currentColor = 0;
    for (int i = right; i >= left; --i)
    {
        for (int j = top; j <= bottom; ++j)
        {
            currentColor = GetPixel(m_hdc, i, j);
            if (startColor != currentColor)
            {
                std::cout << "color: " << currentColor << std::endl;
                std::cout << i << std::endl;
                return i;
            }
        }
    }
    return 0;
}

int CutImage::FindBottomPosition(int left, int top, int right, int bottom)
{
    COLORREF startColor = GetPixel(m_hdc, right, bottom);
    std::cout << startColor << std::endl;
    COLORREF currentColor = 0;
    for (int i = bottom; i >= top; --i)
    {
        for (int j = right; j >= left; --j)
        {
            currentColor = GetPixel(m_hdc, j, i);
            if (startColor != currentColor)
            {
                std::cout << "color: " << currentColor << std::endl;
                std::cout << i << std::endl;
                return i;
            }
        }
    }
    return 0;
}
