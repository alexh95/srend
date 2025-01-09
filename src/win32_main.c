#include <windows.h>

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
)
{
    WNDCLASSEXA WindowClass = {0};
    WindowClass.lpfnWndProc = 0;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = "RendererWindowClass";

    RegisterClassExA(&WindowClass);

    // CreateWindowEx()

    return 0;
}
