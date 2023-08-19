#pragma once

#include <windows.h>

class GDIObject

{
  public:
    GDIObject()
    {
    }

    GDIObject(HDC hdc, int penStyle = PS_SOLID, int width = 1, COLORREF color = RGB(0, 255, 0))
    {
        Initialize(hdc, penStyle, width, color);
    }
    ~GDIObject()
    {
        SelectObject(m_hdc, m_hOldPen);
        DeleteObject(m_hPen);
    }

    void Initialize(HDC hdc, int penStyle, int width, COLORREF color)
    {
        m_hdc = hdc;
        m_hPen = CreatePen(penStyle, width, color);
        m_hOldPen = (HPEN)SelectObject(m_hdc, m_hPen);
    }

    void ObjPen(int penStyle, int width, COLORREF color)
    {
        m_hPen = CreatePen(penStyle, width, color);
        m_hOldPen = (HPEN)SelectObject(m_hdc, m_hPen);
    }

    void ObjBrush(int type)
    {
        m_hBrush = (HBRUSH)GetStockObject(type);
        SelectObject(m_hdc, m_hBrush);
    }

  public:
    HDC m_hdc = NULL;
    HPEN m_hPen = NULL;
    HPEN m_hOldPen = NULL;
    HBRUSH m_hBrush = NULL;
};
