/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  TODO(casey):  THIS IS NOT A FINAL PLATFORM LAYER!!!

  - Saved game locations
  - Getting a handle to our own executable file
  - Asset loading path
  - Threading (launch a thread)
  - Raw Input (support for multiple keyboards)
  - Sleep/timeBeginPeriod
  - ClipCursor() (for multimonitor support)
  - Fullscreen support
  - WM_SETCURSOR (control cursor visibility)
  - QueryCancelAutoplay
  - WM_ACTIVATEAPP (for when we are not the active application)
  - Blit speed improvements (BitBlt)
  - Hardware acceleration (OpenGL or Direct3D or BOTH??)
  - GetKeyboardLayout (for French keyboards, international WASD support)

  Just a partial list of stuff!!
*/

#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>

#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#include "handmade.cpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

// TODO(casey): This is a global for now.
global_variable bool32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name([[maybe_unused]] DWORD dwUserIndex, [[maybe_unused]] XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name([[maybe_unused]] DWORD dwUserIndex, [[maybe_unused]] XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal debug_read_file_result 
DEBUGPlatformReadEntireFile(const char *Filename) {    
    debug_read_file_result Result = debug_read_file_result{};
    HANDLE FileHandle = CreateFileA(
        Filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        GetFileSizeEx(FileHandle, &FileSize);
        if (GetLastError() != INVALID_FILE_SIZE)
        {
            uint32 FileSize32 = SafeTruncateUInt64((uint64)FileSize.QuadPart);
            Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            DWORD BytesRead;
            if (Result.Content)
            {
                if (ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, NULL) && (FileSize32 == BytesRead))
                {
                    //NOTE: File Read Successfully
                    Result.ContentSize = FileSize32;
                }
                else
                {
                    DEBUGPlatformFreeFileMemory(Result.Content);
                    Result.Content = 0;
                }

            }
            else
            {
            //TODO: Log error
            }
            CloseHandle(FileHandle);
        }
        else
        {
            //TODO: Log error
        }
    }
    else
    {
        //TODO: Log error
    }

    return Result;
}

internal void
DEBUGPlatformFreeFileMemory(void *Memory) {
    VirtualFree(
        Memory,
        0,
        MEM_RELEASE
    );
}

internal bool32
DEBUGPlatformWriteEntireFile(const char *Filename, uint32 MemorySize, void *Memory){
    bool32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, NULL, NULL, 
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, NULL) == true)
        {
            //NOTE: File Read Successfully
            Result = BytesWritten == MemorySize;
        }
        else
        {
            //TODO: Log error
        }

        CloseHandle(FileHandle);

    }
    else
    {
        //TODO: Log error
    }

    
    return Result;
}

internal void
Win32LoadXInput(void)    
{
    // TODO(casey): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}

        // TODO(casey): Diagnostic

    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(casey): Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        // NOTE(casey): Get a DirectSound object! - cooperative
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        // TODO(casey): Double-check that this works on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = (DWORD)SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // NOTE(casey): "Create" a primary buffer
                // TODO(casey): DSBCAPS_GLOBALFOCUS?
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        // NOTE(casey): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(casey): Diagnostic
                    }
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
            else
            {
                // TODO(casey): Diagnostic
            }

            // TODO(casey): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = (DWORD) BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO(casey): Diagnostic
        }
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}


internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;

    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    // NOTE(casey): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, (SIZE_T)BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;

    // TODO(casey): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // TODO(casey): Aspect ratio correction
    // TODO(casey): Play with stretch modes
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{       
    LRESULT Result = 0;

    switch(Message)
    {
        case WM_CLOSE:
        {
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            // TODO(casey): Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
           
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                       Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
//            OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, (DWORD)SoundOutput->SecondaryBufferSize,
                    &Region1, &Region1Size,
                    &Region2, &Region2Size,
                    0)))
    {
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;
                    ByteIndex < Region1Size;
                    ++ByteIndex)
            {
                *DestSample++ = 0;
            }
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;
                    ByteIndex < Region2Size;
                    ++ByteIndex)
            {
                *DestSample++ = 0;
            }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
    // TODO(casey): More strenuous test!
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                    &Region1, &Region1Size,
                    &Region2, &Region2Size,
                    0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid

        // TODO(casey): Collapse these two loops
        DWORD Region1SampleCount = (DWORD)Region1Size/(DWORD)SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0;
                SampleIndex < Region1SampleCount;
                ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = (DWORD)Region2Size/(DWORD)SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD SampleIndex = 0;
                SampleIndex < Region2SampleCount;
                ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
ProcessXInputDigitalButton(DWORD XinputButtonState, game_button_state *OldState, game_button_state *NewState, DWORD ButtonBit) {
    NewState->EndedDown = (XinputButtonState & ButtonBit) == ButtonBit;
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal void
Win32ProcessKeyboardMessage(game_button_state *KeyBoardController, bool32 IsDown) {
    KeyBoardController->EndedDown = IsDown;
    ++KeyBoardController->HalfTransitionCount;
}

internal void
Win32ProcessPendingMessages(game_controller_input *KeyboardController)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                    }
                    else if(VKCode == 'A')
                    {
                    }
                    else if(VKCode == 'S')
                    {
                    }
                    else if(VKCode == 'D')
                    {
                    }
                    else if(VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    }
                    else if(VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    }
                    else if(VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Up, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Left, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Down, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Right, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        GlobalRunning = false;
                    }
                    else if(VKCode == VK_SPACE)
                    {
                    }
                }

                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                if((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = false;
                }
            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    (void)ShowCode;
    (void)CommandLine;
    (void)PrevInstance;
    (void)Instance;
#if 0
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
#endif
    Win32LoadXInput();
    
    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
//    WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
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
                0);
        if(Window)
        {
            // NOTE(casey): Since we specified CS_OWNDC, we can just
            // get one device context and use it forever because we
            // are not sharing it with anyone.
            HDC DeviceContext = GetDC(Window);

            win32_sound_output SoundOutput = {};

            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            
            GlobalRunning = true;
            int16 *Samples = (int16 *) VirtualAlloc(0, (SIZE_T)SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
    LPVOID BaseAddres = (LPVOID) Terabytes((uint64)2);
#else
    LPVOID BaseAddres = 0;
#endif
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes((uint64)64);
            GameMemory.TransientStorageSize = Gigabytes((uint64)4);
            
            uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize; 
            GameMemory.PermanentStorage = VirtualAlloc(BaseAddres, (SIZE_T)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);                
            GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

            if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {

                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];
#if 0
                LARGE_INTEGER LastCounter;
                QueryPerformanceCounter(&LastCounter);
                uint64 LastCycleCount = __rdtsc();
#endif                
                while(GlobalRunning)
                {                    
                    // TODO: Zeroin Macro
                    game_controller_input *KeyBoardController = &NewInput->Controllers[0];
                    game_controller_input ZeroController = {};
                    *KeyBoardController = ZeroController;

                    Win32ProcessPendingMessages(KeyBoardController);

                    // TODO(casey): Should we poll this more frequently?
                    DWORD MaxControllerCount = XUSER_MAX_COUNT;
                    if (MaxControllerCount > ArrayCount(NewInput->Controllers))
                    {
                        MaxControllerCount = ArrayCount(NewInput->Controllers);
                    }
                    for (DWORD ControllerIndex = 0;
                        ControllerIndex < MaxControllerCount;
                        ++ControllerIndex)
                    {
                        XINPUT_STATE ControllerState;
                        if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                        {
                            game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
                            game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];
                            // NOTE(casey): This controller is plugged in
                            // TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
                            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                            NewController->IsAnalog = true;
                            NewController->StartX = OldController->EndX;
                            NewController->StartY = OldController->EndY;

                            // TODO(casey): We will do deadzone handling later using
                            // XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
                            // XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 

                            // TODO: Min/Max Macros!!!
                            real32 X;
                            if(Pad->sThumbLX < 0)
                            {
                                X = (real32)Pad->sThumbLX / 32768.0f;
                            }
                            else
                            {
                                X = (real32)Pad->sThumbLX / 32767.0f;
                            }
                            NewController->MinX = NewController->MaxX = NewController->EndX = X;

                            real32 Y;
                            if(Pad->sThumbLY < 0)
                            {
                                Y = (real32)Pad->sThumbLY / 32768.0f;
                            }
                            else
                            {
                                Y = (real32)Pad->sThumbLY / 32767.0f;
                            }
                            NewController->MinY = NewController->MaxY = NewController->EndY = Y;

                            ProcessXInputDigitalButton(Pad->wButtons, &OldController->Down, &NewController->Down, XINPUT_GAMEPAD_A);
                            ProcessXInputDigitalButton(Pad->wButtons, &OldController->Right, &NewController->Right, XINPUT_GAMEPAD_B);
                            ProcessXInputDigitalButton(Pad->wButtons, &OldController->Left, &NewController->Left, XINPUT_GAMEPAD_X);
                            ProcessXInputDigitalButton(Pad->wButtons, &OldController->Up, &NewController->Up, XINPUT_GAMEPAD_Y);

                            ProcessXInputDigitalButton(Pad->wButtons, &OldController->LeftShoulder, &NewController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
                            ProcessXInputDigitalButton(Pad->wButtons, &OldController->RightShoulder, &NewController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);

                            // bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                            // bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        }
                        else
                        {
                            // NOTE(casey): The controller is not available
                        }
                    }

                    // NOTE(casey): DirectSound output test
                    DWORD ByteToLock = 0;
                    DWORD TargetCursor = 0;
                    DWORD BytesToWrite = 0;
                    DWORD PlayCursor = 0;
                    DWORD WriteCursor = 0;
                    bool32 SoundIsValid = false;
                    // Todo: Tighten up sound logic so that we know where we should be writing
                    // to and can anticipate the time spent in the game update.
                    if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                    {
                        ByteToLock = (uint32)((SoundOutput.RunningSampleIndex*(uint32)SoundOutput.BytesPerSample) %
                                            (uint32)SoundOutput.SecondaryBufferSize);
                        TargetCursor =
                            ((PlayCursor +
                            ((uint32)SoundOutput.LatencySampleCount*(uint32)SoundOutput.BytesPerSample)) %
                            (uint32)SoundOutput.SecondaryBufferSize);
                        if(ByteToLock > TargetCursor)
                        {
                            BytesToWrite = ((DWORD)SoundOutput.SecondaryBufferSize - ByteToLock);
                            BytesToWrite += TargetCursor;
                        }
                        else
                        {
                            BytesToWrite = TargetCursor - ByteToLock;
                        }
                        SoundIsValid = true;
                    }
                    game_sound_output_buffer SoundBuffer = {};
                    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                    SoundBuffer.SampleCount = BytesToWrite / (DWORD)SoundOutput.BytesPerSample;
                    SoundBuffer.Samples = Samples;

                    game_offscreen_buffer Buffer = {};
                    Buffer.Memory = GlobalBackbuffer.Memory;
                    Buffer.Width = GlobalBackbuffer.Width; 
                    Buffer.Height = GlobalBackbuffer.Height;
                    Buffer.Pitch = GlobalBackbuffer.Pitch; 

                    GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);


                    if(SoundIsValid)
                    {
                        Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                    }
                    
                    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                    Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                            Dimension.Width, Dimension.Height);



    #if 0
                    uint64 EndCycleCount = __rdtsc();
                    
                    LARGE_INTEGER EndCounter;
                    QueryPerformanceCounter(&EndCounter);
                    uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                    int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    real64 MSPerFrame = (((1000.0f*(real64)CounterElapsed) / (real64)PerfCountFrequency));
                    real64 FPS = (real64)PerfCountFrequency / (real64)CounterElapsed;
                    real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));
                    char Buffer[256];
                    sprintf(Buffer, "%.02fms/f,  %.02ff/s,  %.02fmc/f\n", MSPerFrame, FPS, MCPF);
                    OutputDebugStringA(Buffer);
                    LastCounter = EndCounter;
                    LastCycleCount = EndCycleCount;
    #endif
                    

                    game_input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                }
            }
            else
            {
                // TODO: Logging if the memory or sample does not allocate properly
            }
        }
        else
        {
            // TODO(casey): Logging
        }
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(0);
}

