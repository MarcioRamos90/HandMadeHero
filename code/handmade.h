#pragma once

#include <stdint.h>
#include <stdio.h>
#include <math.h>

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

#define Pi32 3.14159265359f

#define internal static 
#define local_persist static 
#define global_variable static

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;

    // int SamplesPerSecond;
    // int ToneHz;
    // int16 ToneVolume;
    // uint32 RunningSampleIndex;
    // int WavePeriod;
    // int BytesPerSample;
    // int SecondaryBufferSize;
    // real32 tSine;
    // int LatencySampleCount;
};

struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input
{
    bool32 IsAnalog;

    real32 StartX;
    real32 StartY;
    
    real32 MinX;
    real32 MinY;

    real32 MaxX;
    real32 MaxY;

    real32 EndX;
    real32 EndY;
    union {
        game_button_state Buttons[6];
        struct {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};

struct game_input
{
    game_controller_input Controllers[4];
};

struct game_state
{
    int ToneHz;
    int GreenOffset;
    int BlueOffset;
};

struct game_memory
{
    bool32 IsInitialized;
    uint64 PermanentStorageSize;
    void *PermanentStorage;
};

internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer, game_sound_output_buffer *SoundBuffer, int ToneHz);
internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz);