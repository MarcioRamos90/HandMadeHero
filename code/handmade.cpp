#include "handmade.h"

internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, game_state *GameState)
{ 
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/GameState->ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0;
            SampleIndex < (int) SoundBuffer->SampleCount;
            ++SampleIndex)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
    }
}

internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, game_state *GameState )
{
    // TODO(casey): Let's see what the optimizer does

    uint8 *Row = (uint8 *)Buffer->Memory;    
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0;
            X < Buffer->Width;
            ++X)
        {
            uint8 Blue = (uint8)(X + GameState->BlueOffset);
            uint8 Green = (uint8)(Y + GameState->GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }
        
        Row += Buffer->Pitch;
    }
}

internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer)
{
    assert(sizeof(game_state) <= Memory->PermanentStorageSize);

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        const char *Filename = __FILE__;

        // debug_read_file_result ResultRead = DEBUGPlatformReadEntireFile(Filename);
        // if (ResultRead.Content)
        // {
        //     DEBUGPlatformWriteEntireFile((const char *) ".\\test.out", ResultRead.ContentSize, ResultRead.Content);
        //     DEBUGPlatformFreeFileMemory(ResultRead.Content);
        // }

        GameState->ToneHz = 256;

        // NOTE: it should be here or in the platform?
        Memory->IsInitialized = true;
    }    


    game_controller_input *Input0 = &Input->Controllers[0];    
    if(Input0->IsAnalog)
    {
        // NOTE(casey): Use analog movement tuning
        GameState->BlueOffset += (int)(4.0f*(Input0->EndX));
        GameState->ToneHz = 256 + (int)(128.0f*(Input0->EndY));
    }
    else
    {
        // NOTE(casey): Use digital movement tuning
    }

    // Input.AButtonEndedDown;
    // Input.AButtonHalfTransitionCount;
    if(Input0->Down.EndedDown)
    {
        GameState->GreenOffset += 1;
    }


    // TODO: Allow sample offsets here for more rubust platform options
    GameOutputSound(SoundBuffer, GameState);
    RenderWeirdGradient(Buffer, GameState);
}
