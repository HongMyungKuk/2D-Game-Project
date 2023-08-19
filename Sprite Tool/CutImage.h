#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <iostream>

#pragma comment(lib, "Gdiplus.lib")

enum BOX_STATE
{
    NONE,
    DRAG,
    SELECT,
};

class CutImage
{
  public:
    CutImage();
    ~CutImage();

    void Initialize(HWND hwnd);
    void Render();
    void LoadPNGImage(HDC hdc);
    void MakeBox();
    void FindCutPosition(int left, int top, int right, int bottom);

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
    RECT m_box;
    Gdiplus::Graphics *g = NULL;
    BOX_STATE m_state;

    int m_left = 0;
    int m_top = 0;
    int m_right = 0;
    int m_bottom = 0;
};
