#include <Windows.h>
#include <CommCtrl.h>
#include <cassert>
#include <iostream>

#include "CutImage.h"
#include "GDIObject.h"
#include "resource.h"

#define WINDOW_LEFT 100
#define WINDOW_TOP 100
#define WINDOW_PADDING 15

using namespace std;

enum BOX_STATE
{
    NONE,
    DRAG,
    SELECT,
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Box(HDC hdc, RECT rect);
void ScrollProcess(HWND hwnd, int imageWidth, int imageHeight, int &xCurrentScroll, int &yCurrentScroll);

HWND g_hDlg;
HWND g_hListCtrl;
RECT g_mainWndSize = {0, 0, 800, 600};
RECT g_clientSize;
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
                               100, 100, CW_USEDEFAULT, CW_USEDEFAULT,

                               NULL,         // Parent window
                               NULL,         // Menu
                               wc.hInstance, // Instance handle
                               NULL          // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);

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
    // These variables are required for horizontal scrolling.
    static UINT xMinScroll = 0;     // minimum horizontal scroll value
    static UINT xCurrentScroll = 0; // current horizontal scroll value
    static UINT xMaxScroll = 0;     // maximum horizontal scroll value

    // These variables are required for vertical scrolling.
    static UINT yMinScroll = 0;     // minimum vertical scroll value
    static UINT yCurrentScroll = 0; // current vertical scroll value
    static UINT yMaxScroll = 0;     // maximum vertical scroll value

    static int clientWidth = 0;
    static int clientHeight = 0;

    switch (uMsg)
    {
    case WM_CREATE: {

        // 윈도우 사이즈 초기화
        AdjustWindowRect(&g_mainWndSize, WS_OVERLAPPEDWINDOW, false);
        SetWindowPos(hwnd, NULL, WINDOW_LEFT, WINDOW_TOP, g_mainWndSize.right - g_mainWndSize.left,
                     g_mainWndSize.bottom - g_mainWndSize.top, 0);
        GetClientRect(hwnd, &g_clientSize);

        clientWidth = g_clientSize.right - g_clientSize.left;  // ignore scroll bar size 17 pixel
        clientHeight = g_clientSize.bottom - g_clientSize.top; // ignore scroll bar size 17 pixel

        // cutImage class 초기화
        cutImage.Initialize(hwnd);

        // scroll 범위 초기화
        xMaxScroll = cutImage.m_imageWidth - clientWidth;
        yMaxScroll = cutImage.m_imageHeight - clientHeight;

        SetScrollRange(hwnd, SB_HORZ, 0, xMaxScroll, true);
        SetScrollRange(hwnd, SB_VERT, 0, yMaxScroll, true);

        // 다이얼로그 초기화
        g_hDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DlgProc);
        ShowWindow(g_hDlg, SW_SHOW);
        SetWindowPos(g_hDlg, NULL, WINDOW_LEFT + g_mainWndSize.right - g_mainWndSize.left - WINDOW_PADDING, WINDOW_LEFT,
                     300, g_mainWndSize.bottom - g_mainWndSize.top, 0);
    }
    break;
    case WM_COMMAND: {
    }
    break;
    case WM_SIZE: {
        g_width = LOWORD(lParam);  // Macro to get the low-order word.
        g_height = HIWORD(lParam); // Macro to get the high-order word.

        // 윈도우 사이즈 고정
        SetWindowPos(hwnd, NULL, 100, 100, g_mainWndSize.right - g_mainWndSize.left,
                     g_mainWndSize.bottom - g_mainWndSize.top, 0);
        SetWindowPos(g_hDlg, NULL, WINDOW_LEFT + g_mainWndSize.right - g_mainWndSize.left - WINDOW_PADDING, WINDOW_LEFT,
                     300, g_mainWndSize.bottom - g_mainWndSize.top, 0);
    }
    break;
    case WM_MOVE: {
        // 윈도우 이동 시 다이얼로그와 함께 이동
        cout << LOWORD(lParam) << " " << HIWORD(lParam) << endl;
        SetWindowPos(hwnd, NULL, 100, 100, g_mainWndSize.right - g_mainWndSize.left,
                     g_mainWndSize.bottom - g_mainWndSize.top, 0);
    }
    break;
    case WM_HSCROLL: {
        // 스크롤을 움직이면 기존 박스는 없어진다.
        boxState = NONE;

        switch (LOWORD(wParam))
        {
        // User clicked the scroll bar shaft left of the scroll box.
        case SB_LINELEFT:
            xCurrentScroll = max(0, xCurrentScroll - 1);
            break;

        // User clicked the scroll bar shaft right of the scroll box.
        case SB_LINERIGHT:
            xCurrentScroll = min(xCurrentScroll + 1, xMaxScroll);
            break;

        // User clicked the left arrow.
        case SB_PAGELEFT:
            xCurrentScroll = max(0, xCurrentScroll - 10);
            break;

        // User clicked the right arrow.
        case SB_PAGERIGHT:
            xCurrentScroll = min(xCurrentScroll + 10, xMaxScroll);
            break;

        // User dragged the scroll box.
        case SB_THUMBTRACK:
            xCurrentScroll = HIWORD(wParam);
            break;
        }

        cout << xCurrentScroll << endl;

        SetScrollPos(hwnd, SB_HORZ, xCurrentScroll, true);
        InvalidateRect(hwnd, NULL, true);
    }
    break;
    case WM_VSCROLL: {
        // 스크롤을 움직이면 기존 박스는 없어진다.
        boxState = NONE;

        switch (LOWORD(wParam))
        {
        // User clicked the scroll bar shaft left of the scroll box.
        case SB_LINEUP:
            yCurrentScroll = max(0, yCurrentScroll - 1);
            break;

        // User clicked the scroll bar shaft right of the scroll box.
        case SB_LINEDOWN:
            yCurrentScroll = min(yCurrentScroll + 1, yMaxScroll);
            break;

        // User clicked the left arrow.
        case SB_PAGEUP:
            yCurrentScroll = max(0, yCurrentScroll - 10);
            break;

        // User clicked the right arrow.
        case SB_PAGEDOWN:
            yCurrentScroll = min(yCurrentScroll + 10, yMaxScroll);
            break;

        // User dragged the scroll box.
        case SB_THUMBTRACK:
            yCurrentScroll = HIWORD(wParam);
            break;
        }

        SetScrollPos(hwnd, SB_VERT, yCurrentScroll, true);
        InvalidateRect(hwnd, NULL, true);
    }
    break;
    case WM_MOUSEMOVE: {
        if (boxState == DRAG)
        {
            // 마우스의 RB 좌표
            newPos.x = LOWORD(lParam);
            newPos.y = HIWORD(lParam);
            SetRect(&box, oldPos.x, oldPos.y, newPos.x, newPos.y);
        }
        InvalidateRect(hwnd, NULL, true);
    }
    break;
    case WM_LBUTTONDOWN: {
        if (boxState != DRAG)
        {
            boxState = DRAG;
            // 마우스의 LT 좌표
            oldPos.x = LOWORD(lParam);
            oldPos.y = HIWORD(lParam);

            newPos.x = oldPos.x;
            newPos.y = newPos.y;

            cout << oldPos.x << " " << oldPos.y << endl;

            InvalidateRect(hwnd, NULL, true);
        }
    }
    break;
    case WM_LBUTTONUP: {
        if (boxState == DRAG)
        {
            boxState = SELECT;
            // 마우스의 RB 좌표
            newPos.x = LOWORD(lParam);
            newPos.y = HIWORD(lParam);
            // 자르기 좌표 찾기
            optBox = cutImage.FindCutPosition(oldPos.x, oldPos.y, newPos.x, newPos.y);
            // List Control 에 item 삽입
            LVCOLUMN col = {};
            LVITEM lv = {};
            int idx = ListView_GetItemCount(g_hListCtrl);
            lv.iItem = idx;
            lv.mask = LVIF_TEXT;
            lv.stateMask = 0;
            lv.iSubItem = 0;
            wchar_t item[5];
            wsprintf(item, L"%d", optBox.left);
            lv.pszText = item;
            ListView_InsertItem(g_hListCtrl, &lv);
            wsprintf(item, L"%d", optBox.top);
            ListView_SetItemText(g_hListCtrl, idx, 1, item);
            wsprintf(item, L"%d", optBox.right);
            ListView_SetItemText(g_hListCtrl, idx, 2, item);
            wsprintf(item, L"%d", optBox.bottom);
            ListView_SetItemText(g_hListCtrl, idx, 3, item);
        }
        InvalidateRect(hwnd, NULL, true);
    }
    break;
    case WM_RBUTTONDOWN: {
        {
            int xMousePos = LOWORD(lParam);
            int yMousePos = HIWORD(lParam);

            unsigned char r, g, b;

            cutImage.GetPixelColor(xMousePos, yMousePos, r, g, b);

            SetDlgItemInt(g_hDlg, IDC_EDIT4, int(r), true);
            SetDlgItemInt(g_hDlg, IDC_EDIT5, int(g), true);
            SetDlgItemInt(g_hDlg, IDC_EDIT6, int(b), true);
        }
    }
    break;
    case WM_PAINT: {
        PAINTSTRUCT ps;

        HDC hdc = BeginPaint(hwnd, &ps);

        // 화면 Clear
        Rectangle(cutImage.m_hdc, 0, 0, 1280, 780);

        // Image Render
        cutImage.Render(xCurrentScroll, yCurrentScroll);

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

        BitBlt(hdc, 0, 0, 1263, 763, cutImage.m_hdc, 0, 0, SRCCOPY);

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_CLOSE: {
        DestroyWindow(hwnd);
    }
    break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Box(HDC hdc, RECT rect)
{
    GDIObject obj(hdc);
    obj.ObjBrush(HOLLOW_BRUSH);

    Rectangle(hdc, rect.left - 1, rect.top - 1, rect.right + 2, rect.bottom + 2);
}

void ScrollProcess(HWND hwnd, int imageWidth, int imageHeight, int &xCurrentScroll, int &yCurrentScroll)
{
    RECT clientSize = {};
    GetClientRect(hwnd, &clientSize);
    int clientWidth = clientSize.right - clientSize.left;
    int clientHeight = clientSize.bottom - clientSize.top;
}

LRESULT CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG: {
        wchar_t colName[][3] = {L"x1", L"y1", L"x2", L"y2"};
        const int colWidth[] = {50, 50, 50, 50};

        g_hListCtrl = GetDlgItem(hwnd, IDC_LIST3);
        LVCOLUMN col = {};
        LVITEM li = {};

        col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        col.fmt = LVCFMT_LEFT;
        for (int i = 0; i < 4; ++i)
        {
            col.cx = colWidth[i];
            col.pszText = colName[i];
            ListView_InsertColumn(g_hListCtrl, i, &col);
        }
        ListView_SetItemCount(g_hListCtrl, 100000000);
    }
    break;
    case WM_COMMAND: {
        int wmid = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch (wmid)
        {
        case IDC_BUTTON1: {
            int idx = ListView_GetNextItem(g_hListCtrl, -1, LVNI_ALL | LVNI_SELECTED);
            if (idx != -1)
            {
                // cutImage 데이터에서도 삭제
                ListView_DeleteItem(g_hListCtrl, idx);
            }
        }
        break;
        }
    }
    break;
    case WM_NOTIFY: {
        LPNMHDR hdr = {};
        LPNMLISTVIEW nlv = {};
        LPNMITEMACTIVATE nia = {};
        hdr = (LPNMHDR)lParam;
        nlv = (LPNMLISTVIEW)lParam;

        // 현재 윈도우가 리스트컨트롤이라면
        if (hdr->hwndFrom == g_hListCtrl)
        {
            switch (hdr->code)
            {
            // 아이템이 선택됐을 때
            case LVN_ITEMCHANGED:
                if (nlv->uChanged == LVIF_STATE && nlv->uNewState == (LVIS_SELECTED | LVIS_FOCUSED))
                {
                    wchar_t x1[5] = L"0";
                    wchar_t y1[5] = L"0";
                    wchar_t x2[5] = L"0";
                    wchar_t y2[5] = L"0";

                    // 3번째 인자(숫자)에 따라 클릭한 Column의 정보를 얻는다.
                    ListView_GetItemText(g_hListCtrl, nlv->iItem, 0, x1, 5);
                    ListView_GetItemText(g_hListCtrl, nlv->iItem, 1, y1, 5);
                    ListView_GetItemText(g_hListCtrl, nlv->iItem, 2, x2, 5);
                    ListView_GetItemText(g_hListCtrl, nlv->iItem, 2, y2, 5);

                    SetDlgItemText(hwnd, IDC_EDIT7, x1);
                    SetDlgItemText(hwnd, IDC_EDIT8, y1);
                    SetDlgItemText(hwnd, IDC_EDIT9, x2);
                    SetDlgItemText(hwnd, IDC_EDIT10, y2);
                }
                break;
            }
        }
    }
    break;
    case WM_MOVE: {

        SetWindowPos(g_hDlg, NULL, WINDOW_LEFT + g_mainWndSize.right - g_mainWndSize.left - WINDOW_PADDING, WINDOW_LEFT,
                     300, g_mainWndSize.bottom - g_mainWndSize.top, 0);
    }
    break;
    case WM_CLOSE: {
        DestroyWindow(g_hDlg);
    }
    break;
    }
    return FALSE;
}