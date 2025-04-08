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

internal void GameUpdateAndRender(game_offscreen_buffer, int, int, game_sound_output_buffer *SoundBuffer, int ToneHz);
internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz);