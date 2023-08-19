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

void CutImage::MakeBox()
{
    FindCutPosition(m_left, m_top, m_right, m_bottom);

    // 드래그중엔 녹색으로 사각형 표시
    if (m_state == DRAG)
    {
        // 녹색 && Hollow 
        GDIObject gdiPen(m_hdc);
        gdiPen.ObjPen(PS_SOLID, 2, RGB(0, 255, 0));
        gdiPen.ObjBrush(HOLLOW_BRUSH);
        Rectangle(m_hdc, m_left, m_top, m_right, m_bottom);
    }
    // 선택 완료 시 하얀색으로 사각형 표시
    else if (m_state == SELECT)
    {
    }
}

void CutImage::FindCutPosition(int left, int top, int right, int bottom)
{
    if (m_left == m_right || m_top == m_bottom)
        return;

    m_left = FindLeftPosition(left, top, right, bottom);
    m_top = FindTopPosition(m_left, top, right, bottom);
    m_right = FindRightPosition(m_left, m_top, right, bottom);
    m_bottom = FindBottomPosition(m_left, m_top, m_right, bottom);
}

int CutImage::FindLeftPosition(int left, int top, int right, int bottom)
{
    COLORREF startColor = GetPixel(m_hdc, left, top);
    COLORREF currentColor = 0;
    for (int j = top; j <= bottom; ++j)
    {
        for (int i = left; i <= right; ++i)
        {
            currentColor = GetPixel(m_hdc, i, j);
            if (startColor != currentColor)
                return i;
        }
    }
    return 0;
}

int CutImage::FindTopPosition(int left, int top, int right, int bottom)
{
    return top;
}

int CutImage::FindRightPosition(int left, int top, int right, int bottom)
{
    return right;
}

int CutImage::FindBottomPosition(int left, int top, int right, int bottom)
{
    return bottom;
}
