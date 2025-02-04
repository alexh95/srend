#include <windows.h>
#include <stdint.h>

#include "platform.h"

#include "srend.h"

static state GlobalState = {0};

static BOOL Running = TRUE;
static BITMAPINFO FrameBitmapInfo = {0};
static HBITMAP FrameBitmap = 0;
static HDC FrameDeviceContext = 0;

LRESULT WindowProc(
    HWND WindowHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
)
{
    switch (Message)
    {
        case WM_QUIT:
        case WM_DESTROY:
        {
            Running = FALSE;
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint = {0};
            HDC DeviceContext = {0};
            DeviceContext = BeginPaint(WindowHandle, &Paint);
            BitBlt(
                DeviceContext,
                Paint.rcPaint.left,
                Paint.rcPaint.top,
                Paint.rcPaint.right - Paint.rcPaint.left,
                Paint.rcPaint.bottom - Paint.rcPaint.top,
                FrameDeviceContext,
                Paint.rcPaint.left,
                Paint.rcPaint.top,
                SRCCOPY
            );
            EndPaint(WindowHandle, &Paint);
        } break;
        case WM_SIZE:
        {
            FrameBitmapInfo.bmiHeader.biWidth = LOWORD(LParam);
            FrameBitmapInfo.bmiHeader.biHeight = HIWORD(LParam);
            if (FrameBitmap)
            {
                DeleteObject(FrameBitmap);
            }
            FrameBitmap = CreateDIBSection(0, &FrameBitmapInfo, DIB_RGB_COLORS, (void **)&GlobalState.Frame.Pixels, 0, 0);
            SelectObject(FrameDeviceContext, FrameBitmap);
            GlobalState.Frame.Width = FrameBitmapInfo.bmiHeader.biWidth;
            GlobalState.Frame.Height = FrameBitmapInfo.bmiHeader.biHeight;
        } break;
        default:
        {
            return DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }
    
    return 0;
}

int WinMain(
    HINSTANCE Instance,
    HINSTANCE PreviousInstance,
    LPSTR CommandLineArgs,
    int CommandShow
)
{
    HMODULE RendererDll = LoadLibraryA("srend.dll");
    update_and_draw *UpdateAndDraw = (update_and_draw *)GetProcAddress(RendererDll, "UpdateAndDraw");

    WNDCLASSEXA WindowClass = {0};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "RendererWindowClass";

    RegisterClassExA(&WindowClass);

    FrameBitmapInfo.bmiHeader.biSize = sizeof(FrameBitmapInfo.bmiHeader);
    FrameBitmapInfo.bmiHeader.biPlanes = 1;
    FrameBitmapInfo.bmiHeader.biBitCount = 32;
    FrameBitmapInfo.bmiHeader.biCompression = BI_RGB;
    FrameDeviceContext = CreateCompatibleDC(0);

    HWND WindowHandle = CreateWindowExA(
        WS_EX_OVERLAPPEDWINDOW,
        "RendererWindowClass",
        "Software Renderer",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        480,
        360,
        0,
        0,
        Instance,
        0
    );

    ShowWindow(WindowHandle, CommandShow);

    while (Running)
    {        
        MSG Message = {0};
        while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        UpdateAndDraw(&GlobalState);

        InvalidateRect(WindowHandle, 0, FALSE);
        UpdateWindow(WindowHandle);
    }

    return 0;
}
