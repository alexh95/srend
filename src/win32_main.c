#include <windows.h>

#include "platform.h"
#include "srend.h"

#define INITIAL_WIDTH (4 * 200)
#define INITIAL_HEIGHT (3 * 200)

static state GlobalState = {0};

static BOOL Running = TRUE;
static BITMAPINFO FrameBitmapInfo = {0};
static HBITMAP FrameBitmap = 0;
static HDC FrameDeviceContext = 0;

static LRESULT CALLBACK WindowProc(
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
            GlobalState.Frame.Width = FrameBitmapInfo.bmiHeader.biWidth;
            GlobalState.Frame.Height = FrameBitmapInfo.bmiHeader.biHeight;
            FrameBitmap = CreateDIBSection(0, &FrameBitmapInfo, DIB_RGB_COLORS, (void **)&GlobalState.Frame.Pixels, 0, 0);
            SelectObject(FrameDeviceContext, FrameBitmap);

            umm PreviousBufferSize = sizeof(u32) * (GlobalState.Depth.Width * GlobalState.Depth.Height);
            GlobalState.Depth.Width = GlobalState.Frame.Width;
            GlobalState.Depth.Height = GlobalState.Frame.Height;
            umm NewBufferSize = sizeof(f32) * (GlobalState.Depth.Width * GlobalState.Depth.Height);
            if (GlobalState.Depth.Values)
            {
                VirtualFree(GlobalState.Depth.Values, PreviousBufferSize, MEM_RELEASE);
            }
            GlobalState.Depth.Values = VirtualAlloc(0, NewBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            // BOOL AltKeyWasDown = LPARAM & (1 << 29);
            // BOOL ShiftKeyWasDown = GetKeyState(VK_SHIFT) & (1 << 15);
            
            BOOL WasDown = (LParam & (1 << 30)) != 0;
            BOOL IsDown = (LParam & (1 << 31)) == 0;
            if(WasDown != IsDown)
            {
                if (WParam == 'A')
                {
                    GlobalState.Input.Left = IsDown;
                }
                else if (WParam == 'D')
                {
                    GlobalState.Input.Right = IsDown;
                }
                else if (WParam == 'W')
                {
                    GlobalState.Input.Forward = IsDown;
                }
                else if (WParam == 'S')
                {
                    GlobalState.Input.Backward = IsDown;
                }
                else if (WParam == VK_SPACE)
                {
                    GlobalState.Input.Up = IsDown;
                }
                else if (WParam == 'C')
                {
                    GlobalState.Input.Down = IsDown;
                }
                else if (WParam == 'Q')
                {
                    GlobalState.Input.RollLeft = IsDown;
                }
                else if (WParam == 'E')
                {
                    GlobalState.Input.RollRight = IsDown;
                }
                else if (WParam == 'Z')
                {
                    GlobalState.Input.PitchUp = IsDown;
                }
                else if (WParam == 'X')
                {
                    GlobalState.Input.PitchDown = IsDown;
                }
                else if (WParam == 'R')
                {
                    GlobalState.Input.YawLeft = IsDown;
                }
                else if (WParam == 'T')
                {
                    GlobalState.Input.YawRight = IsDown;
                }
                else if (WParam == VK_LEFT)
                {
                    GlobalState.Input.Left = IsDown;
                }
                else if (WParam == VK_RIGHT)
                {
                    GlobalState.Input.Right = IsDown;
                }
                else if (WParam == VK_UP)
                {
                    GlobalState.Input.Forward = IsDown;
                }
                else if (WParam == VK_DOWN)
                {
                    GlobalState.Input.Backward = IsDown;
                }
            }
        } break;
        default:
        {
            return DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }
    
    return 0;
}

OPEN_AND_READ_FILE(Win32OpenAndReadFile)
{
    buffer Result = {0};

    HANDLE FileHandle = CreateFileA(
        FileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    DWORD LastError = GetLastError();
    if (LastError == ERROR_FILE_NOT_FOUND)
    {
        return Result;
    }

    LARGE_INTEGER FileSize = {0};
    GetFileSizeEx(FileHandle, &FileSize);

    c8 *FileData = VirtualAlloc(0, FileSize.QuadPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    ReadFile(FileHandle, FileData, FileSize.LowPart, 0, 0);
    CloseHandle(FileHandle);

    Result.Data = FileData;
    Result.Size = FileSize.QuadPart;

    return Result;
}

int WinMain(
    HINSTANCE Instance,
    HINSTANCE PreviousInstance,
    LPSTR CommandLineArgs,
    int CommandShow
)
{
    HMODULE RendererDll = LoadLibraryA("srend.dll");
    renderer_initialize *RendererInitialize = (renderer_initialize *)(void *)GetProcAddress(RendererDll, "RendererInitialize");
    renderer_update_and_draw *RendererUpdateAndDraw = (renderer_update_and_draw *)(void *)GetProcAddress(RendererDll, "RendererUpdateAndDraw");

    SetProcessDPIAware();

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

    GlobalState.Frame.Width = INITIAL_WIDTH;
    GlobalState.Frame.Height = INITIAL_HEIGHT;
    FrameBitmap = CreateDIBSection(0, &FrameBitmapInfo, DIB_RGB_COLORS, (void **)&GlobalState.Frame.Pixels, 0, 0);

    GlobalState.Depth.Width = INITIAL_WIDTH;
    GlobalState.Depth.Height = INITIAL_HEIGHT;
    umm BufferSize = sizeof(f32) * (GlobalState.Depth.Width * GlobalState.Depth.Height);
    GlobalState.Depth.Values = VirtualAlloc(0, BufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    RECT WindowRect = { 0, 0, INITIAL_WIDTH, INITIAL_HEIGHT };
    AdjustWindowRectEx(
        &WindowRect,
        WS_OVERLAPPEDWINDOW,
        FALSE,
        WS_EX_OVERLAPPEDWINDOW
    );

    HWND WindowHandle = CreateWindowExA(
        WS_EX_OVERLAPPEDWINDOW,
        "RendererWindowClass",
        "Software Renderer",
        WS_OVERLAPPEDWINDOW,
        1000,
        500,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        0,
        0,
        Instance,
        0
    );

    ShowWindow(WindowHandle, CommandShow);

    GlobalState.Arena.MaxSize = MegaBytes(16);
    GlobalState.Arena.Data = VirtualAlloc(0, GlobalState.Arena.MaxSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    GlobalState.Platform.OpenAndReadFile = Win32OpenAndReadFile;

    RendererInitialize(&GlobalState);
    while (Running)
    {
        MSG Message = {0};
        while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        RendererUpdateAndDraw(&GlobalState);

        InvalidateRect(WindowHandle, 0, FALSE);
        UpdateWindow(WindowHandle);
    }

    return 0;
}
