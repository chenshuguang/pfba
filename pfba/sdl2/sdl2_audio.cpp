//
// Created by cpasjuste on 12/12/16.
//

#include <SDL2/SDL.h>
#include <burner.h>
#include <malloc.h>
#include "sdl2_audio.h"

static bool use_mutex = true;

static int buf_size;
static unsigned char *buffer_sdl;
static unsigned int buf_read_pos = 0;
static unsigned int buf_write_pos = 0;
static int buffered_bytes = 0;

static SDL_mutex *sound_mutex;
static SDL_cond *sound_cv;

static inline int calc_samples() {
    int s;
    // Find the value which is slighly bigger than nBurnSoundLen*2
    for (s = 512; s < (nBurnSoundLen * 2); s <<= 1);
    return s;
}

static void write_buffer(unsigned char *data, int len) {

    if (use_mutex) {
        SDL_LockMutex(sound_mutex);
    }

    for (int i = 0; i < len; i += 4) {
        if (!use_mutex) {
            if (buffered_bytes == buf_size) {
                return; // drop samples
            }
        } else {
            while (buffered_bytes == buf_size) {
                SDL_CondWait(sound_cv, sound_mutex);
            }
        }

        *(int *) ((char *) (buffer_sdl + buf_write_pos)) = *(int *) ((char *) (data + i));
        buf_write_pos = (buf_write_pos + 4) % buf_size;
        buffered_bytes += 4;
    }

    if (use_mutex) {
        SDL_CondSignal(sound_cv);
        SDL_UnlockMutex(sound_mutex);
    }
}

static void read_buffer(void *unused, unsigned char *data, int len) {

    if (use_mutex) {
        SDL_LockMutex(sound_mutex);
    }

    if (buffered_bytes >= len) {
        if (buf_read_pos + len <= buf_size) {
            memcpy(data, buffer_sdl + buf_read_pos, len);
        } else {
            int tail = buf_size - buf_read_pos;
            memcpy(data, buffer_sdl + buf_read_pos, tail);
            memcpy(data + tail, buffer_sdl, len - tail);
        }
        buf_read_pos = (buf_read_pos + len) % buf_size;
        buffered_bytes -= len;
    }

    if (use_mutex) {
        SDL_CondSignal(sound_cv);
        SDL_UnlockMutex(sound_mutex);
    }
}

SDL2Audio::SDL2Audio(int freq_id) : Audio(freq_id) {

    if (!enabled) {
        return;
    }

    int sample_size;
    SDL_AudioSpec aspec, obtained;

    sample_size = calc_samples();
    buf_size = sample_size * channels * 2 * 8;
    buffer_sdl = (unsigned char *) malloc((size_t) buf_size);

    buffered_bytes = 0;
    buf_read_pos = 0;
    buf_write_pos = 0;

    aspec.format = AUDIO_S16;
    aspec.freq = nBurnSoundRate;
    aspec.channels = (Uint8) channels;
    aspec.samples = (Uint16) sample_size;
    aspec.callback = read_buffer;
    aspec.userdata = NULL;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)) {
        printf("SDL2Audio: Initialize failed: %s.\n", SDL_GetError());
        pBurnSoundOut = NULL;
        nBurnSoundRate = 0;
        nBurnSoundLen = 0;
        return;
    }

    if (SDL_OpenAudio(&aspec, &obtained) < 0) {
        printf("SDL2Audio: Unable to open audio: %s\n", SDL_GetError());
        pBurnSoundOut = NULL;
        nBurnSoundRate = 0;
        nBurnSoundLen = 0;
        return;
    }

    if (use_mutex) {
        sound_mutex = SDL_CreateMutex();
        sound_cv = SDL_CreateCond();
        printf("SDL2Audio: using mutexes for synchro\n");
    }

    printf("SDL2Audio: frequency %d\n", obtained.freq);
    printf("SDL2Audio: samples %d\n", obtained.samples);
    printf("SDL2Audio: channels %d\n", obtained.channels);

    SDL_PauseAudio(0);
}

SDL2Audio::~SDL2Audio() {

    if (!enabled) {
        return;
    }
    SDL_PauseAudio(1);

    if(use_mutex) {
        SDL_LockMutex(sound_mutex);
        buffered_bytes = buf_size;
        SDL_CondSignal(sound_cv);
        SDL_UnlockMutex(sound_mutex);
        SDL_Delay(100);

        SDL_DestroyCond(sound_cv);
        SDL_DestroyMutex(sound_mutex);
    }

    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    if (buffer_sdl != NULL) {
        free(buffer_sdl);
        buffer_sdl = NULL;
    }
}

void SDL2Audio::Play() {

    if (!enabled) {
        return;
    }

    write_buffer((unsigned char *) buffer_fba, size);
}

void SDL2Audio::Pause(int pause) {

    if (!enabled) {
        return;
    }

    Audio::Pause(pause);
    SDL_PauseAudio(pause);

    if(use_mutex) {
        if(pause) {
            SDL_LockMutex(sound_mutex);
            buffered_bytes = 0;
            SDL_CondSignal(sound_cv);
            SDL_UnlockMutex(sound_mutex);
            SDL_Delay(100);

            SDL_DestroyCond(sound_cv);
            sound_cv = NULL;
            SDL_DestroyMutex(sound_mutex);
            sound_mutex = NULL;
        } else {
            sound_mutex = SDL_CreateMutex();
            sound_cv = SDL_CreateCond();
        }
    }
}