#include <Windows.h>
#include <stdint.h>
#include <Xinput.h>

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
typedef int32 bool32;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state); 

X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);

X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}

global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void
Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryW(L"xinput1_4.dll");
    if (!XInputLibrary)
    {
        HMODULE XInputLibrary = LoadLibraryW(L"xinput1_3.dll");
    }
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

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


internal win32_window_dimension
GetWindowDimention(HWND Window) {
    win32_window_dimension dimention = {};
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    dimention.Width = ClientRect.right - ClientRect.left;
    dimention.Height = ClientRect.bottom - ClientRect.top;
    return dimention;
}

internal void
RenderWeirdGredient(win32_oofscreen_buffer *Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 * )Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {

            uint8 Blue = (uint8)(X) + XOffset;
            uint8 Green = (uint8)(Y) + YOffset;

            
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer->Pitch;
    }
}

internal void 
Win32ResizedDIBSection(win32_oofscreen_buffer *Buffer, int Width, int Height)
{
    Buffer->Width = Width;
    Buffer->Height = Height;
    
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
Win32DisplayBufferInWindow(win32_oofscreen_buffer *Buffer, HDC DeviceContext, int Width, int Height, int X, int Y)
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
        0, 0, Buffer->Width, Buffer->Height,
        Buffer->Memory,
        &Buffer->Info,
        DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT Win32MainWindowCallback(
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
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = WParam;
            bool WasDown = (LParam & (1 << 30)) != 0;
            bool IsDown = (LParam & (1 << 31)) == 0;

            if (WasDown != IsDown)
            {
                if (VKCode == 'W')
                {

                } else if (VKCode == 'A')
                {

                } else if (VKCode == 'S')
                {

                } else if (VKCode == 'D')
                {

                } else if (VKCode == 'Q')
                {

                } else if (VKCode == 'E')
                {

                } else if (VKCode == VK_UP)
                {

                } else if (VKCode == VK_DOWN)
                {

                } else if (VKCode == VK_LEFT)
                {

                } else if (VKCode == VK_RIGHT)
                {

                } else if (VKCode == VK_ESCAPE)
                {

                } else if (VKCode == VK_SPACE)
                {
                    OutputDebugStringW(L"SPACE!!!");
                    if (IsDown)
                    {
                       OutputDebugStringW(L"IsDown!!!");
                    }
                    if (WasDown)
                    {
                       OutputDebugStringW(L"WasDown!!!");
                    }
                    OutputDebugStringW(L"\n");
                }
            }
            bool32 AltWasDown = (LParam & (1 << 29));
            if (VKCode == VK_F4 && AltWasDown)
            {
                GlobalRunning = false;
            }
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

            Win32DisplayBufferInWindow(&GlobalBuffer, DeviceContext, Dimention.Width, Dimention.Height, X, Y);
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
    Win32LoadXInput();

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

                for (DWORD ControllerIndex=0; ControllerIndex < MAX_TOUCH_COUNT; ++ControllerIndex) {

                }

                // Should we poll this more frequently
                for (DWORD CotnrollerIndex=0; CotnrollerIndex< XUSER_MAX_COUNT; CotnrollerIndex++ )
                {
                    XINPUT_STATE ControlerState;
                    // ZeroMemory( &ControlerState, sizeof(XINPUT_STATE) );

                    // Simply get the state of the controller from XInput.
                    if(XInputGetState_( CotnrollerIndex, &ControlerState ) == ERROR_SUCCESS )
                    {
                        // Controller is connected
                        // see if number increments too rapidelly
                        XINPUT_GAMEPAD *Pad = &ControlerState.Gamepad;
                        bool Up = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool Down = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool Left = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                        bool Right = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool Start = Pad->wButtons & XINPUT_GAMEPAD_START;
                        bool Back = Pad->wButtons & XINPUT_GAMEPAD_BACK;
                        bool LeftShoulder = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool RightShoulder = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                        bool A = Pad->wButtons & XINPUT_GAMEPAD_A;
                        bool B = Pad->wButtons & XINPUT_GAMEPAD_B;
                        bool X = Pad->wButtons & XINPUT_GAMEPAD_X;
                        bool Y = Pad->wButtons & XINPUT_GAMEPAD_Y;

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;
                        XOffset += StickX >> 13;
                        YOffset += StickY >> 13;
                    }
                    else
                    {
                        // Controller is not connected
                    }
                }

                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 60000;
                Vibration.wRightMotorSpeed = 60000;
                //XInputSetState(0, &Vibration);
                RenderWeirdGredient(&GlobalBuffer, XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimention = GetWindowDimention(Window);

                Win32DisplayBufferInWindow(&GlobalBuffer, DeviceContext, Dimention.Width, Dimention.Height, 0, 0);
                ReleaseDC(Window, DeviceContext);

            }
        } else {
            // Todo: logging
        }
    } else {
        // Todo: logging
    }

    return 0;
}