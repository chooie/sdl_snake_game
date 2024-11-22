#include "audio.h"
#include <stdio.h>

bool32 audio_init(Audio_Context* ctx)
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        fprintf(stderr, "SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return false;
    }

    // Load audio files
    ctx->background_music = Mix_LoadMUS("music/mixkit-feast-from-the-east.mp3");
    if (!ctx->background_music)
    {
        fprintf(stderr, "Failed to load background music! Mix_Error: %s\n", Mix_GetError());
    }

    ctx->effect_beep = Mix_LoadWAV("sounds/beep.wav");
    if (!ctx->effect_beep)
    {
        fprintf(stderr, "Failed to load beep sound effect! Mix_Error: %s\n", Mix_GetError());
    }

    ctx->effect_beep_2 = Mix_LoadWAV("sounds/beep-2.mp3");

    if (!ctx->effect_beep_2)
    {
        fprintf(stderr, "Failed to load beep 2 sound effect! Mix_Error: %s\n", Mix_GetError());
    }

    ctx->effect_boom = Mix_LoadWAV("sounds/boom.mp3");

    if (!ctx->effect_beep_2)
    {
        fprintf(stderr, "Failed to load boom sound effect! Mix_Error: %s\n", Mix_GetError());
    }

    return true;
}

void audio_cleanup(Audio_Context* ctx)
{
    if (ctx->background_music)
    {
        Mix_FreeMusic(ctx->background_music);
    }
    if (ctx->effect_beep)
    {
        Mix_FreeChunk(ctx->effect_beep);
    }
    if (ctx->effect_beep_2)
    {
        Mix_FreeChunk(ctx->effect_beep_2);
    }
    Mix_CloseAudio();
}

void play_music(Audio_Context* ctx)
{
    if (ctx->background_music)
    {
        Mix_PlayMusic(ctx->background_music, -1);
    }
}

void stop_music(void)
{
    Mix_HaltMusic();
}

// Volume given as percent
void set_music_volume(real32 volume)
{
    real32 percent = volume / 100;
    Mix_VolumeMusic(MIX_MAX_VOLUME * percent);
}

void play_sound_effect(Mix_Chunk* effect)
{
    if (effect)
    {
        Mix_PlayChannel(-1, effect, 0);
    }
}
