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

// Todo: This is a global for now.
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesByPixel = 4;

internal void
RenderWeirdGredient(int XOffset, int YOffset)
{
    int Width = BitmapWidth;
    int Heigth = BitmapHeight;

    int Pitch = Width*BytesByPixel;
    uint8 *Row = (uint8 * )BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < BitmapWidth; ++X)
        {
            uint8 Blue = (uint8)(X + XOffset);
            uint8 Green = (uint8)(Y + YOffset);

           *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Pitch;
    }
}

internal void 
Win32ResizedDIBSection(int Width, int Height)
{

    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize =  (BitmapWidth*BitmapHeight)*BytesByPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO: Porblaby clear this to black 
}

internal void 
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(
        DeviceContext,
        /*
        X,Y,Width,Height,
        X,Y,Width,Height,
        */
        0, 0, BitmapWidth, BitmapHeight,
        0, 0, WindowWidth, WindowHeight,
        BitmapMemory,
        &BitmapInfo,
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
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;
            Win32ResizedDIBSection(Width, Height);
            break;
        }
        case WM_DESTROY:
        {
            Running = false; // TODO: Handle this with message to the user?
            break;
        }
        case WM_CLOSE:
        {
            Running = false;// TODO: Handle this as an error - recreate window?
            break;
        }
        case WM_ACTIVATEAPP:
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
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
            
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);

            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
            break;
        }
        default:
        {
            // OutputDebugString("default\n");
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
    WNDCLASSA WindowClass = {};

    // TODO(marcio): check if CS_OWNDC|CS_HREDRAW|CS_VREDRAW still matter 
    // WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;

    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWidnowClass";

    if (RegisterClass(&WindowClass)) {
        HWND Window =
            CreateWindowEx(
                0, 
                WindowClass.lpszClassName,
                "Handmade Hero",
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

            Running = true;
            while(Running)
            {
                
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {

                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                RenderWeirdGredient(XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                ++XOffset;
            }
        } else {
            // Todo: logging
        }
    } else {
        // Todo: logging
    }

    return 0;
}