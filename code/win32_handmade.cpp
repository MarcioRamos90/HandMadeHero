#include <Windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_oofscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesByPixel;
};

// Todo: This is a global for now.
global_variable bool GlobalRunning;
win32_oofscreen_buffer GlobalBuffer = {0};

struct win32_window_dimension
{
    int Width;
    int Height;
};


win32_window_dimension
GetWindowDimention(HWND Window) {
    win32_window_dimension dimention = {};
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    dimention.Width = ClientRect.right - ClientRect.left;
    dimention.Height = ClientRect.bottom - ClientRect.top;
    return dimention;
}

internal void
RenderWeirdGredient(win32_oofscreen_buffer Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 * )Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer.Width; ++X)
        {

            uint8 Color = (uint8)255;
            if (X == 10){// / 2 && X <  Buffer.Width + Buffer.Width/2) {
                // uint8 Blue = (uint8)(X) * 2;
                // uint8 Green = (uint8)(Y) * 3;

                
                //    *Pixel++ = ((Green << 8) | Blue);
                *Pixel++ = Color << 8;
            }
        }

        Row += Buffer.Pitch;
    }
}

internal void 
Win32ResizedDIBSection(win32_oofscreen_buffer *Buffer, int Width, int Height)
{
    GlobalBuffer.Width = Width;
    GlobalBuffer.Height = Height;
    
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->BytesByPixel = 4;
    
    // Note: When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int MemorySize =  (Buffer->Width*Buffer->Height)*Buffer->BytesByPixel;
    Buffer->Memory = VirtualAlloc(0, MemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO: Porblaby clear this to black 
    Buffer->Pitch = Buffer->Width*Buffer->BytesByPixel;

}

internal void 
Win32DisplayBufferInWindow(win32_oofscreen_buffer Buffer, HDC DeviceContext, int Width, int Height, int X, int Y)
{
    int WindowWidth = Width;
    int WindowHeight = Height;
    StretchDIBits(
        DeviceContext,
        /*
        X,Y,Width,Height,
        X,Y,Width,Height,
        */
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer.Width, Buffer.Height,
        Buffer.Memory,
        &Buffer.Info,
        DIB_RGB_COLORS, SRCCOPY);
}

LRESULT Win32MainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
)
{
    LRESULT Result = 0;
    switch (Message)
    {
        case WM_SIZE:
        {
            break;
        }
        case WM_DESTROY:
        {
            GlobalRunning = false; // TODO: Handle this with message to the user?
            break;
        }
        case WM_CLOSE:
        {
            GlobalRunning = false;// TODO: Handle this as an error - recreate window?
            break;
        }
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringW(L"WM_ACTIVATEAPP\n");
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            
            win32_window_dimension Dimention = GetWindowDimention(Window);

            Win32DisplayBufferInWindow(GlobalBuffer, DeviceContext, Dimention.Width, Dimention.Height, X, Y);
            EndPaint(Window, &Paint);
            break;
        }
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
            break;
        }
    }
    return Result;
}

int CALLBACK 
WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstace,
    LPSTR ComandLine,
    int ShowCode
) {
    WNDCLASSW WindowClass = {};

    Win32ResizedDIBSection(&GlobalBuffer, 1280, 720);

    WindowClass.style = CS_OWNDC|CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = L"HandmadeHeroWidnowClass";

    if (RegisterClassW(&WindowClass)) {
        HWND Window =
            CreateWindowExW(
                0, 
                (LPWSTR) WindowClass.lpszClassName,
                L"Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0
            );
        if (Window) {
            int XOffset = 0;
            int YOffset = 0;

            GlobalRunning = true;
            while(GlobalRunning)
            {
                
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {

                    if (Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                RenderWeirdGredient(GlobalBuffer, XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimention = GetWindowDimention(Window);

                Win32DisplayBufferInWindow(GlobalBuffer, DeviceContext, Dimention.Width, Dimention.Height, 0, 0);
                ReleaseDC(Window, DeviceContext);

                ++XOffset;
                ++YOffset;
            }
        } else {
            // Todo: logging
        }
    } else {
        // Todo: logging
    }

    return 0;
}