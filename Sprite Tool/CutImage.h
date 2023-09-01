#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <iostream>

#pragma comment(lib, "Gdiplus.lib")

class CutImage
{
  public:
    CutImage();
    ~CutImage();

    void Initialize(HWND hwnd);
    void Render(int x, int y);
    void LoadPNGImage(HDC hdc);
    RECT FindCutPosition(int left, int top, int right, int bottom);
    void GetPixelColor(int xPos, int yPos, unsigned char &r, unsigned char &g, unsigned char &b);

  private:
    int FindLeftPosition(int left, int top, int right, int bottom);
    int FindTopPosition(int left, int top, int right, int bottom);
    int FindRightPosition(int left, int top, int right, int bottom);
    int FindBottomPosition(int left, int top, int right, int bottom);

  public:
    HWND m_hwnd = NULL;
    HDC m_hdc = NULL;
    HBITMAP m_hBitmap = NULL;
    Gdiplus::Image *m_pImage = NULL;
    Gdiplus::Graphics *g = NULL;

    UINT m_imageWidth;
    UINT m_imageHeight;
};
