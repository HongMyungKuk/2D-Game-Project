#include <Windows.h>
#include <cassert>
#include <iostream>

#include "CutImage.h"
#include "GDIObject.h"

using namespace std;

enum BOX_STATE
{
    NONE,
    DRAG,
    SELECT,
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Box(HDC hdc, RECT rect);
void OnVScroll(HWND hwnd, WPARAM wParam, LPARAM lParam);

CutImage cutImage;
POINT oldPos;
POINT newPos;
RECT box;
RECT optBox;
int g_width, g_height;
POINT tempPos;

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

    HWND hwnd = CreateWindowEx(0,                                             // Optional window styles.
                               CLASS_NAME,                                    // Window class
                               L"Sprite Tool",                                // Window text
                               WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL, // Window style

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
    static BOX_STATE boxState = NONE;

    switch (uMsg)
    {
    case WM_CREATE: {
        //SetScrollRange(hwnd, SB_VERT, 0, g_height, true);
        //SetScrollRange(hwnd, SB_HORZ, 0, g_width, true);
        //SetScrollPos(hwnd, SB_VERT, 0, true);
        //SetScrollPos(hwnd, SB_HORZ, 0, true);

        SetScrollRange(hwnd, SB_BOTH, 0, 255, true);
        SetScrollPos(hwnd, SB_BOTH, 0, true);
    }
        return FALSE;
    case WM_SIZE: {
        g_width = LOWORD(lParam);  // Macro to get the low-order word.
        g_height = HIWORD(lParam); // Macro to get the high-order word.
    }
        return FALSE;
    case WM_HSCROLL: {

        switch (LOWORD(wParam))
        {
        case SB_LINELEFT:
            tempPos.x = max(0, tempPos.x - 1);
            break;
        case SB_LINERIGHT:
            tempPos.x = min(g_width, tempPos.x + 1);
            break;
        case SB_PAGELEFT:
            tempPos.x = max(0, tempPos.x - 10);
            break;
        case SB_PAGERIGHT:
            tempPos.x = min(g_width, tempPos.x + 10);
            break;
        case SB_THUMBTRACK:
            tempPos.x = HIWORD(wParam);
            break;
        }
        SetScrollPos(hwnd, SB_HORZ, tempPos.x, true);
        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_VSCROLL: {

        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
            tempPos.y = max(0, tempPos.y - 1);
            break;
        case SB_LINEDOWN:
            tempPos.y = min(g_width, tempPos.y + 1);
            break;
        case SB_PAGEUP:
            tempPos.y = max(0, tempPos.y - 10);
            break;
        case SB_PAGEDOWN:
            tempPos.y = min(g_width, tempPos.y + 10);
            break;
        case SB_THUMBTRACK:
            tempPos.y = HIWORD(wParam);
            break;
        }
        SetScrollPos(hwnd, SB_VERT, tempPos.y, true);
        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_MOUSEMOVE: {
        if (boxState == DRAG)
        {
            // ¸¶¿ì½ºÀÇ RB ÁÂÇ¥
            newPos.x = LOWORD(lParam);
            newPos.y = HIWORD(lParam);
            SetRect(&box, oldPos.x + tempPos.x, oldPos.y + tempPos.y, newPos.x + tempPos.x, newPos.y + tempPos.y);
        }
        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_LBUTTONDOWN: {
        if (boxState != DRAG)
        {
            boxState = DRAG;
            // ¸¶¿ì½ºÀÇ LT ÁÂÇ¥
            oldPos.x = LOWORD(lParam);
            oldPos.y = HIWORD(lParam);

            newPos.x = oldPos.x;
            newPos.y = newPos.y;

            cout << oldPos.x << " " << oldPos.y << endl;

            InvalidateRect(hwnd, NULL, true);
        }
    }
        return FALSE;
    case WM_LBUTTONUP: {
        if (boxState == DRAG)
        {
            boxState = SELECT;
            // ¸¶¿ì½ºÀÇ RB ÁÂÇ¥
            newPos.x = LOWORD(lParam);
            newPos.y = HIWORD(lParam);
            // ÀÚ¸£±â ÁÂÇ¥ Ã£±â
            optBox = cutImage.FindCutPosition(oldPos.x + tempPos.x, oldPos.y + tempPos.y, newPos.x + tempPos.x,
                                              newPos.y + tempPos.y);
        }
        InvalidateRect(hwnd, NULL, true);
    }
        return FALSE;
    case WM_PAINT: {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        // È­¸é Clear
        Rectangle(cutImage.m_hdc, 0, 0, 1280, 780);

        // Image Render
        cutImage.Render(tempPos.x, tempPos.y);

        // Box Render
        switch (boxState)
        {
        case DRAG: {
            Box(cutImage.m_hdc, box);
        }
        break;
        case SELECT: {
            Box(cutImage.m_hdc, optBox);
        }
        break;
        }

        BitBlt(hdc, 0, 0, 1280, 780, cutImage.m_hdc, tempPos.x, tempPos.y, SRCCOPY);

        EndPaint(hwnd, &ps);
    }
        return FALSE;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Box(HDC hdc, RECT rect)
{
    GDIObject obj(hdc);
    obj.ObjBrush(HOLLOW_BRUSH);
    Rectangle(hdc, rect.left - 1, rect.top - 1, rect.right + 2, rect.bottom + 2);
}
