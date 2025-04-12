#pragma once

/*
    HANDMADE_INTERNAL:
        0: Build for public release
        1: Build for developer
    HANDMADE_SLOW:
        0: Not slow code allowed
        1: Slow code welcome
 */

#if HANDMADE_SLOW
// #define Assert(Expression) \
//     if(!(Expression)) {*(int *)0 = 0;}
#include <assert.h>
#else
#define assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO: Defines for maximum values
    assert(Value <= 0XFFFFFFFF);
    return (uint32) Value;
}

/**Note: Services that the platform layer provides for the game 
*/
#if HANDMADE_INTERNAL
    /**IMPORTANT:
        These are NOT for doing anything on the shipping game - they are
        blocking and the write doesn't protect against lost data!
     */
    struct debug_read_file_result
    {
        uint32 ContentSize;
        void* Content;
    };

    debug_read_file_result DEBUGPlatformReadEntireFile(const char *Filename);
    void DEBUGPlatformFreeFileMemory(void *Memory);
    bool32 DEBUGPlatformWriteEntireFile(const char *Filename, uint32 MemorySize, void *Memory);
#endif

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
    // TODO: Insert clock value here
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

    uint64 TransientStorageSize;
    void *TransientStorage;
};

internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer, game_sound_output_buffer *SoundBuffer, int ToneHz);
internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz);