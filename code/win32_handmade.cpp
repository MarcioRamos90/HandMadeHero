#include <Windows.h>
#include <stdint.h>

#define internal_function static
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

internal_function void 
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

    int BytesByPixel = 4;
    int TotalPixels = (BitmapHeight*BitmapHeight) ;
    int BitmapMemorySize = TotalPixels*BytesByPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    int Pitch = Width*BytesByPixel;
    uint8 *Row = (uint8 * )BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint8 *Pixel = (uint8 *)Row;
        for (int X = 0; X < BitmapWidth; ++X)
        {
           *Pixel = 255;
           ++Pixel;
           *Pixel = 0;
           ++Pixel;
           *Pixel = 0;
           ++Pixel;
           *Pixel = 0;
           ++Pixel;
        }

        Row += Pitch;
    }
    
}

internal_function void 
Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;
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
            OutputDebugString("WM_SIZE\n");
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
            OutputDebugString("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
            break;
        }
    }
    return Result;
}

int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstace,
    LPSTR ComandLine,
    int ShowCode
) {
    OutputDebugString("init\n");

    LPCSTR classname = "HandmadeHeroWidnowClass";

    WNDCLASSA WindowClass = {0};

    // TODO(marcio): check if CS_OWNDC|CS_HREDRAW|CS_VREDRAW still matter 
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    // WindowClass.cbClsExtra = 0;
    // WindowClass.cbWndExtra = 0;
    // WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    // WindowClass.hIcon = LoadIcon(0, IDI_WINLOGO);
    // WindowClass.lpszMenuName = 0;
    // WindowClass.hbrBackground = 0;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = classname;

    int regis = RegisterClass(&WindowClass);
    if (regis) {
        HWND WindowHandle = CreateWindowEx(
            0, 
            classname,
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
        if (WindowHandle) {
            MSG Message;
            Running = true;
            while(Running)
            {
                BOOL MessageResult = GetMessage(&Message,0,0,0);
                if (MessageResult) {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } else {
                    Running = false;
                }
            }
        } else {
            // Todo: logging
        }
    } else {
        // Todo: logging
    }

    return 0;
}