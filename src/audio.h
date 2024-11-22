#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL_mixer.h>

// Audio_Context to store audio resources
typedef struct
{
    Mix_Music* background_music;
    Mix_Chunk* effect_beep;
    Mix_Chunk* effect_beep_2;
    Mix_Chunk* effect_boom;
} Audio_Context;

// Initialize the audio system
bool32 audio_init(Audio_Context* ctx);

// Cleanup the audio system
void audio_cleanup(Audio_Context* ctx);

// Play a music file
void play_music(Audio_Context* ctx);

// Stop the music
void stop_music(void);

void set_music_volume(real32 volume);

// Play a sound effect
void play_sound_effect(Mix_Chunk* effect);

#endif  // AUDIO_H