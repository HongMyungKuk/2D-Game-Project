#include <Windows.h>
#include <cassert>
#include <iostream>

#include "CutImage.h"

using namespace std;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

CutImage cutImage;

bool g_mouseBtnDown;
RECT g_rect;

int main()
{
    ULONG_PTR gdiPlusToken;

    Gdiplus::GdiplusStartupInput si;
    Gdiplus::GdiplusStartup(&gdiPlusToken, &si, NULL);

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(0,                   // Optional window styles.
                               CLASS_NAME,          // Window class
                               L"Sprite Tool",      // Window text
                               WS_OVERLAPPEDWINDOW, // Window style

                               // Size and position
                               CW_USEDEFAULT, CW_USEDEFAULT, 1280, 780,

                               NULL,         // Parent window
                               NULL,         // Menu
                               wc.hInstance, // Instance handle
                               NULL          // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);

    cutImage.Initialize(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Gdiplus::GdiplusShutdown(gdiPlusToken);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HDC memDC;
    static HBITMAP hBitmap;

    switch (uMsg)
    {
    case WM_SIZE: {
        int width = LOWORD(lParam);  // Macro to get the low-order word.
        int height = HIWORD(lParam); // Macro to get the high-order word.

        cout << "Width: " << width << ", Height: " << height << endl;
    }
        return FALSE;
    case WM_MOUSEMOVE: {
        if (g_mouseBtnDown == true)
        {
            cutImage.m_right = LOWORD(lParam);
            cutImage.m_bottom = HIWORD(lParam);
            cutImage.m_state = DRAG;
        }
        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_LBUTTONDOWN: {
        if (g_mouseBtnDown == false)
        {
            g_mouseBtnDown = true;

            cutImage.m_left = LOWORD(lParam);
            cutImage.m_top = HIWORD(lParam);

            cutImage.m_right = cutImage.m_left;
            cutImage.m_bottom = cutImage.m_top;
        }
        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_LBUTTONUP: {
        if (g_mouseBtnDown == true)
        {
            g_mouseBtnDown = false;

            cutImage.m_right = LOWORD(lParam);
            cutImage.m_bottom = HIWORD(lParam);
        }

        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_PAINT: {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        Rectangle(cutImage.m_hdc, 0, 0, 1280, 780);

        cutImage.Render();

        if (g_mouseBtnDown)
            cutImage.MakeBox();

        BitBlt(hdc, 0, 0, 1280, 780, cutImage.m_hdc, 0, 0, SRCCOPY);

        EndPaint(hwnd, &ps);
    }
        return FALSE;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
